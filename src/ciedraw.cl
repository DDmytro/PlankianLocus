/*
 *  ciedraw.cl
 *  Contains OpenCL kernels code.
 *  Created on: Aug 3, 2017
 *      Author: Dmytro Dadyka
 */


float xFit_1931( float wave )
{
   float t1 = (wave - 442.0f) * ((wave < 442.0f) ? 0.0624f : 0.0374f);
   float t2 = (wave - 599.8f) * ((wave < 599.8f) ? 0.0264f : 0.0323f);
   float t3 = (wave - 501.1f) * ((wave<501.1f) ? 0.0490f : 0.0382f);
   return 0.362f * expf(-0.5f * t1 * t1) + 1.056f * expf(-0.5f * t2* t2)
          - 0.065f * expf(-0.5f * t3 * t3);
}

float yFit_1931( float wave )
{
float t1 = (wave-568.8f) * ((wave<568.8f)?0.0213f:0.0247f);
float t2 = (wave-530.9f) * ((wave<530.9f)?0.0613f:0.0322f);
return 0.821f * exp(-0.5f * t1 * t1) + 0.286f * expf(-0.5f * t2 * t2);
}

float zFit_1931( float wave )
{
   float t1 = (wave - 437.0f) * ((wave < 437.0f) ? 0.0845f : 0.0278f);
   float t2 = (wave - 459.0f) * ((wave < 459.0f) ? 0.0385f : 0.0725f);
   return 1.217f * exp(-0.5f * t1 *t1) + 0.681f * expf(-0.5f * t2 *t2);
}




// tongue left(y), right(y) points. y-step 0.03
// source: https://rip94550.wordpress.com/2009/10/18/color-the-1931-cie-color-matching-functions-and-chromaticity-chart/
__constant float tong_inv_step = 1 / 0.03;
__constant float2 tong_lr[] = {
   (float2)(0.174f,   1.0f),
   (float2)(0.143f,   0.97f),
   (float2)(0.122f,   0.94f),
   (float2)(0.108f,   0.91f),
   (float2)(0.096f,   0.88f),
   (float2)(0.0851f,	 0.85f),
   (float2)(0.075f,	 0.82f),
   (float2)(0.066f,	 0.79f),
   (float2)(0.0581f,	 0.76f),
   (float2)(0.0509f,	 0.73f),
   (float2)(0.0443f,	 0.7f),
   (float2)(0.0382f,	 0.67f),
   (float2)(0.0325f,	 0.64f),
   (float2)(0.0272f,	 0.609f),
   (float2)(0.0223f,	 0.579f),
   (float2)(0.018f,	 0.549f),
   (float2)(0.0143f,	 0.519f),
   (float2)(0.011f,	 0.489f),
   (float2)(0.00804f, 0.459f),
   (float2)(0.00585f, 0.429f),
   (float2)(0.00464f, 0.398f),
   (float2)(0.00409f, 0.367f),
   (float2)(0.00392f, 0.336f),
   (float2)(0.00578f, 0.304f),
   (float2)(0.00957f, 0.270f),
   (float2)(0.0138f,	 0.234f),
   (float2)(0.0232f,	 0.195f),
   (float2)(0.0379f,	 0.147f),
   (float2)(0.0933f,	 0.0304f),
   (float2)(0.257f,	 -0.211f),
   (float2)(0.5f,     -0.5f)
};

//Polynominal (7-order) aproximation coeffs for x(T), y(T) from Plank`s law.
// source x(T) and y(T) data:  http://www.vendian.org/mncharity/dir3/blackbody/UnstableURLs/bbr_color.html
// 600K < T < 30000K; RMSE =0,999114
__constant float8 plankianA = 
    (float8)(0.2429412f, 0.1260084f, 3.2346594f, -8.1675792f,
             9.3780297f, -5.7417308f, 1.8108807f, -0.2309579f);
__constant float8 plankianB = 
    (float8)(0.2349946f, 0.3149833f, 3.4476308f, -14.5562547f, 23.2823582f,
    	     -18.6777448f, 7.5000861f, -1.1998424f);


//XYZ to sRGB matrix source: https://webstore.iec.ch/publication/6168
__constant float3 XYZtoSRGBRow1 = (float3)(3.2406255f,  -1.537208f, -0.4986286f);
__constant float3 XYZtoSRGBRow2 = (float3)(-0.9689307f, 1.8757561f, 0.0415175f);
__constant float3 XYZtoSRGBRow3 = (float3)(0.0557101f, -0.2040211f, 1.0569959f);

inline void writePixel(__global uchar* img, uint2 imgSize, int2 coord, float3 rgb)
{
   uint idx = (coord.x + coord.y * imgSize.x) * 3; 
   img[idx] = rgb.x * 255;
   img[idx + 1] = rgb.y * 255;
   img[idx + 2] = rgb.z * 255;
}

//OpenCL optimized bhm algorithm
inline void drawline(__global uchar* img, uint2 imgSize, 
                     int2 p0, int2 p1, float3 rgb)
{
    int2 d = p1 - p0;
    int steps, i;
    int3 inc0, inc1;
    
    int2 sign = select((int2)1, (int2)-1, d < (int2)0);
    
    d = convert_int(abs(d));
    
    steps = max(d.x, d.y);
    
    d *= 2;
    
    inc0 = select((int3)(sign.x, 0, d.y), (int3)(0, sign.y, d.x),
                  (int3)-1*(d.x < d.y));
                  
    inc1 = select((int3)(sign.x, sign.y, d.y - d.x),
                  (int3)(sign.x, sign.y, d.x  - d.y),
                  (int3)-1*(d.x < d.y));
 
    int3 p = (int3)(p0, inc1.z);
    for(i = 0; i < steps; i++)
    {
        writePixel(img, imgSize, p.xy, rgb);
        p += select(inc1, inc0, (int3) -1 * (p.z < 0));
    }
}

//Userful vector
__constant uint2 e =  (uint2)(1);

__attribute__((reqd_work_group_size(8, 8, 1)))
__kernel void ChromaDraw(float4 bgColor, uint2 imgSize,
                         __global uchar* img)
{
   int2 coord = (int2)(get_global_id(0), get_global_id(1)); // pixel coords
   
   if (coord.x >= imgSize.x || coord.y >= imgSize.y) return;
   
   //Get xy space coords
   float3 xyz;
   xyz.xy = convert_float2(coord) / convert_float2(imgSize - e);
   xyz.y = 1 - xyz.y;
   xyz.z =  1.0 - (xyz.x + xyz.y);

   float3 rgb;
   // Apply XYZ to sRGB matrix
   rgb.x = dot(xyz, XYZtoSRGBRow1);
   rgb.y = dot(xyz, XYZtoSRGBRow2);
   rgb.z = dot(xyz, XYZtoSRGBRow3);
   
   //normalize
   float jmax = fmax(rgb.x, fmax(rgb.y, rgb.z));
   rgb /= (float3) jmax;
  
   //remove negative values
   rgb = clamp(rgb, (float3)0, (float3)1);
   
   // sRGB gamma correction: https://webstore.iec.ch/publication/6168
   float3 unlin = pow((1.055f * rgb),  1 / 2.4f) - (float3)0.055f;
   float3 lin = 12.92f * rgb;
   rgb = select(unlin, lin, rgb < (float3)0.003131f);

   // find tongue boundary fragment two nearest points
   float norm_y = xyz.y * tong_inv_step;
   int fr_idx = convert_int_rtz(norm_y);
   float dy = norm_y - fr_idx;
   
   fr_idx = clamp(fr_idx, 0, 28);
   float2 startp = tong_lr[fr_idx];
   float2 stopp = tong_lr[fr_idx + 1];

   if(//xyz.y > 0.8338 // tongue top
      (xyz.x - 0.8f) / (0.1f - 0.8f)  < (xyz.y - 0.2f) / (0.9f - 0.2f) // first line
      || (xyz.x - 0.172f) / (0.74f - 0.172f)  > (xyz.y) / 0.26f // second line
//      || (xyz.x - startp.x) < dy * (stopp.x - startp.x) // tongue left
//      || (xyz.x - startp.y) > dy * (stopp.y - startp.y)
) // tongue right
   {
      writePixel(img, imgSize, coord, bgColor.xyz);
      return;
   }
   writePixel(img, imgSize, coord, rgb);
}

//Polynomial approximation of parametric curve.
inline float2 parametricApprox(float8 coefs1, float8 coefs2, float t)
{   
   //Polinoms powers
   float8 p;
   p.xyz = (float3)(1.0f, t, t * t);
   p.w = p.z * t;
   p.hi = p.lo * p.w * t;

   return (float2)(dot(coefs1.lo, p.lo) + dot(coefs1.hi, p.hi),
                   1.0f - dot(coefs2.lo, p.lo) - dot(coefs2.hi, p.hi));
}


__attribute__((reqd_work_group_size(8, 1, 1)))
__kernel void PlankianLocusDraw(float4 curveCol, float invTStart, float d,
							 uint points, uint2 imgSize, __global uchar* img)

{
   int p = get_global_id(0);
   
   if (p >= points) return;
   
   float2 imgRect = convert_float2(imgSize - e);

   //Draw axis (plot part of axis foreach thread)
   float2 ds = imgRect / (float2)points;
   int2 start = convert_int(ds * p);
   int2 stop = convert_int(ds * (p + 1));
   //X axis
   drawline(img, imgSize, (int2)(stop.x, imgSize.x - 1),
           (int2)(start.x, imgSize.x - 1), (float3)(1.0f));
   //Y axis
   drawline(img, imgSize, (int2)(0, start.y),
           (int2)(0, stop.y), (float3)(1.0f));

   //Find approximation xy(1000/T) for two nearest points.
   float invT =  fma(p, d, invTStart);
   float2 xy = parametricApprox(plankianA, plankianB, invT);
   int2 coord0 = convert_int2(clamp(imgRect * xy, (float2)0.0f, imgRect));
   
   invT += d;
   xy = parametricApprox(plankianA, plankianB, invT);
   int2 coord1 = convert_int2(clamp(imgRect * xy, (float2)0.0f, imgRect));
   
   // draw patr of xy curve
   drawline(img, imgSize, coord0, coord1, curveCol.xyz);
}

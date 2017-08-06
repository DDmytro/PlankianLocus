/*
 *  main.cpp
 *  Created on: Aug 3, 2017
 *      Author: Dmytro Dadyka
 */
#include "OCLEngine.h"
#include <iostream>
#include <fstream>
#include <stdio.h>

//Diagram params
const cl_float4 BG_COLOR =
{ 0, 0, 0, 1 };
const int OUT_WIDTH = 500;
const int OUT_HEIGHT = 500;
const std::string OUTPUT_FILE_NAME = "planckianLocus.ppm";

//Locus curve params
const float LOCUS_START_T = 1500;
const float LOCUS_STOP_T = 10000;
const float LOCUS_CURVE_POINTS = 16;
const cl_float4 LOCUS_CURVE_COLOR =
{ 0, 0, 0, 1 };

//Kernels
const std::string PROGRAM_FILE_NAME = "./src/ciedraw.cl";
const std::string DIAGRAM_KERNEL = "ChromaDraw";
const std::string PLANKIAN_LOCUS_KERNEL = "PlankianLocusDraw";

using namespace std;

static bool PPMSave(const char* pix, size_t w, size_t h, const char* filename)
{
   std::ofstream out(filename, std::ios::out | std::ios::binary);
   if (out.is_open())
   {
      out << "P6\n" << w << " " << h << "\n" << 255 << "\n";
      out.write(pix, 3 * w * h);
      out.close();
      return true;
   }
   else
      return false;
};

//TODO move to OCL engine
inline size_t matchSize(size_t global, size_t local)
{
   return ((global + local - 1) / local) * local;
}

bool DrawCIE(OCLEngine& ocleng)
{
   auto prog = ocleng.loadProgramFromFile(PROGRAM_FILE_NAME);
   if (prog == OCL_WRONG_OBJECT)
   {
      cout << "OpenCL program creation failed." << endl;
      return false;
   }

   auto diagDrawKern = ocleng.createKernel(prog, DIAGRAM_KERNEL);
   if (diagDrawKern == OCL_WRONG_OBJECT)
   {
      cout << "OpenCL kernel creation failed." << endl;
      return false;
   }

   auto locusDrawKernel = ocleng.createKernel(prog, PLANKIAN_LOCUS_KERNEL);
   if (locusDrawKernel == OCL_WRONG_OBJECT)
   {
      cout << "OpenCL kernel creation failed." << endl;
      return false;
   }

   auto outImg = ocleng.createRGB8Image(OCL_MEM_WRITE, OUT_WIDTH,
         OUT_HEIGHT);
   if (outImg == OCL_WRONG_OBJECT)
   {
      cout << "OpenCL image creation failed" << endl;
      return false;
   }

   // Draw CIE diagram
   int argn = 0;
   ocleng.addFloat4Arg(diagDrawKern, argn++, BG_COLOR);
   ocleng.addUInt2Arg(diagDrawKern, argn++, OUT_WIDTH, OUT_HEIGHT);
   ocleng.addImageArg(diagDrawKern, argn++, outImg);

   size_t localsize[] =
   { 8, 8, 1 };
   size_t globalsize[] =
   { matchSize(OUT_WIDTH, 8), matchSize(OUT_HEIGHT, 8), 1 };
   if (!ocleng.enqueueKernel(diagDrawKern, 3, globalsize, localsize))
   {
      cout << "Error of kernel execution" << endl;
      return false;
   }

   // Draw plankian locus
   // For x(t), y(t) linearisation inverse scaled 1000 temperature used.
   //Minimum supported T is 100K
   float invStartT;
   if (LOCUS_START_T > 100)
      invStartT = 1000.0f / LOCUS_START_T;
   else
      invStartT = 1000.0f / 100;
   float dInvT = (1000.0f / LOCUS_STOP_T - invStartT) / LOCUS_CURVE_POINTS;

   argn = 0;
   ocleng.addFloat4Arg(locusDrawKernel, argn++, LOCUS_CURVE_COLOR);
   ocleng.addFloatArg(locusDrawKernel, argn++, invStartT);
   ocleng.addFloatArg(locusDrawKernel, argn++, dInvT);
   ocleng.addUIntArg(locusDrawKernel, argn++, LOCUS_CURVE_POINTS);
   ocleng.addUInt2Arg(locusDrawKernel, argn++, OUT_WIDTH, OUT_HEIGHT);
   ocleng.addImageArg(locusDrawKernel, argn, outImg);

   localsize[0] = 8;
   localsize[1] = 1;
   // find nearest match size
   globalsize[0] = matchSize(LOCUS_CURVE_POINTS, 8);
   globalsize[1] = 1;
   if (!ocleng.enqueueKernel(locusDrawKernel, 3, globalsize, localsize))
   {
      cout << "Error of kernel execution" << endl;
      return false;
   }

   auto ptr = (char*) ocleng.mapImage(outImg, OUT_WIDTH, OUT_HEIGHT);
   if (!ptr)
   {
      cout << "Error mapping image" << endl;
      return false;
   }

   PPMSave(ptr, OUT_WIDTH, OUT_HEIGHT, OUTPUT_FILE_NAME.c_str());

   if (!ocleng.unMapImage(outImg, ptr))
   {
      cout << "Error unmapping image" << endl;
      return false;
   }

   return true;
}

int main(int argc, const char** argv)
{
   OCLEngine eng;

   if (!eng.init())
   {
      cout << "Can`t init OpenCL engine.";
      return EXIT_SUCCESS;
   }

   struct timespec start, end;
   clock_gettime(CLOCK_PROCESS_CPUTIME_ID, &start);

   if (DrawCIE(eng))
      cout << "Output image saved to " << OUTPUT_FILE_NAME << endl;
   else
      cout << "Nothing to save." << endl;

   clock_gettime(CLOCK_PROCESS_CPUTIME_ID, &end);
   double duration = (end.tv_nsec - start.tv_nsec) * 1e-6
         + (end.tv_sec - start.tv_sec) * 1e+3;

   cout << "Time: " << duration << " msec" << endl;

   return EXIT_SUCCESS;
}

/*
 *  OCLEngine.h
 *  Simple OpenCL wrapper.
 *  Created on: Aug 3, 2017
 *      Author: Dmytro Dadyka
 */

#include "OCLEngine.h"
#include <iostream>
#include <vector>
#include <fstream>
#include <cstring>

using namespace std;

string opencl_error_to_str(cl_int error)
{
#define CASE_CL_CONSTANT(NAME) case NAME: return #NAME;
   switch (error)
   {
   CASE_CL_CONSTANT(CL_SUCCESS)
   CASE_CL_CONSTANT(CL_DEVICE_NOT_FOUND)
   CASE_CL_CONSTANT(CL_DEVICE_NOT_AVAILABLE)
   CASE_CL_CONSTANT(CL_COMPILER_NOT_AVAILABLE)
   CASE_CL_CONSTANT(CL_MEM_OBJECT_ALLOCATION_FAILURE)
   CASE_CL_CONSTANT(CL_OUT_OF_RESOURCES)
   CASE_CL_CONSTANT(CL_OUT_OF_HOST_MEMORY)
   CASE_CL_CONSTANT(CL_PROFILING_INFO_NOT_AVAILABLE)
   CASE_CL_CONSTANT(CL_MEM_COPY_OVERLAP)
   CASE_CL_CONSTANT(CL_IMAGE_FORMAT_MISMATCH)
   CASE_CL_CONSTANT(CL_IMAGE_FORMAT_NOT_SUPPORTED)
   CASE_CL_CONSTANT(CL_BUILD_PROGRAM_FAILURE)
   CASE_CL_CONSTANT(CL_MAP_FAILURE)
   CASE_CL_CONSTANT(CL_MISALIGNED_SUB_BUFFER_OFFSET)
   CASE_CL_CONSTANT(CL_EXEC_STATUS_ERROR_FOR_EVENTS_IN_WAIT_LIST)
   CASE_CL_CONSTANT(CL_INVALID_VALUE)
   CASE_CL_CONSTANT(CL_INVALID_DEVICE_TYPE)
   CASE_CL_CONSTANT(CL_INVALID_PLATFORM)
   CASE_CL_CONSTANT(CL_INVALID_DEVICE)
   CASE_CL_CONSTANT(CL_INVALID_CONTEXT)
   CASE_CL_CONSTANT(CL_INVALID_QUEUE_PROPERTIES)
   CASE_CL_CONSTANT(CL_INVALID_COMMAND_QUEUE)
   CASE_CL_CONSTANT(CL_INVALID_HOST_PTR)
   CASE_CL_CONSTANT(CL_INVALID_MEM_OBJECT)
   CASE_CL_CONSTANT(CL_INVALID_IMAGE_FORMAT_DESCRIPTOR)
   CASE_CL_CONSTANT(CL_INVALID_IMAGE_SIZE)
   CASE_CL_CONSTANT(CL_INVALID_SAMPLER)
   CASE_CL_CONSTANT(CL_INVALID_BINARY)
   CASE_CL_CONSTANT(CL_INVALID_BUILD_OPTIONS)
   CASE_CL_CONSTANT(CL_INVALID_PROGRAM)
   CASE_CL_CONSTANT(CL_INVALID_PROGRAM_EXECUTABLE)
   CASE_CL_CONSTANT(CL_INVALID_KERNEL_NAME)
   CASE_CL_CONSTANT(CL_INVALID_KERNEL_DEFINITION)
   CASE_CL_CONSTANT(CL_INVALID_KERNEL)
   CASE_CL_CONSTANT(CL_INVALID_ARG_INDEX)
   CASE_CL_CONSTANT(CL_INVALID_ARG_VALUE)
   CASE_CL_CONSTANT(CL_INVALID_ARG_SIZE)
   CASE_CL_CONSTANT(CL_INVALID_KERNEL_ARGS)
   CASE_CL_CONSTANT(CL_INVALID_WORK_DIMENSION)
   CASE_CL_CONSTANT(CL_INVALID_WORK_GROUP_SIZE)
   CASE_CL_CONSTANT(CL_INVALID_WORK_ITEM_SIZE)
   CASE_CL_CONSTANT(CL_INVALID_GLOBAL_OFFSET)
   CASE_CL_CONSTANT(CL_INVALID_EVENT_WAIT_LIST)
   CASE_CL_CONSTANT(CL_INVALID_EVENT)
   CASE_CL_CONSTANT(CL_INVALID_OPERATION)
   CASE_CL_CONSTANT(CL_INVALID_GL_OBJECT)
   CASE_CL_CONSTANT(CL_INVALID_BUFFER_SIZE)
   CASE_CL_CONSTANT(CL_INVALID_MIP_LEVEL)
   CASE_CL_CONSTANT(CL_INVALID_GLOBAL_WORK_SIZE)
   CASE_CL_CONSTANT(CL_INVALID_PROPERTY)
   default:
      return "UNKNOWN ERROR CODE ";
   }

#undef CASE_CL_CONSTANT
}

#define CHECK_CL_ERROR(ERR, ret)                \
    if(ERR != CL_SUCCESS)                       \
    {											            \
			cout << "Error in file " << __FILE__	\
			<< " at line " << __LINE__ << ": "     \
			<< opencl_error_to_str(ERR) << endl;   \
			return ret;                            \
    }

#define CHECK_OBJECT(obj, container, msg, ret) \
   if (obj < 0 || obj >=  container.size())    \
   {                                           \
      cout << msg << endl;                     \
      return ret;                              \
   }                                           \

OCLEngine::OCLEngine() :
      platform(0), device(0), context(0), queue(0)
{
}

bool OCLEngine::platformInit()
{
   cl_uint ret_num_platforms;
   cl_int ret = clGetPlatformIDs(0, NULL, &ret_num_platforms);
   CHECK_CL_ERROR(ret, false);

   if (!ret_num_platforms)
   {
      cout << "There are no valid platforms on PC!";
      return false;
   }

   std::cout << "Platforms(" << ret_num_platforms << "):" << std::endl;

   vector<cl_platform_id> platforms(ret_num_platforms);
   ret = clGetPlatformIDs(ret_num_platforms, &platforms[0], NULL);
   CHECK_CL_ERROR(ret, false);

   for (cl_uint i = 0; i < ret_num_platforms; ++i)
   {
      // Get the length for the i-th platform name
      size_t platform_name_length = 0;
      ret = clGetPlatformInfo(platforms[i],
      CL_PLATFORM_NAME, 0, 0, &platform_name_length);
      CHECK_CL_ERROR(ret, false);

      // Get the name itself for the i-th platform
      // use vector for automatic memory management
      vector<char> platform_name(platform_name_length);
      ret = clGetPlatformInfo(platforms[i],
      CL_PLATFORM_NAME, platform_name_length, &platform_name[0], 0);
      CHECK_CL_ERROR(ret, false);
      cout << "    [" << i << "] " << &platform_name[0] << std::endl;
   }

   std::cout << "Please, select platform " << endl;
   unsigned int selected_platform;
   while (!(std::cin >> selected_platform)
         || selected_platform >= ret_num_platforms)
   {
      cin.clear();
      cin.ignore(512, '\n');
      cout << "Incorrect platform number. Please repeat." << endl;
   }
   platform = platforms[selected_platform];

   return true;
}

bool OCLEngine::deviceInit()
{
   cl_int ret;
   cl_uint ret_num_devices;
   cl_device_id device_id = NULL;
   ret = clGetDeviceIDs(platform, CL_DEVICE_TYPE_ALL, 1, &device_id,
         &ret_num_devices);
   CHECK_CL_ERROR(ret, false);

   std::cout << "Devices(" << ret_num_devices << "):" << std::endl;

   vector<cl_device_id> devices(ret_num_devices);

   ret = clGetDeviceIDs(platform,
   CL_DEVICE_TYPE_ALL, ret_num_devices, &devices[0], 0);
   CHECK_CL_ERROR(ret, false);

   for (cl_uint i = 0; i < ret_num_devices; ++i)
   {
      // Get the length for the i-th device name
      size_t device_name_length = 0;
      ret = clGetDeviceInfo(devices[i],
      CL_DEVICE_NAME, 0, 0, &device_name_length);
      CHECK_CL_ERROR(ret, false);
      // Get the name itself for the i-th device
      // use vector for automatic memory management
      vector<char> device_name(device_name_length);
      ret = clGetDeviceInfo(devices[i],
      CL_DEVICE_NAME, device_name_length, &device_name[0], 0);
      CHECK_CL_ERROR(ret, false);

      cout << "    [" << i << "] " << &device_name[0] << std::endl;
   }

   std::cout << "Please, select device " << endl;
   unsigned int selected_device;
   while (!(std::cin >> selected_device) || selected_device >= ret_num_devices)
   {
      cin.clear();
      cin.ignore(512, '\n');
      cout << "Incorrect device number. Please repeat." << endl;
   }
   device = devices[selected_device];
   return true;
}

bool OCLEngine::init()
{
   cl_int ret;
   if (!platformInit())
   {
      cout << "Can`t init OpenCL platform.";
      return false;
   }
   if (!deviceInit())
   {
      cout << "Can`t init OpenCL device.";
      return false;
   }

   context = clCreateContext( NULL, 1, &device, NULL, NULL, &ret);
   CHECK_CL_ERROR(ret, false);
   queue = clCreateCommandQueue(context, device, NULL, &ret);
   CHECK_CL_ERROR(ret, false);
   return true;
}

OCLProgram OCLEngine::loadProgram(const std::string str)
{
   cl_int ret;
   auto p = str.c_str();
   auto ps = str.size();
   auto program = clCreateProgramWithSource(context, 1, &p, &ps, &ret);
   CHECK_CL_ERROR(ret, OCL_WRONG_OBJECT);

   ret = clBuildProgram(program, 1, &device, NULL, NULL, NULL);
   if (ret == CL_BUILD_PROGRAM_FAILURE)
   {
      size_t log_length = 0;
      ret = clGetProgramBuildInfo(program, device,
      CL_PROGRAM_BUILD_LOG, 0, 0, &log_length);
      CHECK_CL_ERROR(ret, OCL_WRONG_OBJECT);

      string log;
      log.resize(log_length);

      ret = clGetProgramBuildInfo(program, device,
      CL_PROGRAM_BUILD_LOG, log_length, (void*) log.c_str(), 0);
      CHECK_CL_ERROR(ret, OCL_WRONG_OBJECT);
      cout << log << endl;
      return OCL_WRONG_OBJECT;
   }
   programs.push_back(program);
   return programs.size() - 1;
}

OCLProgram OCLEngine::loadProgramFromFile(const std::string filename)
{
   string str;
   ifstream file;
   file.exceptions(ifstream::failbit | ifstream::badbit);
   try
   {
      file.open(filename.c_str());
      str.assign((std::istreambuf_iterator<char>(file)),
            std::istreambuf_iterator<char>());
      file.close();
   } catch (const ifstream::failure& e)
   {
      cout << "Error opening/reading OpenCL program file" << endl;
      return OCL_WRONG_OBJECT;
   }

   return loadProgram(str);
}

OCLKernel OCLEngine::createKernel(OCLProgram prog, const std::string name)
{
   cl_int ret;
   CHECK_OBJECT(prog, programs, "Invalid OCL program.", OCL_WRONG_OBJECT);
   auto kernel = clCreateKernel(programs[prog], name.c_str(), &ret);
   CHECK_CL_ERROR(ret, OCL_WRONG_OBJECT);
   kernels.push_back(kernel);
   return kernels.size() - 1;
}

bool OCLEngine::releaseAllResources()
{
   cl_int ret = true;

   for (auto it : images)
   {
      ret = clReleaseMemObject(it);
      if (ret != CL_SUCCESS)
      {
         ret &= false;
         cout << "Error while image releasing." << endl;
      }
   }

   for (auto it : programs)
   {
      ret = clReleaseProgram(it);
      if (ret != CL_SUCCESS)
         if (ret != CL_SUCCESS)
         {
            ret &= false;
            cout << "Error while program releasing." << endl;
         }
   }

   for (auto it : kernels)
   {
      ret = clReleaseKernel(it);
      if (ret != CL_SUCCESS)
      {
         ret &= false;
         cout << "Error while kernel releasing." << endl;
      }
   }

   if (queue)
   {
      ret = clReleaseCommandQueue(queue);
      if (ret != CL_SUCCESS)
      {
         ret &= false;
         cout << "Error while queue releasing." << endl;
      }
   }

   if (context)
   {
      ret = clReleaseContext(context);
      if (ret != CL_SUCCESS)
      {
         ret &= false;
         cout << "Error while context releasing." << endl;
      }
   }

   return true;
}

void* OCLEngine::mapImage(OCLImage image, int width, int heigth)
{
   cl_int ret;

   CHECK_OBJECT(image, images, "Invalid OCL image.", nullptr);

   size_t originst[3];
   size_t regionst[3];
   size_t rowPitch = 0;
   size_t slicePitch = 0;
   originst[0] = 0;
   originst[1] = 0;
   originst[2] = 0;
   regionst[0] = width;
   regionst[1] = heigth;
   regionst[2] = 1;

   auto ptr = clEnqueueMapBuffer(queue, images[image],
   CL_TRUE, CL_MAP_READ, 0,
   width * heigth * 3,
   0,
   NULL,
   NULL,
   &ret);

   CHECK_CL_ERROR(ret, nullptr);
   ret = clFinish(queue);
   CHECK_CL_ERROR(ret, nullptr);
   return ptr;
}

bool OCLEngine::unMapImage(OCLImage image, void* ptr)
{
   CHECK_OBJECT(image, images, "Invalid OCL image.", false);
   clEnqueueUnmapMemObject(queue, images[image], ptr, 0,
      NULL,
      NULL);
   auto ret = clFinish(queue);
   CHECK_CL_ERROR(ret, false);
   return true;

}

bool OCLEngine::readImage(OCLImage image, int width, int heigth, char * data)
{
   CHECK_OBJECT(image, images, "Invalid OCL image.", false);

   size_t originst[3];
   size_t regionst[3];
   size_t rowPitch = 0;
   size_t slicePitch = 0;
   originst[0] = 0;
   originst[1] = 0;
   originst[2] = 0;
   regionst[0] = width;
   regionst[1] = heigth;
   regionst[2] = 1;

   auto ret = clEnqueueReadImage(queue, images[image],
   CL_TRUE, originst, regionst, rowPitch, slicePitch, data, 0,
   NULL,
   NULL);
   CHECK_CL_ERROR(ret, false);
   return true;
}

bool OCLEngine::enqueueKernel(OCLKernel kernel, cl_uint dim, size_t* globalsize,
      size_t* localsize)
{
   CHECK_OBJECT(kernel, kernels, "Invalid OCL kernel,", false);
   auto ret = clEnqueueNDRangeKernel(queue, kernels[kernel], dim,
           NULL, globalsize, localsize, 0, NULL, NULL);
   CHECK_CL_ERROR(ret, false);
   ret = clFinish(queue);
   CHECK_CL_ERROR(ret, false);
   return true;
}

cl_mem_flags getCLMemFlags(OCLMemFlags flags)
{
   switch(flags)
   {
   case OCL_MEM_READ:
      return CL_MEM_READ_ONLY;
   case OCL_MEM_WRITE:
      return CL_MEM_WRITE_ONLY;
   case OCL_MEM_READ_WRITE:
      return CL_MEM_READ_WRITE;
   default:
      return OCL_MEM_READ;
   }

}

OCLImage OCLEngine::createRGB8Image(OCLMemFlags flags, int width, int heigth)
{
   cl_int ret;
   cl_image_format format;             // structure to define image format
   cl_image_desc desc;               // structure to define image description

   format.image_channel_data_type = CL_UNORM_INT8;
   format.image_channel_order = CL_RGBA;

   memset(&desc, 0, sizeof(desc));
   desc.image_type = CL_MEM_OBJECT_IMAGE2D;
   desc.image_width = width;
   desc.image_height = heigth;

   auto outputImage = clCreateBuffer(context, getCLMemFlags(flags),
         width * heigth * 3, NULL, &ret);

   CHECK_CL_ERROR(ret, OCL_WRONG_OBJECT);
   images.push_back(outputImage);
   return images.size() - 1;
}

bool OCLEngine::finish()
{
   auto ret = clFinish(queue);
   CHECK_CL_ERROR(ret, false);
   return true;
}

OCLEngine::~OCLEngine()
{
   releaseAllResources();
}

bool OCLEngine::addImageArg(OCLKernel kernel, int argIndex, OCLImage image)
{
   CHECK_OBJECT(image, images, "Invalid OCL image,", false);
   CHECK_OBJECT(kernel, kernels, "Invalid OCL kernel,", false);

   cl_int ret;
   ret = clSetKernelArg(kernels[kernel], argIndex, sizeof(cl_mem),
         &images[image]);
   CHECK_CL_ERROR(ret, false);
   return true;
}

bool OCLEngine::addFloat4Arg(OCLKernel kernel, int argIndex, cl_float4 color)
{
   CHECK_OBJECT(kernel, kernels, "Invalid OCL kernel,", false);
   cl_int ret;
   ret = clSetKernelArg(kernels[kernel], argIndex, sizeof(cl_float4), &color);
   CHECK_CL_ERROR(ret, false);
   return true;
}

bool OCLEngine::addUIntArg(OCLKernel kernel, int argIndex, cl_uint val)
{
   CHECK_OBJECT(kernel, kernels, "Invalid OCL kernel,", false);
   cl_int ret;
   ret = clSetKernelArg(kernels[kernel], argIndex, sizeof(cl_uint), &val);
   CHECK_CL_ERROR(ret, false);
   return true;
}

bool OCLEngine::addUInt2Arg(OCLKernel kernel, int argIndex, cl_uint x,
      cl_uint y)
{
   CHECK_OBJECT(kernel, kernels, "Invalid OCL kernel,", false);
   cl_uint2 vec2 =
   { x, y };
   cl_int ret;
   ret = clSetKernelArg(kernels[kernel], argIndex, sizeof(cl_uint2), &vec2);
   CHECK_CL_ERROR(ret, false);
   return true;
}

bool OCLEngine::addFloatArg(OCLKernel kernel, int argIndex, cl_float val)
{
   CHECK_OBJECT(kernel, kernels, "Invalid OCL kernel,", false);
   cl_int ret;
   ret = clSetKernelArg(kernels[kernel], argIndex, sizeof(cl_float), &val);
   CHECK_CL_ERROR(ret, false);
   return true;
}

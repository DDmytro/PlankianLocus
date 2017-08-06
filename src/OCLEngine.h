/*
 *  OCLEngine.h
 *  Simple OpenCL wrapper.
 *  Created on: Aug 3, 2017
 *      Author: Dmytro Dadyka
 */

#pragma once

#include <CL/cl.h>
#include <string>
#include <vector>

typedef int OCLImage;
typedef int OCLProgram;
typedef int OCLKernel;

#define OCL_WRONG_OBJECT -1

enum OCLMemFlags
{
   OCL_MEM_READ,
   OCL_MEM_WRITE,
   OCL_MEM_READ_WRITE
};

class OCLEngine
{
private:
	cl_platform_id platform = 0;
	cl_context context = 0;
	cl_command_queue queue = 0;
	cl_device_id device = 0;

	std::vector<cl_program> programs;
	std::vector<cl_kernel> kernels;
	std::vector<cl_mem> images;

	bool platformInit();
	bool deviceInit();
public:
	bool init(); // Show the OpenCL hardware initialisation dialog.
	bool releaseAllResources(); // Delete all created objects. 

	OCLProgram loadProgram(const std::string str); // Create and build program from string code.
	OCLProgram loadProgramFromFile(const std::string filename); // Create and build program from text file.
	OCLKernel createKernel(OCLProgram prog, const std::string name);

	bool enqueueKernel(OCLKernel kernel, cl_uint dim, size_t* globalsize, size_t* localsize);
	bool finish();

	OCLImage createRGB8Image(OCLMemFlags flags, int width, int heigth);
	bool readImage(OCLImage image, int width, int heigth, char * data); // Slow.Try not to use. 
	void* mapImage(OCLImage image, int width, int heigth);
	bool unMapImage(OCLImage image, void* ptr);

	bool addImageArg(OCLKernel kernel, int argIndex, OCLImage image);
   bool addFloatArg(OCLKernel kernel, int argIndex, cl_float val);
	bool addFloat4Arg(OCLKernel kernel, int argIndex, cl_float4 color);
	bool addUIntArg(OCLKernel kernel, int argIndex, cl_uint val);
	bool addUInt2Arg(OCLKernel kernel, int argIndex, cl_uint x, cl_uint y);


	OCLEngine();
	~OCLEngine();
};

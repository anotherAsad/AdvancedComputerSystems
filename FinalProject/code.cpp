#define CL_HPP_TARGET_OPENCL_VERSION 210

#include <iostream>
#include <fstream>
#include <sstream>
#include <CL/cl2.hpp>

#include "openCLsharable.h"

int gl_main(int argc, char* argv[]);
extern double rotate_x, rotate_y;

openCLsharable global_ocs;

cl::Platform getCLplatform() {
	// get all platforms (drivers)
	std::vector<cl::Platform> all_platforms;
	cl::Platform::get(&all_platforms);
	
	if(all_platforms.size() == 0) {
		std::cerr << "No platforms found. Check OpenCL installation!\n";
		exit(1);
	}
	else {
		std::cout << "\nFollowing platforms (drivers) were found:\n";
		
		for(auto plaform: all_platforms)
			std::cout << "\t" << plaform.getInfo<CL_PLATFORM_NAME>() << "\n";
			
		std::cout << "\n\n";
	}

	return all_platforms[0];	// Hopefully, a deep copy ensues.
}

cl::Device getCLdevice(cl::Platform &platform) {
	// get default device of the default platform
	std::vector<cl::Device> all_devices;
	platform.getDevices(CL_DEVICE_TYPE_ALL, &all_devices);
	
	if(all_devices.size() == 0) {
		std::cout << " No devices found. Check OpenCL installation!\n";
		exit(1);
	}
	else {
		std::cout << "\nFollowing devices were found:\n";
		
		for(auto device: all_devices)
			std::cout << "\t" << device.getInfo<CL_DEVICE_NAME>() << "\n";
			
		std::cout << "\n\n";
	}
	
	return all_devices[0];		// Hopefully, a deep copy ensues.
}

std::string readKernels(const char *filepath) {
	std::ifstream fin;
	std::stringstream file_sstream;
	
	fin.open(filepath);
	
	if(!fin.fail())
		std::cout << filepath << " opened successfully ...\n\n";
	else {
		std::cerr << "Failed to open " << filepath << "!!!\n";
		exit(0xDEADBEEF);		// 0xEF is 239
	}
	
	file_sstream << fin.rdbuf();
	
	return file_sstream.str();
}

void buildKernels(cl::Context &context, cl::Program &program, cl::Device &default_device, std::string kernel_code) {
	// Create an openCL program for the given device 

	cl::Program::Sources sources;
	
	sources.push_back({kernel_code.c_str(), kernel_code.length()});
	
	program = cl::Program(context, sources);
	
	if(program.build({default_device}) != CL_SUCCESS){
		std::cout<<" Error building: " << program.getBuildInfo<CL_PROGRAM_BUILD_LOG>(default_device)<<"\n";
		exit(1);
	}

	return;	
}

void runOpenCLkernel(const openCLsharable &ocs, const float *A, const float *B, float *C, std::string kernel_string, int buff_size) {
	std::cout << "Running kernel \"" << kernel_string << "\"\n";
	
    //create queue to which we will push commands for the device.
	const cl::CommandQueue queue(ocs.context, ocs.device);
	
	// write arrays A and B to the device. Uses DMA.
	queue.enqueueWriteBuffer(*ocs.buffer_A_ptr, CL_TRUE, 0, sizeof(float)*buff_size, A);
	queue.enqueueWriteBuffer(*ocs.buffer_B_ptr, CL_TRUE, 0, sizeof(float)*buff_size, B);
	
	cl::Kernel kernel(ocs.program, kernel_string.c_str());
	
	kernel.setArg(0, *ocs.buffer_A_ptr);
	kernel.setArg(1, *ocs.buffer_B_ptr);
	kernel.setArg(2, *ocs.buffer_C_ptr);
	
	// Enque an N dimensional kernel
	queue.enqueueNDRangeKernel(
		kernel,						// kernel name
		cl::NullRange,				// ???
		cl::NDRange(buff_size),		// range of operations
		cl::NullRange				// ??
	);
	
	queue.finish();
	
	//read result C from the device to array C
	queue.enqueueReadBuffer(*ocs.buffer_C_ptr,CL_TRUE,0,sizeof(float)*buff_size,C);
	
	return;
}

int main() {
	// Get the openCL platform (driver), and device
	cl::Platform default_platform = getCLplatform(); 
	std::cout << "Using platform: " << default_platform.getInfo<CL_PLATFORM_NAME>() << "\n";
	
	cl::Device default_device = getCLdevice(default_platform);
	std::cout << "Using device: " << default_device.getInfo<CL_DEVICE_NAME>() << "\n";
	
	// Declare CL context and platform
	cl::Context context({default_device});
	cl::Program program;
	std::string kernel_code = readKernels("opencl_kernels.cl");
	
	buildKernels(context, program, default_device, kernel_code);
	
	// make an openCLsharable device
	openCLsharable ocs(default_platform, default_device, context, program, 10);
	global_ocs = ocs;
    
	float A[] = {0, 1, 2, 3, 4, 5, 6, 7, 8, 9};
	float B[] = {0, 1, 2, 0, 1, 2, 0, 1, 2, 10};
	
    float C[10];
    
    // Simple Add
    runOpenCLkernel(ocs, A, B, C, "simple_add", 10);
   	
	std::cout<<"result: \n";
	
	for(int i=0;i<10;i++) {
		std::cout << A[i] << "+" << B[i] << "=" << C[i] << "\n";
	}
	
	std::cout << std::endl;
	
	// Euclidean Distance
	runOpenCLkernel(ocs, A, B, C, "euclidean_distance", 10);
	
	for(int i=0;i<10;i++) {
		std::cout << A[i] << "+" << B[i] << "=" << C[i] << "\n";
	}
	
	std::cout << std::endl;	
		
	gl_main(1, NULL);
	
	std::cout << "final rotate: (" << rotate_x << ", " << rotate_y << ")" << std::endl; 

	return 0;
}

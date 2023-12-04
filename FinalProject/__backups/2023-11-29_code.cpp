#define CL_HPP_TARGET_OPENCL_VERSION 210

#include <iostream>
#include <fstream>
#include <sstream>
#include <CL/cl2.hpp>

std::string readKernel(const char *filepath) {
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

void buildKernel(cl::Context &context, cl::Program &program, cl::Device &default_device, std::string kernel_code) {
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

int main() {
	// Get the openCL platform (driver), and device
	cl::Platform default_platform = getCLplatform(); 
	std::cout << "Using platform: " << default_platform.getInfo<CL_PLATFORM_NAME>() << "\n";
	
	cl::Device default_device = getCLdevice(default_platform);
	std::cout << "Using device: " << default_device.getInfo<CL_DEVICE_NAME>() << "\n";
	
	// Declare CL context and platform
	cl::Context context({default_device});
	cl::Program program;
	std::string kernel_code = readKernel("vector_add_kernel.cl");
	
	buildKernel(context, program, default_device, kernel_code);
	
    // create buffers on the iGPU
    cl::Buffer buffer_A(context,CL_MEM_READ_WRITE, sizeof(int)*10);
    cl::Buffer buffer_B(context,CL_MEM_READ_WRITE, sizeof(int)*10);
    cl::Buffer buffer_C(context,CL_MEM_READ_WRITE, sizeof(int)*10);
    
	int A[] = {0, 1, 2, 3, 4, 5, 6, 7, 8, 9};
	int B[] = {0, 1, 2, 0, 1, 2, 0, 1, 2, 0};

	//create queue to which we will push commands for the device.
	cl::CommandQueue queue(context, default_device);
	
	//write arrays A and B to the device
	queue.enqueueWriteBuffer(buffer_A,CL_TRUE,0,sizeof(int)*10,A);
	queue.enqueueWriteBuffer(buffer_B,CL_TRUE,0,sizeof(int)*10,B);
	
	cl::Kernel simple_add(program, "simple_add");
	simple_add.setArg(0, buffer_A);
	simple_add.setArg(1, buffer_B);
	simple_add.setArg(2, buffer_C);
	
	queue.enqueueNDRangeKernel(simple_add, cl::NullRange, cl::NDRange(10), cl::NullRange);
	queue.finish();

    int C[10];
    
	//read result C from the device to array C
	queue.enqueueReadBuffer(buffer_C,CL_TRUE,0,sizeof(int)*10,C);

	std::cout<<"result: \n";
	
	for(int i=0;i<10;i++) {
		std::cout << A[i] << "+" << B[i] << "=" << C[i] << "\n";
	}
	
	std::cout << std::endl;

	return 0;
}

struct openCLsharable {
	cl::Platform platform;
	cl::Device device;
	cl::Context context;
	cl::Program program; 
	
	cl::Buffer *buffer_A_ptr = NULL;
	cl::Buffer *buffer_B_ptr = NULL;
	cl::Buffer *buffer_C_ptr = NULL;
	
	int buff_size;
	
	void createNewBuffers(int buff_size) {
		// free-up old buffers
		if(buffer_A_ptr != NULL)
			delete buffer_A_ptr;
		
		if(buffer_B_ptr != NULL)
			delete buffer_B_ptr;
			
		if(buffer_C_ptr != NULL)
			delete buffer_C_ptr;
			
		// init new buffers on the iGPU
		buffer_A_ptr = new cl::Buffer(context, CL_MEM_READ_WRITE, sizeof(float)*buff_size);
		buffer_B_ptr = new cl::Buffer(context, CL_MEM_READ_WRITE, sizeof(float)*buff_size);
		buffer_C_ptr = new cl::Buffer(context, CL_MEM_READ_WRITE, sizeof(float)*buff_size);
		
		return;
	}
	
	openCLsharable(cl::Platform &platform, cl::Device &device, cl::Context &context, cl::Program &program, int buff_size) {
		this->platform = platform;
		this->device = device;
		this->context = context;
		this->program = program;
		this->buff_size = buff_size;
		createNewBuffers(buff_size);
	}
	
	openCLsharable() {
	}
	
	openCLsharable(int buff_size) {
		this->buff_size = buff_size;
		createNewBuffers(buff_size);
	}
	
	~openCLsharable() {
		// free-up old buffers
		if(buffer_A_ptr != NULL)
			delete buffer_A_ptr;
		
		if(buffer_B_ptr != NULL)
			delete buffer_B_ptr;
			
		if(buffer_C_ptr != NULL)
			delete buffer_C_ptr;
	}
};

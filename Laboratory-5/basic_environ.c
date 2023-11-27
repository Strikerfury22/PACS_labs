////////////////////////////////////////////////////////////////////
//File: basic_environ.c
//
//Description: base file for environment exercises with openCL
//
// 
////////////////////////////////////////////////////////////////////

#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#ifdef __APPLE__
  #include <OpenCL/opencl.h>
#else
  #include <CL/cl.h>
#endif
  
// check error, in such a case, it exits

void cl_error(cl_int code, const char *string){
	if (code != CL_SUCCESS){
		printf("%d - %s\n", code, string);
	    exit(-1);
	}
}
////////////////////////////////////////////////////////////////////////////////

int main(int argc, char** argv)
{
  int err;                            	// error code returned from api calls
  size_t t_buf = 50;			// size of str_buffer
  char str_buffer[t_buf];		// auxiliary buffer	
  size_t e_buf;				// effective size of str_buffer in use
	    
  size_t global_size;                      	// global domain size for our calculation
  size_t local_size;                       	// local domain size for our calculation

  const cl_uint num_platforms_ids = 10;				// max of allocatable platforms
  cl_platform_id platforms_ids[num_platforms_ids];		// array of platforms
  cl_uint n_platforms;						// effective number of platforms in use
  const cl_uint num_devices_ids = 10;				// max of allocatable devices
  cl_device_id devices_ids[num_platforms_ids][num_devices_ids];	// array of devices
  cl_uint n_devices[num_platforms_ids];				// effective number of devices in use for each platform
	
  cl_device_id device_id;             				// compute device id 
  cl_context context;                 				// compute context
  cl_command_queue command_queue;     				// compute command queue
  
  //First task
  size_t num_params = sizeof(cl_char) * 50;
  cl_char params[50];
  size_t actual_value;
    

  // 1. Scan the available platforms:
  err = clGetPlatformIDs (num_platforms_ids, platforms_ids, &n_platforms);
  cl_error(err, "Error: Failed to Scan for	 Platforms IDs");
  printf("Number of available platforms: %d\n\n", n_platforms);

  for (int i = 0; i < n_platforms; i++ ){
															 /***???         ???          ???***/
    err= clGetPlatformInfo(platforms_ids[i], CL_PLATFORM_NAME, num_params, params, &actual_value);
    cl_error (err, "Error: Failed to get info of the platform\n");
    printf( "\t[%d]-Platform Name: %s\n", i, str_buffer);
  }
  printf("\n");
  // ***Task***: print on the screen the name, host_timer_resolution, vendor, versionm, ...
	
  // 2. Scan for devices in each platform
  for (int i = 0; i < n_platforms; i++ ){
							/***???                     ???***/
    err = clGetDeviceIDs( platforms_ids[i], CL_DEVICE_TYPE_ALL, num_devices_ids, devices_ids[i], &(n_devices[i]));
    cl_error(err, "Error: Failed to Scan for Devices IDs");
    printf("\t[%d]-Platform. Number of available devices: %d\n", i, n_devices[i]);

    for(int j = 0; j < n_devices[i]; j++){
      err = clGetDeviceInfo(devices_ids[i][j], CL_DEVICE_NAME, sizeof(str_buffer), &str_buffer, NULL);
      cl_error(err, "clGetDeviceInfo: Getting device name");
      printf("\t\t [%d]-Platform [%d]-Device CL_DEVICE_NAME: %s\n", i, j,str_buffer);

      cl_uint max_compute_units_available;
      err = clGetDeviceInfo(devices_ids[i][j], CL_DEVICE_MAX_COMPUTE_UNITS, sizeof(max_compute_units_available), &max_compute_units_available, NULL);
      cl_error(err, "clGetDeviceInfo: Getting device max compute units available");
      printf("\t\t [%d]-Platform [%d]-Device CL_DEVICE_MAX_COMPUTE_UNITS: %d\n\n", i, j, max_compute_units_available);
    }
  }	
  // ***Task***: print on the screen the cache size, global mem size, local memsize, max work group size, profiling timer resolution and ... of each device



  // 3. Create a context, with a device
  cl_int ePlatID = 0; //Para indicar el índice de la plataforma del contexto
  cl_int eDevID = 0; //Para indicar el índice del dispositivo del contexto (Si queremos usar todos los de la plataforma sería poner solo n_devices[ePlatID], devices_ids[ePlatID] para crear el contexto)
	//Recordar: Un contexto tiene una única plataforma, una plataforma puede tener varios contextos.
																								  /***???***/		
  cl_context_properties properties[] = { CL_CONTEXT_PLATFORM, (cl_context_properties)platforms_ids[ePlatID], 0};
									   /***???                   ???***/		
  context = clCreateContext(properties, 1, &devices_ids[ePlatID][eDevID], NULL, NULL, &err);
  cl_error(err, "Failed to create a compute context\n");

  // 4. Create a command queue
  cl_command_queue_properties proprt[] = { CL_QUEUE_PROPERTIES, CL_QUEUE_PROFILING_ENABLE, 0 };
													/***???                          ???***/
  command_queue = clCreateCommandQueueWithProperties( context, devices_ids[ePlatID][eDevID], proprt, &err);
  cl_error(err, "Failed to create a command queue\n");

	// First Kernel on the device

  //2.
  // Calculate size of the file
  //https://cdiv.sourceforge.net/cdivhlp/const_SEEK.htm
							/***???***/
  FILE *fileHandler = fopen("kernel_pow.cl", "r");
		/***???***/
  fseek(fileHandler, 0, SEEK_END);
  size_t fileSize = ftell(fileHandler);
  rewind(fileHandler);

  // read kernel source into buffer
  char * sourceCode = (char*) malloc(fileSize + 1);
  sourceCode[fileSize] = '\0';
  fread(sourceCode, sizeof(char), fileSize, fileHandler);
  fclose(fileHandler);
  
  const char * constSourceCode = sourceCode;

  // create program from buffer
									  /***Todo                      ???***/
  cl_program program = clCreateProgramWithSource(context,1,&constSourceCode,&fileSize,&err);
  cl_error(err, "Failed to create program with source\n");
  free(sourceCode);
  
  //3.
    // Build the executable and check errors
  err = clBuildProgram(program, 0, NULL, NULL, NULL, NULL);
  if (err != CL_SUCCESS){
    size_t len;
    char buffer[2048];

    printf("Error: Some error at building process.\n");
	//Añadido
	cl_build_status estado;
	size_t bytesLeidos = 0;
						/***Todo ???***/
    clGetProgramBuildInfo(program, devices_ids[ePlatID][eDevID], CL_PROGRAM_BUILD_STATUS, sizeof(cl_build_status), &estado, &bytesLeidos);
    printf("%s\n", buffer);
    exit(-1);
  }
  //4.
    // Create a compute kernel with the program we want to run
						  /***Todo              ???***/
  cl_kernel kernel = clCreateKernel(program, "kernel_pow", &err);
  cl_error(err, "Failed to create kernel from the program\n");
	
  //5.
	//Inicializamos los buffers de entrada y salida:
	unsigned int count = 16;
	float entrada[count] = {0,1,2,3,4,5,6,7,8,9,10,11,12,13,14,15};
	float salida[count];	
  //6.
    // Create OpenCL buffer visible to the OpenCl runtime
																	    /***???***/
  cl_mem in_device_object  = clCreateBuffer(context, CL_MEM_READ_ONLY, sizeof(float)*count, NULL, &err);
  cl_error(err, "Failed to create memory buffer at device\n");
																	    /***???***/
  cl_mem out_device_object = clCreateBuffer(context, CL_MEM_WRITE_ONLY, sizeof(float)*count, NULL, &err);
  cl_error(err, "Failed to create memory buffer at device\n");
  
  //7.
    // Write date into the memory object 
																								/***???***/
  err = clEnqueueWriteBuffer(command_queue, in_device_object, CL_TRUE, 0, sizeof(float) * count, entrada, 0, NULL, NULL);
  cl_error(err, "Failed to enqueue a write command\n");
  
  //8.
    // Set the arguments to the kernel
  err = clSetKernelArg(kernel, 0, sizeof(cl_mem), &in_device_object);
  cl_error(err, "Failed to set argument 0\n");
  err = clSetKernelArg(kernel, 1, sizeof(cl_mem), &out_device_object);
  cl_error(err, "Failed to set argument 1\n");
								/***???***/
  err = clSetKernelArg(kernel, 2, sizeof(unsigned int), &count);
  cl_error(err, "Failed to set argument 2\n");
  
  //9.
    // Launch Kernel
  local_size = 128;
  global_size = count; /***<- ???***/
							   /*** ???       ???***/
  err = clEnqueueNDRangeKernel(command_queue, kernel, 1, NULL, &global_size, &local_size, 0, NULL, NULL);
  cl_error(err, "Failed to launch kernel to the device\n");
  
  //10.
    // Read data form device memory back to host memory
							/***???                   ???***/            /***???               ???***/
  err = clEnqueueReadBuffer(command_queue, out_device_object, CL_TRUE, 0, sizeof(float)*count, salida, 0, NULL, NULL);
  cl_error(err, "Failed to enqueue a read command\n");
  
  //11.
	//Comprobamos corrección del algoritmo
  //std::cout << "----Comprobamos resultados----" << std::endl;
  printf("----Comprobamos resultados----");
  for (int indice = 0; indice < count; indice++){
	  float valor_correcto = entrada[indice] * entrada[indice];
	  //std::cout << "Valor calculado: " << salida[indice] << " - Valor esperado: " << valor_correcto << std::endl;
		printf("Valor calculado: %d - Valor esperado: %d.",salida[indice],valor_correcto);
  }
  printf("------------------------------");
  
  //12.
	//Liberamos la memoria usada /***Todo ???***/
  clReleaseMemObject(in_device_object);
  clReleaseMemObject(out_device_object);
  clReleaseProgram(program);
  clReleaseKernel(kernel);
  clReleaseCommandQueue(command_queue);
  clReleaseContext(context);
  
  return 0;
}


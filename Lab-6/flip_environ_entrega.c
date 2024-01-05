////////////////////////////////////////////////////////////////////
//File: basic_environ.c
//
//Description: base file for environment exercises with openCL
//
// 
////////////////////////////////////////////////////////////////////
#define cimg_use_jpeg
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <ctime> // Medir programa entero
#include <iomanip> //setprecision
#include "CImg.h"
#include <iostream>//std::cout
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
  if (argc != 4) {
	  std::cout << "La llamada correcta es: ./flip_host img_salida.jpg img_entrada.jpg {horizontal | vertical} " << std::endl;
	  return -1;
  }

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
    
  clock_t time = clock(); //Inicio de medición del programa

  // 1. Scan the available platforms:
  err = clGetPlatformIDs (num_platforms_ids, platforms_ids, &n_platforms);
  cl_error(err, "Error: Failed to Scan for	 Platforms IDs");
	
  // 2. Scan for devices in each platform
  for (int i = 0; i < n_platforms; i++ ){
    err = clGetDeviceIDs( platforms_ids[i], CL_DEVICE_TYPE_ALL, num_devices_ids, devices_ids[i], &(n_devices[i]));
    cl_error(err, "Error: Failed to Scan for Devices IDs");
  }	



  // 3. Create a context, with a device
  cl_int ePlatID = 0; //Para indicar el índice de la plataforma del contexto
  cl_int eDevID = 0; //Para indicar el índice del dispositivo del contexto (Si queremos usar todos los de la plataforma sería poner solo n_devices[ePlatID], devices_ids[ePlatID] para crear el contexto)
	//Recordar: Un contexto tiene una única plataforma, una plataforma puede tener varios contextos.	
  cl_context_properties properties[] = { CL_CONTEXT_PLATFORM, (cl_context_properties)platforms_ids[ePlatID], 0};
  context = clCreateContext(properties, 1, &devices_ids[ePlatID][eDevID], NULL, NULL, &err);
  cl_error(err, "Failed to create a compute context\n");

  // 4. Create a command queue
  cl_command_queue_properties proprt[] = { CL_QUEUE_PROPERTIES, CL_QUEUE_PROFILING_ENABLE, 0 };
  command_queue = clCreateCommandQueueWithProperties( context, devices_ids[ePlatID][eDevID], proprt, &err);
  cl_error(err, "Failed to create a command queue\n");

	// First Kernel on the device

  //2.
  // Calculate size of the file
  FILE *fileHandler = fopen("kernel_image_flip.cl", "r");
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
    clGetProgramBuildInfo(program, devices_ids[ePlatID][eDevID], CL_PROGRAM_BUILD_STATUS, sizeof(cl_build_status), &estado, &bytesLeidos);
    //printf("%s\n", buffer);
    exit(-1);
  }
  //4.
    // Create a compute kernel with the program we want to run
  cl_kernel kernel = clCreateKernel(program, "image_flip", &err);
  cl_error(err, "Failed to create kernel from the program\n");
	
  //5.
	//Inicializamos los buffers de entrada y salida:
	cimg_library::CImg<unsigned char> img(argv[2]);
	
	
	clock_t tiempo_carga_ejecucion_descarga = clock();
	
	unsigned int columnas = img.width();
	unsigned int filas = img.height();
	std::cout << "Image dimensions: (" << img.width() << "," << img.height() << "," << img.spectrum() << ")" << std::endl; 
	unsigned int spectrum = img.spectrum();
	unsigned int num_elementos = columnas * filas;
	unsigned char pixeles_entrada[num_elementos * spectrum];
	//Cargamos la imagen en el buffer "pixeles_entrada" (por columnas)
	int indice_p = 0; //índice para recorrer el vector "pixeles_entrada"
	if (strcmp(argv[3],"horizontal")==0){
		for (int col = 0; col < columnas; col++){
			for (int fila = 0; fila < filas; fila++){
				for (int color = 0; color < spectrum; color++){
					pixeles_entrada[indice_p] = img(col,fila,0,color);
					indice_p++;
				}
			}
		}
	} else {
		for (int fila = 0; fila < filas; fila++){
			for (int col = 0; col < columnas; col++){
				for (int color = 0; color < spectrum; color++){
					pixeles_entrada[indice_p] = img(col,fila,0,color);
					indice_p++;
				}
			}
		}
	}
	unsigned char pixeles_salida[num_elementos * spectrum];

  //6.
    // Create OpenCL buffer visible to the OpenCl runtime
  cl_mem in_out_device_object  = clCreateBuffer(context, CL_MEM_READ_WRITE, sizeof(unsigned char) * num_elementos * spectrum, NULL, &err);
  cl_error(err, "Failed to create memory buffer at device\n");
  
  //7.
    // Write date into the memory object 
	cl_event host_dev_event; //medir tiempo de paso de memoria HOST->DEVICE
  err = clEnqueueWriteBuffer(command_queue, in_out_device_object, CL_TRUE, 0, sizeof(unsigned char) * num_elementos * spectrum, pixeles_entrada, 0, NULL, &host_dev_event);
  cl_error(err, "Failed to enqueue a write command\n");
  
  //8.
    // Set the arguments to the kernel
  err = clSetKernelArg(kernel, 0, sizeof(cl_mem), &in_out_device_object);
  cl_error(err, "Failed to set argument 0\n");
  if (strcmp(argv[3],"horizontal")==0){
	  err = clSetKernelArg(kernel, 1, sizeof(unsigned int), &filas);
	  cl_error(err, "Failed to set argument 1\n");
	  
	  err = clSetKernelArg(kernel, 2, sizeof(unsigned int), &columnas);
	  cl_error(err, "Failed to set argument 2\n");
  }else {
	  err = clSetKernelArg(kernel, 1, sizeof(unsigned int), &columnas);
	  cl_error(err, "Failed to set argument 1\n");
	
	  err = clSetKernelArg(kernel, 2, sizeof(unsigned int), &filas);
	  cl_error(err, "Failed to set argument 2\n");
  }
  err = clSetKernelArg(kernel, 3, sizeof(unsigned int), &spectrum);
  cl_error(err, "Failed to set argument 3\n");
  err = clSetKernelArg(kernel, 4, sizeof(unsigned int), &num_elementos);
  cl_error(err, "Failed to set argument 4\n");
  
  //9.
    // Launch Kernel
  local_size = 128; 
  if (strcmp(argv[3],"horizontal")==0){
	global_size = (columnas/2) * filas;
  } else {
	global_size = (filas/2) * columnas;
  }
  cl_event kernel_event; //medir tiempo de kernel
  err = clEnqueueNDRangeKernel(command_queue, kernel, 1, NULL, &global_size, NULL, 0, NULL, &kernel_event);
  cl_error(err, "Failed to launch kernel to the device\n");

  
  //10.
  cl_event dev_host_event; //medir tiempo de paso de memoria DEVICE->HOST
    // Read data form device memory back to host memory
  err = clEnqueueReadBuffer(command_queue, in_out_device_object, CL_TRUE, 0, sizeof(unsigned char) * num_elementos * spectrum, &pixeles_salida, 0, NULL, &dev_host_event); // Debido a que bloquea hasta que hace la lectura, asumo que al llegar aquí ya se ha acabado de ejecutar el kernel (y no se ha hecho nada más en el host entre medias).
  cl_error(err, "Failed to enqueue a read command\n");
  tiempo_carga_ejecucion_descarga = clock() - tiempo_carga_ejecucion_descarga;
  std::cout << "Total time of flipping the image: " << std::fixed << std::setprecision(4)
				<< ((float)tiempo_carga_ejecucion_descarga)/CLOCKS_PER_SEC << " seconds = " << ((float)tiempo_carga_ejecucion_descarga) * 1000000000/CLOCKS_PER_SEC << " nanoseconds." << std::endl;
  //11.
	//Comprobamos corrección del algoritmo
  std::cout << "----Comprobamos resultados----" << std::endl;
  cimg_library::CImg<unsigned char> salida(columnas,filas,1,spectrum,0);
  indice_p = 0; //índice para recorrer el vector "pixeles_entrada"
  if (strcmp(argv[3],"horizontal")==0){
	  for (int col = 0; col < columnas; col++){
		for (int fila = 0; fila < filas; fila++){
		  for (int color = 0; color < spectrum; color++){
			salida(col,fila,0,color) = pixeles_salida[indice_p];
			indice_p++;
		  }
		}
	  }
  } else {
	  for (int fila = 0; fila < filas; fila++){
		for (int col = 0; col < columnas; col++){
		  for (int color = 0; color < spectrum; color++){
			salida(col,fila,0,color) = pixeles_salida[indice_p];
			indice_p++;
		  }
		}
	  }
  }
  salida.save(argv[1]);
  printf("Imagen guardada en %s.\n", argv[1]);
  printf("------------------------------\n");
  time = clock() - time;
  
  //11.2
	//Extraemos información de los eventos
  clFinish(command_queue);
  cl_ulong ev_start_time=(cl_ulong)0;     
  cl_ulong ev_end_time=(cl_ulong)0; 
  //kernel_event  //Execution time of the kernel
  err = clWaitForEvents(1, &kernel_event);
  err |= clGetEventProfilingInfo(kernel_event, CL_PROFILING_COMMAND_START, sizeof(cl_ulong), &ev_start_time, NULL);
  err |= clGetEventProfilingInfo(kernel_event, CL_PROFILING_COMMAND_END, sizeof(cl_ulong), &ev_end_time, NULL);
  cl_error(err, "Failed to measure time from kernel_event\n");
  std::cout << "Kernel's time of execution: " << ev_end_time - ev_start_time << " nanoseconds." << std::endl;
  int swaps_por_wi = 12;
  std::cout << "Kernel's bandwith: " << ((float)(sizeof(unsigned char) * global_size * swaps_por_wi)) / (ev_end_time - ev_start_time) << " Bytes/nanosecond." << std::endl;
  
  //host_dev_event (H->D) y dev_host_event (D->H)
  std::cout << "Bytes transferred: " << sizeof(unsigned char) * num_elementos * spectrum << std::endl;
  cl_ulong host_dev_start_time=(cl_ulong)0;     
  cl_ulong host_dev_end_time=(cl_ulong)0;
  err = clWaitForEvents(1, &host_dev_event);
  err |= clGetEventProfilingInfo(host_dev_event, CL_PROFILING_COMMAND_START, sizeof(cl_ulong), &host_dev_start_time, NULL);
  err |= clGetEventProfilingInfo(host_dev_event, CL_PROFILING_COMMAND_END, sizeof(cl_ulong), &host_dev_end_time, NULL);
  cl_error(err, "Failed to measure time from host_dev_event\n");
  std::cout << "Bandwith of the Host->Device: " << static_cast<float>(sizeof(unsigned char) * num_elementos * spectrum) / ((host_dev_end_time - host_dev_start_time)) << " Bytes/nanosecond." << std::endl;
  
  cl_ulong dev_host_start_time=(cl_ulong)0;     
  cl_ulong dev_host_end_time=(cl_ulong)0;
  err = clWaitForEvents(1, &dev_host_event);
  err |= clGetEventProfilingInfo(dev_host_event, CL_PROFILING_COMMAND_START, sizeof(cl_ulong), &dev_host_start_time, NULL);
  err |= clGetEventProfilingInfo(dev_host_event, CL_PROFILING_COMMAND_END, sizeof(cl_ulong), &dev_host_end_time, NULL);
  cl_error(err, "Failed to measure time from dev_host_event\n");
																												// Dividir por 1000000000 hace que salga infinito
  std::cout << "Bandwith of the Device->Host: " << static_cast<float>(sizeof(unsigned char) * num_elementos * spectrum) / ((dev_host_end_time - dev_host_start_time)) << " Bytes/nanoseconds." << std::endl;
  
  //12.
	//Liberamos la memoria usada
  clock_t time2 = clock();
  clReleaseMemObject(in_out_device_object);
  clReleaseProgram(program);
  clReleaseKernel(kernel);
  clReleaseCommandQueue(command_queue);
  clReleaseContext(context);

  time = time + (clock() - time2);
  std::cout << "Total time of execution: " << std::fixed << std::setprecision(4)
				<< ((float)time)/CLOCKS_PER_SEC << " seconds = " << ((float)time) * 1000000000/CLOCKS_PER_SEC << " nanoseconds" << std::endl;
  
  return 0;
}


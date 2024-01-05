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
#include <vector>//overflow de long long int si no
#ifdef __APPLE__
  #include <OpenCL/opencl.h>
#else
  #include <CL/cl.h>
#endif
// Para las 5000 imagenes
#include "Cargador.hpp"
#include "join_threads.hpp"
//
// check error, in such a case, it exits

void cl_error(cl_int code, const char *string){
	if (code != CL_SUCCESS){
		printf("%d - %s\n", code, string);
	    exit(-1);
	}
}
////////////////////////////////////////////////////////////////////////////////

//Función de los threads (el kernel es unitario, por lo que en cada iteración se llamará numCargas veces)
void procesoThread(Cargador& cargador, unsigned int indiceBase, unsigned int cantidadACargar, unsigned char* buffer, 
                    cl_context context, cl_command_queue command_queue, cl_kernel kernel, const int tID){
  std::clock_t c_start = std::clock();

  unsigned int numCargas = cargador.cargaImagenes(indiceBase,cantidadACargar,buffer);
  int err;
  cl_mem in_out_device_object = clCreateBuffer(context, CL_MEM_READ_WRITE, sizeof(unsigned char) * cargador.num_elementos_imagen, NULL, &err);
  cl_error(err, "Failed to build cl_mem object (procesoThread)\n");
  unsigned long long tiempoKernel;
  //tiempoKernel.push_back(0);
  unsigned long long tiempoHostDev;
  //tiempoHostDev.push_back(0);
  unsigned long long int tiempoDevHost;
  //tiempoDevHost.push_back(0);
  unsigned long long int numImagenes;
  int num_elementos = cargador.filas * cargador.columnas;
  //Specify number of WI -> Constant for each iteration
  size_t global_size;
  if (strcmp(cargador.modo,"horizontal")==0){
    std::cout << "Modo Horizontal" << std::endl;
    global_size = (cargador.columnas/2) * cargador.filas;
  } else {
    std::cout << "Modo Vertical" << std::endl;
    global_size = (cargador.filas/2) * cargador.columnas;
  }
  while (numCargas > 0){
    numImagenes += numCargas;
    for(int i = 0; i < numCargas; ++i){
        //Pass image to the kernel
        cl_event host_dev_event; //medir tiempo de paso de memoria HOST->DEVICE
        err = clEnqueueWriteBuffer(command_queue, in_out_device_object, CL_TRUE, 0, sizeof(unsigned char) * cargador.num_elementos_imagen, &buffer[(i + indiceBase) * sizeof(unsigned char) * cargador.num_elementos_imagen], 0, NULL, &host_dev_event);
        //Set Parameters of the kernel
        err = clSetKernelArg(kernel, 0, sizeof(cl_mem), &in_out_device_object);
        cl_error(err, "Failed to set argument 0\n");
        if (strcmp(cargador.modo,"horizontal")==0){
          err = clSetKernelArg(kernel, 1, sizeof(unsigned int), &cargador.filas);
          cl_error(err, "Failed to set argument 1\n");
          
          err = clSetKernelArg(kernel, 2, sizeof(unsigned int), &cargador.columnas);
          cl_error(err, "Failed to set argument 2\n");
        }else {
          err = clSetKernelArg(kernel, 1, sizeof(unsigned int), &cargador.columnas);
          cl_error(err, "Failed to set argument 1\n");
        
          err = clSetKernelArg(kernel, 2, sizeof(unsigned int), &cargador.filas);
          cl_error(err, "Failed to set argument 2\n");
        }
        err = clSetKernelArg(kernel, 3, sizeof(unsigned int), &cargador.spectrum);
        cl_error(err, "Failed to set argument 3\n");
        err = clSetKernelArg(kernel, 4, sizeof(unsigned int), &num_elementos);
        cl_error(err, "Failed to set argument 4\n");
        
        //Launch the kernel
        cl_event kernel_event; //medir tiempo de kernel
        err = clEnqueueNDRangeKernel(command_queue, kernel, 1, NULL, &global_size, NULL, 0, NULL, &kernel_event);
        cl_error(err, "Failed to launch kernel to the device\n");
        //Read the output
        cl_event dev_host_event; //medir tiempo de paso de memoria DEVICE->HOST
        err = clEnqueueReadBuffer(command_queue, in_out_device_object, CL_TRUE,  0, sizeof(unsigned char) * cargador.num_elementos_imagen, &buffer[(i + indiceBase) * sizeof(unsigned char) * cargador.num_elementos_imagen], 0, NULL, &dev_host_event); // Debido a que bloquea hasta que hace la lectura, asumo que al llegar aquí ya se ha acabado de ejecutar el kernel (y no se ha hecho nada más en el host entre medias).
        // We measure the results
        //clFinish(command_queue); //clWaitForEvents just waits for the specific event (what is what we want).
        cl_ulong ev_start_time=(cl_ulong)0;     
        cl_ulong ev_end_time=(cl_ulong)0; 
        //kernel_event  //Execution time of the kernel
        err = clWaitForEvents(1, &kernel_event);
        err |= clGetEventProfilingInfo(kernel_event, CL_PROFILING_COMMAND_START, sizeof(cl_ulong), &ev_start_time, NULL);
        err |= clGetEventProfilingInfo(kernel_event, CL_PROFILING_COMMAND_END, sizeof(cl_ulong), &ev_end_time, NULL);
        tiempoKernel += (ev_end_time - ev_start_time);
        /*tiempoKernel.back() += (ev_end_time - ev_start_time);
        if(tiempoKernel.back() > 140000000000000){
          unsigned long long int sobra = tiempoKernel.back() - 140000000000000;
          tiempoKernel.back() -= sobra;
          tiempoKernel.push_back(sobra);
        }*/
        //host_dev_event //Time of transference HOST->DEV
        cl_ulong host_dev_start_time=(cl_ulong)0;     
        cl_ulong host_dev_end_time=(cl_ulong)0;
        err = clWaitForEvents(1, &host_dev_event);
        err |= clGetEventProfilingInfo(host_dev_event, CL_PROFILING_COMMAND_START, sizeof(cl_ulong), &host_dev_start_time, NULL);
        err |= clGetEventProfilingInfo(host_dev_event, CL_PROFILING_COMMAND_END, sizeof(cl_ulong), &host_dev_end_time, NULL);
        tiempoHostDev += (host_dev_end_time - host_dev_start_time);
        /*tiempoHostDev.back() += (host_dev_end_time - host_dev_start_time);
        if(tiempoHostDev.back() > 140000000000000){
          unsigned long long int sobra = tiempoHostDev.back() - 140000000000000;
          tiempoHostDev.back() -= sobra;
          tiempoHostDev.push_back(sobra);
        }*/
        //dev_host_event //Time of transference DEV->HOST
        cl_ulong dev_host_start_time=(cl_ulong)0;     
        cl_ulong dev_host_end_time=(cl_ulong)0;
        err = clWaitForEvents(1, &dev_host_event);
        err |= clGetEventProfilingInfo(dev_host_event, CL_PROFILING_COMMAND_START, sizeof(cl_ulong), &dev_host_start_time, NULL);
        err |= clGetEventProfilingInfo(dev_host_event, CL_PROFILING_COMMAND_END, sizeof(cl_ulong), &dev_host_end_time, NULL);
        tiempoDevHost += (dev_host_end_time - dev_host_start_time);
        /*tiempoDevHost.back() += (dev_host_end_time - dev_host_start_time);
        if(tiempoDevHost.back() > 140000000000000){
          unsigned long long int sobra = tiempoDevHost.back() - 140000000000000;
          tiempoDevHost.back() -= sobra;
          tiempoDevHost.push_back(sobra);
        }*/
    }
    //Preparamos la siguiente iteración
    numCargas = cargador.cargaImagenes(indiceBase,cantidadACargar,buffer);
  }
  std::clock_t c_end = std::clock();
  std::cout << tID << " Tiempo real: " << 1000 * (c_end - c_start) / CLOCKS_PER_SEC << "ms" << std::endl;

  std::cout << "Tiempo de ejecución de kernel " << tID << " total: " << tiempoKernel << " nanoseconds." << std::endl;
  //std::cout << tID << " Mas " << tiempoKernel.size() - 1 << " veces 140000000000000" << std::endl;
  std::cout << "Numero de imagenes procesadas " << tID << ": " << numImagenes << "." << std::endl;
  int swaps_por_wi = 12;
  std::cout << "Bandwidth del kernel " << tID << ": " << ((float)(sizeof(unsigned char) * global_size * swaps_por_wi * numImagenes)) / tiempoKernel << " Bytes/nanoseconds." << std::endl;
  //std::cout << "El kernel leyó/escribió " << tID << ": " << ((float)(sizeof(unsigned char) * global_size * swaps_por_wi * numImagenes))<< " Bytes." << std::endl;
  std::cout << "Bandwith of the Host->Device " << tID << ": " << static_cast<float>(sizeof(unsigned char) * cargador.num_elementos_imagen * numImagenes) / tiempoHostDev << " Bytes/nanoseconds." << std::endl;
  //std::cout << "Tiempo Host->Device " << tID << ": " << tiempoHostDev.back() << " nanoseconds." << std::endl;
  //std::cout << tID << " Mas " << tiempoHostDev.size() - 1 << " veces 140000000000000" << std::endl;
  //std::cout << "Con Host->Device se han desplazado " << tID << ": " << static_cast<float>(sizeof(unsigned char) * cargador.num_elementos_imagen * numImagenes) << " Bytes." << std::endl;
  std::cout << "Bandwith of the Device->Host " << tID << ": " << static_cast<float>(sizeof(unsigned char) * cargador.num_elementos_imagen * numImagenes) / tiempoDevHost << " Bytes/nanoseconds." << std::endl;
  //std::cout << "Tiempo Device->Host " << tID << ": " << tiempoDevHost.back() << " nanoseconds." << std::endl;
  //std::cout << tID << " Mas " << tiempoDevHost.size() - 1 << " veces 140000000000000" << std::endl;
  //std::cout << "Con Device->Host se han desplazado " << tID << ": " << static_cast<float>(sizeof(unsigned char) * cargador.num_elementos_imagen * numImagenes) << " Bytes." << std::endl;
  clReleaseMemObject(in_out_device_object);
}

int main(int argc, char** argv)
{
  if (argc != 5) {
	  std::cout << "La llamada correcta es: ./flip_host img_salida.jpg img_entrada.jpg {horizontal | vertical} num_imagenes " << std::endl;
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
  cl_context context1, context2;                 				// compute context
  cl_command_queue command_queue1, command_queue2;     				// compute command queue
  
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
  cl_int ePlatID[2] = {1,2}; //Para indicar el índice de la plataforma del contexto -> [CPU,GPU]
  cl_int eDevID[2] = {0,0}; //Para indicar el índice del dispositivo del contexto (Si queremos usar todos los de la plataforma sería poner solo n_devices[ePlatID], devices_ids[ePlatID] para crear el contexto)
	//Recordar: Un contexto tiene una única plataforma, una plataforma puede tener varios contextos.	
  // Contexto 1: Plataforma 1
  cl_context_properties properties[] = { CL_CONTEXT_PLATFORM, (cl_context_properties)platforms_ids[ePlatID[0]], 0};
  context1 = clCreateContext(properties, 1, &devices_ids[ePlatID[0]][eDevID[0]], NULL, NULL, &err);
  cl_error(err, "Failed to create a compute context 1\n");
  // Contexto 2: Plataforma 2
  cl_context_properties properties2[] = { CL_CONTEXT_PLATFORM, (cl_context_properties)platforms_ids[ePlatID[1]], 0};
  context2 = clCreateContext(properties2, 1, &devices_ids[ePlatID[1]][eDevID[1]], NULL, NULL, &err);
  cl_error(err, "Failed to create a compute context 2\n");

  // 4. Create a command queue
  // CommandQueue 1: Plataforma 1
  cl_command_queue_properties proprt[] = { CL_QUEUE_PROPERTIES, CL_QUEUE_PROFILING_ENABLE, 0 };
  command_queue1 = clCreateCommandQueueWithProperties( context1, devices_ids[ePlatID[0]][eDevID[0]], proprt, &err);
  cl_error(err, "Failed to create a command queue 1\n");
  
  // CommandQueue 2: Plataforma 2
  command_queue2 = clCreateCommandQueueWithProperties( context2, devices_ids[ePlatID[1]][eDevID[1]], proprt, &err);
  cl_error(err, "Failed to create a command queue 2\n");

	// First Kernel on the device

  //2.
  // Calculate size of the file
  //FILE *fileHandler = fopen("kernel_image_flip_multiple.cl", "r");
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
  cl_program program1 = clCreateProgramWithSource(context1,1,&constSourceCode,&fileSize,&err);
  cl_program program2 = clCreateProgramWithSource(context2,1,&constSourceCode,&fileSize,&err);
  cl_error(err, "Failed to create program with source\n");
  free(sourceCode);
  
  //3.
    // Build the executable and check errors
  err = clBuildProgram(program1, 0, NULL, NULL, NULL, NULL);
  if (err != CL_SUCCESS){
    size_t len;
    char buffer[2048];

    printf("Error: Some error at building process 1: %d.\n",err);
	//Añadido
	cl_build_status estado;
	size_t bytesLeidos = 0;
    err = clGetProgramBuildInfo(program1, devices_ids[ePlatID[0]][eDevID[0]], CL_PROGRAM_BUILD_STATUS, sizeof(cl_build_status), &estado, &bytesLeidos);
    printf("%d.\n",err);
	//printf("%s\n", buffer);
    exit(-1);
  }
  err = clBuildProgram(program2, 0, NULL, NULL, NULL, NULL);
  if (err != CL_SUCCESS){
    size_t len;
    char buffer[2048];

    printf("Error: Some error at building process 2.\n");
	//Añadido
	cl_build_status estado;
	size_t bytesLeidos = 0;
    clGetProgramBuildInfo(program2, devices_ids[ePlatID[1]][eDevID[1]], CL_PROGRAM_BUILD_STATUS, sizeof(cl_build_status), &estado, &bytesLeidos);
    printf("%s\n", buffer);
    exit(-1);
  }
  //4.
    // Create a compute kernel with the program we want to run
  //cl_kernel kernel1 = clCreateKernel(program1, "images_flip", &err);
  cl_kernel kernel1 = clCreateKernel(program1, "image_flip", &err);
  cl_error(err, "Failed to create kernel1 from the program\n");
  //cl_kernel kernel2 = clCreateKernel(program2, "images_flip", &err);
  cl_kernel kernel2 = clCreateKernel(program2, "image_flip", &err);
  cl_error(err, "Failed to create kernel2 from the program\n");
	
  //5.
	//Inicializamos los buffers de entrada y salida:
	cimg_library::CImg<unsigned char> img(argv[2]);
	int num_imagenes = atoi(argv[4]);
	
	clock_t tiempo_carga_ejecucion_descarga = clock();
	
	unsigned int columnas = img.width();
	unsigned int filas = img.height();
	std::cout << "Image dimensions: (" << img.width() << "," << img.height() << "," << img.spectrum() << ")" << std::endl; 
	unsigned int spectrum = img.spectrum();
	unsigned int num_elementos = columnas * filas;
	std::cout << "Tamaño total de imagenes a flipear: " << num_elementos * spectrum * num_imagenes << " Bytes." << std::endl;
	
  const int tamanoBuffer = 100;
  unsigned char* pixeles_entrada = new unsigned char[tamanoBuffer * num_elementos * spectrum];//[num_elementos * spectrum * num_imagenes];
	//Cargamos la imagen en el buffer "pixeles_entrada" (por columnas)
	int indice_p = 0; //índice para recorrer el vector "pixeles_entrada"
	std::cout << "Iniciamos la carga de imagenes" << std::endl;
	//while(indice_p < num_elementos * spectrum){//
	for(int imagen = 0; imagen < 10; ++imagen){ //La prueba se hace con 5 imágenes cada dispositivo
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
	}
	std::cout << "Se han cargado las imagenes para el balance" << std::endl;
	//unsigned char* pixeles_salida = new unsigned char[tamanoBuffer * num_elementos * spectrum];//[num_elementos * spectrum * num_imagenes];
	//std::cout << "Reservado espacio de salida" << std::endl;

  //6.
    // Create OpenCL buffer visible to the OpenCl runtime
	//Los hago tan grandes como el buffer con todas las imágenes porque no sabemos cómo se van a acabar distribuyendo
  //std::cout << "Buffer 1" << std::endl;
  cl_mem in_out_device_object_1  = clCreateBuffer(context1, CL_MEM_READ_WRITE, sizeof(unsigned char) * num_elementos * spectrum, NULL, &err);
  cl_error(err, "Failed to create memory buffer at device\n");
  //std::cout << "Buffer 2" << std::endl;
  cl_mem in_out_device_object_2  = clCreateBuffer(context2, CL_MEM_READ_WRITE, sizeof(unsigned char) * num_elementos * spectrum, NULL, &err);
  //std::cout << "Buffers done" << std::endl;
  cl_error(err, "Failed to create memory buffer at device\n");
  //exit(-1);
  //7.
  //PRIMERA LLAMADA PARA MEDIR LA CAPACIDAD DE CÓMPUTO DE CADA DEVICE (Lo hacemos con las mismas primeras 5 imágenes
    // Write date into the memory object 
  unsigned int num_imagenes_input = 5;
  unsigned long long int tiempo1 = 0;
  unsigned long long int tiempo2 = 0;
  
  for (int i = 0; i < num_imagenes_input; ++i){

  
    cl_event host_dev_event1; //medir tiempo de paso de memoria HOST->DEVICE
    //std::cout << "Cargo al buffer 1" << std::endl;
    err = clEnqueueWriteBuffer(command_queue1, in_out_device_object_1, CL_TRUE, 0, sizeof(unsigned char) * num_elementos * spectrum, &pixeles_entrada[i * sizeof(unsigned char) * num_elementos * spectrum], 0, NULL, &host_dev_event1);
    cl_error(err, "Failed to enqueue a write command\n");
      // Write date into the memory object 
    cl_event host_dev_event2; //medir tiempo de paso de memoria HOST->DEVICE
    //std::cout << "Cargo al buffer 2" << std::endl;
    err = clEnqueueWriteBuffer(command_queue2, in_out_device_object_2, CL_TRUE, 0, sizeof(unsigned char) * num_elementos * spectrum, &pixeles_entrada[(i+5) * sizeof(unsigned char) * num_elementos * spectrum], 0, NULL, &host_dev_event2);
    cl_error(err, "Failed to enqueue a write command\n");

    //8.
      // Set the arguments to the kernel
    //std::cout << "Intento hacer setArgs" << std::endl;
    err = clSetKernelArg(kernel1, 0, sizeof(cl_mem), &in_out_device_object_1);
    cl_error(err, "Failed to set argument 0\n");
    //std::cout << "setArgs 0-1 hecho" << std::endl;
    err = clSetKernelArg(kernel2, 0, sizeof(cl_mem), &in_out_device_object_2);
    cl_error(err, "Failed to set argument 0\n");
    //std::cout << "setArgs 0-2 hecho" << std::endl;
    if (strcmp(argv[3],"horizontal")==0){
      err = clSetKernelArg(kernel1, 1, sizeof(unsigned int), &filas);
      cl_error(err, "Failed to set argument 1\n");
      
      err = clSetKernelArg(kernel1, 2, sizeof(unsigned int), &columnas);
      cl_error(err, "Failed to set argument 2\n");
      
      err = clSetKernelArg(kernel2, 1, sizeof(unsigned int), &filas);
      cl_error(err, "Failed to set argument 1\n");
      
      err = clSetKernelArg(kernel2, 2, sizeof(unsigned int), &columnas);
      cl_error(err, "Failed to set argument 2\n");
    }else {
      err = clSetKernelArg(kernel1, 1, sizeof(unsigned int), &columnas);
      cl_error(err, "Failed to set argument 1\n");
    
      err = clSetKernelArg(kernel1, 2, sizeof(unsigned int), &filas);
      cl_error(err, "Failed to set argument 2\n");

      err = clSetKernelArg(kernel2, 1, sizeof(unsigned int), &columnas);
      cl_error(err, "Failed to set argument 1\n");
    
      err = clSetKernelArg(kernel2, 2, sizeof(unsigned int), &filas);
      cl_error(err, "Failed to set argument 2\n");
    }
    err = clSetKernelArg(kernel1, 3, sizeof(unsigned int), &spectrum);
    cl_error(err, "Failed to set argument 3\n");
    err = clSetKernelArg(kernel1, 4, sizeof(unsigned int), &num_elementos);
    cl_error(err, "Failed to set argument 4\n");
    err = clSetKernelArg(kernel2, 3, sizeof(unsigned int), &spectrum);
    cl_error(err, "Failed to set argument 3\n");
    err = clSetKernelArg(kernel2, 4, sizeof(unsigned int), &num_elementos);
    cl_error(err, "Failed to set argument 4\n");
    //std::cout << "setArgs hechos" << std::endl;
    if (strcmp(argv[3],"horizontal")==0){
    global_size = (columnas/2) * filas * num_imagenes_input;
    } else {
    global_size = (filas/2) * columnas * num_imagenes_input;
    }
    //9.
      // Launch Kernel  
    //std::cout << "Intento lanzar kernel 1" << std::endl;
    cl_event kernel_event1; //medir tiempo de kernel
    err = clEnqueueNDRangeKernel(command_queue1, kernel1, 1, NULL, &global_size, NULL, 0, NULL, &kernel_event1);
    cl_error(err, "Failed to launch kernel to the device\n");
    
    //std::cout << "Intento lanzar kernel 2" << std::endl;
    cl_event kernel_event2; //medir tiempo de kernel
    err = clEnqueueNDRangeKernel(command_queue2, kernel2, 1, NULL, &global_size, NULL, 0, NULL, &kernel_event2);
    cl_error(err, "Failed to launch kernel to the device\n");

    //std::cout << "kernels lanzados" << std::endl;
    
    clFinish(command_queue1);
    clFinish(command_queue2);
    cl_ulong ev_start_time=(cl_ulong)0;     
    cl_ulong ev_end_time=(cl_ulong)0; 
    //kernel_event1  //Execution time of the kernel
    err = clWaitForEvents(1, &kernel_event1);
    err |= clGetEventProfilingInfo(kernel_event1, CL_PROFILING_COMMAND_START, sizeof(cl_ulong), &ev_start_time, NULL);
    err |= clGetEventProfilingInfo(kernel_event1, CL_PROFILING_COMMAND_END, sizeof(cl_ulong), &ev_end_time, NULL);
    tiempo1 += ev_end_time - ev_start_time;
    //kernel_event2  //Execution time of the kernel
    err = clWaitForEvents(1, &kernel_event2);
    err |= clGetEventProfilingInfo(kernel_event2, CL_PROFILING_COMMAND_START, sizeof(cl_ulong), &ev_start_time, NULL);
    err |= clGetEventProfilingInfo(kernel_event2, CL_PROFILING_COMMAND_END, sizeof(cl_ulong), &ev_end_time, NULL);
    tiempo2 += ev_end_time - ev_start_time;
    //std::cout << "Iteraciones completas" << std::endl;
  }
  std::cout << "Tiempo medio por imagen <1,2>: <" << tiempo1/5 << "," << tiempo2/5 << ">." << std::endl;
  //Medimos el tiempo de trabajo del kernel de media para asignar cantidad de carga (ambos han usado exactamente 5 imágenes):
  unsigned int cargaKernels[2] = {std::round(tamanoBuffer * tiempo2/(tiempo1+tiempo2)),std::round(tamanoBuffer * tiempo1/(tiempo1+tiempo2))}; //Si el kernel uno consume el 30% del tiempo total, y el kernel dos el 70%, el kernel 1 tendrá el 70% de la carga de trabajo.
  std::cout << "División en la carga de trabajo <1,2>: <" << cargaKernels[0] << "," << cargaKernels[1] << ">." << std::endl;
  //unsigned int cargaKernel2 = std::round(100 * tiempo1/(tiempo1+tiempo2));
  //Liberamos los cl_mem (se generarán nuevos dentro de los thread)
  clReleaseMemObject(in_out_device_object_1);
  clReleaseMemObject(in_out_device_object_2);

  //Parámetros para la creación de los thread
  cl_context contexts[2] = {context1, context2};
  cl_command_queue command_queues[2] = {command_queue1, command_queue2}; 
  cl_kernel kernels[2] = {kernel1,kernel2};
  Cargador cargador(std::atoi(argv[4]), img, argv[3]); 
  int indices[2] = {0, cargaKernels[0]-1};
  std::vector<std::thread> _threads;
  for (int i = 0; i < 2; ++i){
      _threads.push_back(std::thread(procesoThread,std::ref(cargador),indices[i],cargaKernels[i],pixeles_entrada,contexts[i],command_queues[i],kernels[i], i));
  }
  { 
      join_threads j(_threads);
  }
  //Guardamos la primera imagen del rango de cada device
  const char* names[2] = {"s1.jpg","s2.jpg"};
  for (int i = 0; i < 2; ++i){
      cimg_library::CImg<unsigned char> salida(img.width(),img.height(),1,img.spectrum(),0);
      int indice_p = indices[i] * filas * columnas * spectrum;
      if (strcmp(argv[3],"horizontal")==0){
          for (int col = 0; col < columnas; col++){
              for (int fila = 0; fila < filas; fila++){
                  for (int color = 0; color < spectrum; color++){
                      salida(col,fila,0,color) = pixeles_entrada[indice_p];
                      indice_p++;
                  }
              }
          }
      } else {
          for (int fila = 0; fila < filas; fila++){
              for (int col = 0; col < columnas; col++){
                  for (int color = 0; color < spectrum; color++){
                      salida(col,fila,0,color) = pixeles_entrada[indice_p];
                      indice_p++;
                  }
              }
          }
      }
      salida.save(names[i]);
  }
  //10.
  //cl_event dev_host_event1; //medir tiempo de paso de memoria DEVICE->HOST
  //cl_event dev_host_event2; //medir tiempo de paso de memoria DEVICE->HOST
    // Read data form device memory back to host memory
  /*
  err = clEnqueueReadBuffer(command_queue1, in_out_device_object_1, CL_TRUE, 0, sizeof(unsigned char) * num_elementos * spectrum * num_imagenes_input, &pixeles_salida, 0, NULL, &dev_host_event1); // Debido a que bloquea hasta que hace la lectura, asumo que al llegar aquí ya se ha acabado de ejecutar el kernel (y no se ha hecho nada más en el host entre medias).
  cl_error(err, "Failed to enqueue a read command\n");
  err = clEnqueueReadBuffer(command_queue2, in_out_device_object_2, CL_TRUE, 0, sizeof(unsigned char) * num_elementos * spectrum * num_imagenes_input, &pixeles_salida, 0, NULL, &dev_host_event2); // Debido a que bloquea hasta que hace la lectura, asumo que al llegar aquí ya se ha acabado de ejecutar el kernel (y no se ha hecho nada más en el host entre medias).
  cl_error(err, "Failed to enqueue a read command\n");
  tiempo_carga_ejecucion_descarga = clock() - tiempo_carga_ejecucion_descarga;
  std::cout << "Total time of flipping the image: " << std::fixed << std::setprecision(4)
				<< ((float)tiempo_carga_ejecucion_descarga)/CLOCKS_PER_SEC << " seconds = " << ((float)tiempo_carga_ejecucion_descarga) * 1000000000/CLOCKS_PER_SEC << " nanoseconds." << std::endl;
  //11.
	//Comprobamos corrección del algoritmo
  std::cout << "----Comprobamos resultados----" << std::endl;
  cimg_library::CImg<unsigned char> salida(columnas,filas,1,spectrum,0);
  indice_p = num_elementos * spectrum; //índice para recorrer el vector "pixeles_salida" --> Solo la segunda imagen
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
  clFinish(command_queue1);
  clFinish(command_queue2);
  cl_ulong ev_start_time=(cl_ulong)0;     
  cl_ulong ev_end_time=(cl_ulong)0; 
  //kernel_event  //Execution time of the kernel
  err = clWaitForEvents(1, &kernel_event1);
  err |= clGetEventProfilingInfo(kernel_event1, CL_PROFILING_COMMAND_START, sizeof(cl_ulong), &ev_start_time, NULL);
  err |= clGetEventProfilingInfo(kernel_event1, CL_PROFILING_COMMAND_END, sizeof(cl_ulong), &ev_end_time, NULL);
  cl_error(err, "Failed to measure time from kernel_event\n");
  std::cout << "1st Kernel's time of execution: " << ev_end_time - ev_start_time << " nanoseconds." << std::endl;
  int swaps_por_wi = 12;
  std::cout << "1st Kernel's bandwith: " << ((float)(sizeof(unsigned char) * global_size * swaps_por_wi)) / (ev_end_time - ev_start_time) << " Bytes/nanosecond." << std::endl;
  //kernel_event
  err = clWaitForEvents(1, &kernel_event2);
  err |= clGetEventProfilingInfo(kernel_event2, CL_PROFILING_COMMAND_START, sizeof(cl_ulong), &ev_start_time, NULL);
  err |= clGetEventProfilingInfo(kernel_event2, CL_PROFILING_COMMAND_END, sizeof(cl_ulong), &ev_end_time, NULL);
  cl_error(err, "Failed to measure time from kernel_event\n");
  std::cout << "2nd Kernel's time of execution: " << ev_end_time - ev_start_time << " nanoseconds." << std::endl;
  std::cout << "2nd Kernel's bandwith: " << ((float)(sizeof(unsigned char) * global_size * swaps_por_wi)) / (ev_end_time - ev_start_time) << " Bytes/nanosecond." << std::endl;
  
  
  //12.
	//Liberamos la memoria usada
  clock_t time2 = clock();
  clReleaseMemObject(in_out_device_object_1);
  clReleaseMemObject(in_out_device_object_2);
  clReleaseProgram(program1);
  clReleaseProgram(program2);
  clReleaseKernel(kernel1);
  clReleaseKernel(kernel2);
  clReleaseCommandQueue(command_queue1);
  clReleaseCommandQueue(command_queue2);
  clReleaseContext(context1);
  clReleaseContext(context2);

  time = time + (clock() - time2);
  std::cout << "Total time of execution: " << std::fixed << std::setprecision(4)
				<< ((float)time)/CLOCKS_PER_SEC << " seconds = " << ((float)time) * 1000000000/CLOCKS_PER_SEC << " nanoseconds" << std::endl;
  */
  return 0;
}


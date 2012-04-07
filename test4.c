#include <CL/cl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

void checkErr(cl_int err, const char *name)
{
  if(err != CL_SUCCESS)
  {
    printf("ERROR: %s(%d)\n", name, err);
    exit(EXIT_FAILURE);
  }
}

int main(int argc, char **argv)
{
  cl_platform_id platform;  
  cl_uint numPlatforms, numEntries;
  cl_int errorcode;
  
  
  //get list of platforms. choose one platform
  numEntries = 1;
  errorcode = clGetPlatformIDs(numEntries, &platform, &numPlatforms);
  checkErr(errorcode, "clGetPlatformIDs");  
  
  cl_device_id device;
  cl_device_type deviceType;
  cl_uint numDevices, numDevicesReturned;
  
  deviceType = CL_DEVICE_TYPE_GPU;
  numEntries = 1;
  numDevices = 1;
  errorcode = clGetDeviceIDs(platform, deviceType, numDevices, &device, &numDevicesReturned);
  checkErr(errorcode, "clGetDeviceIDs");
  numDevices = numDevicesReturned;
  
  //create an OpenCL context
  cl_context_properties context_properties[3] = {CL_CONTEXT_PLATFORM, (cl_context_properties)platform, 0};  
  cl_context context = clCreateContext(context_properties, numDevices, &device, NULL, NULL, &errorcode);
  checkErr(errorcode, "clCreateContext");
  
  //create OpenCL command queue
  cl_command_queue queue = clCreateCommandQueue(context, device, 0, &errorcode);
  checkErr(errorcode, "clCreateCommandQueue");
  
  cl_uint numOpenCLPrograms;
  
  numOpenCLPrograms = 1;
  const char *opencl_program = 
  "__kernel void vecadd(const __global int *a, const __global int *b,__global int *c, const __global int *matdim)\n"
  "{\n"
  "uint gid = get_global_id(0);\n"
  "int dimensions = matdim[0];\n"
  "int numrows=dimensions, numcols=dimensions;\n"
  "int counter1,counter2;\n"
  "int sum;"
  "for(counter1=0; counter1<numcols; counter1++)\n"
  "{\n"
  "sum=0;\n"
  "for(counter2=0; counter2<numcols; counter2++)\n"
  "{\n"
  "sum = sum + a[gid*numcols + counter2]*b[counter2*numcols + counter1];\n"
  "}\n"
  "c[gid*numcols + counter1] = sum;\n"
  "}\n"
  "}\n";
  
  //create OpenCL program from source
  size_t opencl_program_size = strlen(opencl_program);
  cl_program program = clCreateProgramWithSource(context, numOpenCLPrograms, &opencl_program, &opencl_program_size, &errorcode);
  checkErr(errorcode, "clCreateProgramWithSource");
  
  //build OpenCL program
  errorcode = clBuildProgram(program, numDevices, &device, NULL, NULL, NULL);  
  checkErr(errorcode, "clBuildProgram");

  char *execKernelString = "vecadd";
  //create OpenCL kernel from OpenCL program
  cl_kernel kernel = clCreateKernel(program, execKernelString, &errorcode);
  checkErr(errorcode, "clCreateKernel");
  
  //create memory buffers
  cl_mem_flags a_buffer_flags = CL_MEM_READ_ONLY|CL_MEM_COPY_HOST_PTR;
  cl_mem_flags b_buffer_flags = CL_MEM_READ_ONLY|CL_MEM_COPY_HOST_PTR;
  cl_mem_flags c_buffer_flags = CL_MEM_WRITE_ONLY;
  cl_int mat_dims = 100;
  size_t array_sizes = mat_dims * mat_dims;
  size_t a_buffer_size = array_sizes;
  size_t b_buffer_size = array_sizes;
  size_t c_buffer_size = array_sizes;
  int *a, *b, *c;
  a = (int *) calloc(a_buffer_size, sizeof(int));
  b = (int *) calloc(b_buffer_size, sizeof(int)); 
  //initialize a and b
  int counter;  
  printf("\nContents of array A:\n");
  for(counter=0; counter<a_buffer_size; counter++)
  {
    a[counter] = 1;
    printf("%d ", a[counter]);
  }
  printf("\n");
  
  printf("Contents of array B:\n");
  for(counter=0; counter<b_buffer_size; counter++)
  {
    b[counter] = 1;
    printf("%d ", b[counter]);
  }
  printf("\n");
  
  cl_mem a_buffer = clCreateBuffer(context, a_buffer_flags, a_buffer_size*sizeof(int), (void *)a, &errorcode);
  checkErr(errorcode, "clCreateBuffer(a)");
  cl_mem b_buffer = clCreateBuffer(context, b_buffer_flags, b_buffer_size*sizeof(int), (void *)b, &errorcode);
  checkErr(errorcode, "clCreateBuffer(b)");
  cl_mem c_buffer = clCreateBuffer(context, c_buffer_flags, c_buffer_size*sizeof(int), NULL, &errorcode);
  checkErr(errorcode, "clCreateBuffer(c)");
  cl_mem matdim_buffer = clCreateBuffer(context, CL_MEM_READ_ONLY|CL_MEM_COPY_HOST_PTR, sizeof(cl_int), (void *) &mat_dims, &errorcode);
  checkErr(errorcode, "clCreateBuffer(mat_dims)");
  
  //create kernel arguments
  clSetKernelArg(kernel, 0, sizeof(a_buffer), (void *) &a_buffer);
  clSetKernelArg(kernel, 1, sizeof(b_buffer), (void *) &b_buffer);
  clSetKernelArg(kernel, 2, sizeof(c_buffer), (void *) &c_buffer);
  clSetKernelArg(kernel, 3, sizeof(matdim_buffer), (void *) &matdim_buffer);
  
  //enqueue kernel for execution
  cl_uint global_work_dim;
  size_t global_work_size;
  cl_event addKernelEvent;
  
  global_work_dim = 1;
  global_work_size = mat_dims;
  errorcode = clEnqueueNDRangeKernel(queue, kernel, global_work_dim, NULL, &global_work_size, NULL, 0, NULL, &addKernelEvent);
  checkErr(errorcode, "clEnqueueNDRangeKernel");
  
  //waits for all the jobs in that queue to finish
  clFinish(queue);
  
  //read array c from OpenCL device memory. map buffer to host memory
  cl_event memReadEvent;
  c = (int *) clEnqueueMapBuffer(queue, c_buffer, CL_TRUE, CL_MAP_READ, 0, c_buffer_size*sizeof(int), 0, NULL, &memReadEvent, &errorcode);
  checkErr(errorcode, "clEnqueueMapBuffer");
  
  printf("\nContents of array C:\n");
  for(counter=0; counter<c_buffer_size; counter++)
  { printf("%d ", c[counter]); }
  printf("\n");  
  
  return 0;
}
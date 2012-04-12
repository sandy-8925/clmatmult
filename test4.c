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
  
  deviceType = CL_DEVICE_TYPE_CPU;
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
  "__kernel void vecadd(__global const int *a, __global const int *b, __global int *c, __global const int *matdims)\n"
  "{\n"
  "__private uint gid = get_global_id(0);\n"
  "__private int counter1,counter2;\n"
  "__private int sum;\n"
  "int dim1,dim2,dim3;\n"
  
  "dim1 = matdims[0];\n"
  "dim2 = matdims[1];\n"
  "dim3 = matdims[2];\n"
  "for(counter1=0; counter1<dim3; counter1++)\n"
  "{\n"
  "sum=0;\n"
  "for(counter2=0; counter2<dim2; counter2++)\n"  
  "{ sum = sum + a[gid*dim2 + counter2]*b[counter2*dim3 + counter1]; }\n"  
  "c[gid*dim3 + counter1] = sum;\n"
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
  cl_int dim1, dim2, dim3;
  dim1 = 20;
  dim2 = 30;
  dim3 = 3;
  cl_int mat_dims[3] = {dim1, dim2, dim3};
  cl_mem_flags dim_buffer_flags = CL_MEM_READ_ONLY|CL_MEM_COPY_HOST_PTR;  
  size_t a_buffer_size = dim1 * dim2;
  size_t b_buffer_size = dim2 * dim3;
  size_t c_buffer_size = dim1 * dim3;
  size_t dim_buffer_size = sizeof(mat_dims);
  cl_int *a, *b, *c;
  a = (cl_int *) calloc(a_buffer_size, sizeof(cl_int));
  b = (cl_int *) calloc(b_buffer_size, sizeof(cl_int));
  //initialize a and b
  unsigned int counter;  
  printf("Contents of array A:\n");
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
  
  cl_mem a_buffer = clCreateBuffer(context, a_buffer_flags, a_buffer_size*sizeof(cl_int), (void *)a, &errorcode);
  checkErr(errorcode, "clCreateBuffer(a)");
  cl_mem b_buffer = clCreateBuffer(context, b_buffer_flags, b_buffer_size*sizeof(cl_int), (void *)b, &errorcode);
  checkErr(errorcode, "clCreateBuffer(b)");
  cl_mem c_buffer = clCreateBuffer(context, c_buffer_flags, c_buffer_size*sizeof(cl_int), NULL, &errorcode);
  checkErr(errorcode, "clCreateBuffer(c)");
  cl_mem dim_buffer;
  dim_buffer = clCreateBuffer(context, dim_buffer_flags, dim_buffer_size, (void *) mat_dims, &errorcode);
  checkErr(errorcode, "clCreateBuffer(dim_buffer)");
  
  //create kernel arguments
  clSetKernelArg(kernel, 0, sizeof(a_buffer), (void *) &a_buffer);
  clSetKernelArg(kernel, 1, sizeof(b_buffer), (void *) &b_buffer);
  clSetKernelArg(kernel, 2, sizeof(c_buffer), (void *) &c_buffer);
  clSetKernelArg(kernel, 3, sizeof(dim_buffer), (void *) &dim_buffer);
  
  //enqueue kernel for execution
  cl_uint global_work_dim;
  size_t global_work_size;
  cl_event addKernelEvent;
  
  global_work_dim = 1;
  global_work_size = dim1;
  errorcode = clEnqueueNDRangeKernel(queue, kernel, global_work_dim, NULL, &global_work_size, NULL, 0, NULL, &addKernelEvent);
  checkErr(errorcode, "clEnqueueNDRangeKernel");  
  
  //read array c from OpenCL device memory. map buffer to host memory
  cl_event memReadEvent;
  c = (cl_int *) clEnqueueMapBuffer(queue, c_buffer, CL_TRUE, CL_MAP_READ, 0, c_buffer_size*sizeof(cl_int), 0, NULL, &memReadEvent, &errorcode);
  checkErr(errorcode, "clEnqueueMapBuffer");
  
  //waits for all the jobs in that queue to finish
  clFinish(queue);
  
  printf("\nContents of array C:\n");
  for(counter=0; counter<c_buffer_size; counter++)
  { printf("%d ", c[counter]); }
  printf("\n");  
  
  return 0;
}
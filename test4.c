#include <CL/cl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

cl_int *a, *b, *c;
cl_uint *rowinfo;


void checkErr(cl_int err, const char *name)
{
  if(err != CL_SUCCESS)
  {
    printf("ERROR: %s(%d)\n", name, err);
    exit(EXIT_FAILURE);
  }
}

void thatsAllFolks()
{
  if(a)  free(a);
  if(b)  free(b);  
  if(rowinfo)  free(rowinfo);
}

int main(int argc, char **argv)
{
  cl_platform_id platform;
  cl_uint numPlatforms, numEntries, numDevices, numDevicesReturned;
  cl_int dim1, dim2, dim3, errorcode;
  // global work size - number of work items
  size_t global_work_size;
  cl_device_id device;
  cl_device_type deviceType;  
  
  a = b = NULL;
  rowinfo = NULL;
  c = NULL;
  
  //set defaults for variables
  dim1 = 100;
  dim2 = 100;
  dim3 = 100;
  global_work_size = 5;
  deviceType = CL_DEVICE_TYPE_GPU;
  
  //check commandline arguments and process accordingly
  if(argc >= 2)
  {    
    if(atoi(argv[1]) > 0)
    { global_work_size = atoi(argv[1]); }
  }
  if(argc >= 5)
  {
    if(atoi(argv[2]) > 0)
    { global_work_size = atoi(argv[2]); }
    if(atoi(argv[3]) > 0)
    { global_work_size = atoi(argv[3]); }
    if(atoi(argv[4]) > 0)
    { global_work_size = atoi(argv[4]); }
  }
  
  //get list of platforms. choose one platform
  numEntries = 1;
  errorcode = clGetPlatformIDs(numEntries, &platform, &numPlatforms);
  checkErr(errorcode, "clGetPlatformIDs");
    
  numEntries = 1;
  numDevices = 1;
  errorcode = clGetDeviceIDs(platform, deviceType, numDevices, &device, &numDevicesReturned);
  checkErr(errorcode, "clGetDeviceIDs");
  
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
  "__kernel void vecadd(__global const int *a, __global const int *b, __global int *c, __global const int *matdims, __global const uint *rowinfo)\n"
  "{\n"
  "__private uint gid = get_global_id(0);\n"
  "__private uint startingrownum = rowinfo[gid*2];\n"
  "__private uint numrows = rowinfo[gid*2 + 1];\n"
  "__private int counter1, counter2, counter3;\n"
  "__private int sum;\n"
  "int dim1,dim2,dim3;\n"
  "dim1 = matdims[0];\n"
  "dim2 = matdims[1];\n"
  "dim3 = matdims[2];\n"
  "for(counter3=startingrownum; counter3<startingrownum + numrows; counter3++)\n"
  "{\n"
  "for(counter1=0; counter1<dim3; counter1++)\n"
  "{\n"
  "sum=0;\n"
  "for(counter2=0; counter2<dim2; counter2++)\n"
  "{ sum = sum + a[counter3*dim2 + counter2]*b[counter2*dim3 + counter1]; }\n"
  "c[counter3*dim3 + counter1] = sum;\n"
  "}\n"
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
  
  cl_int mat_dims[3] = {dim1, dim2, dim3};
  cl_mem_flags dim_buffer_flags = CL_MEM_READ_ONLY|CL_MEM_COPY_HOST_PTR;
  size_t a_buffer_size = dim1 * dim2;
  size_t b_buffer_size = dim2 * dim3;
  size_t c_buffer_size = dim1 * dim3;
  size_t dim_buffer_size = sizeof(mat_dims);
  
  a = (cl_int *) calloc(a_buffer_size, sizeof(cl_int));
  b = (cl_int *) calloc(b_buffer_size, sizeof(cl_int));
  
  if(!a || !b)
  {
    printf("Error: unable to allocate memory for matrices\n");
    printf("Exiting....\n");
    thatsAllFolks();
    return -1;
  }
  
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
  
  /* distribute work equally among work items */
  
  rowinfo = (cl_uint *) calloc(global_work_size, sizeof(cl_uint)*2);
  cl_uint *temp, startingrownum;
  if(rowinfo == NULL)
  {
    printf("Error: unable to allocate memory for rowinfo matrix\nExiting...\n");
    thatsAllFolks();
    return -2;
  }
  
  size_t rowinfo_buffer_size = global_work_size * sizeof(cl_uint) * 2;
  cl_mem_flags rowinfo_buffer_flags = CL_MEM_READ_ONLY|CL_MEM_COPY_HOST_PTR;  
  cl_uint workperitem = (cl_uint) ceil(dim1/global_work_size), remainder;
  temp = rowinfo;
  startingrownum = 0;
  for(counter=0; counter<global_work_size; counter++)
  {
    *temp = startingrownum;
    temp++;
    *temp = workperitem;
    startingrownum += workperitem;
    temp++;
  }
  temp--;
  remainder = dim1 - workperitem*(global_work_size-1);
  *temp = remainder;
  temp = NULL;
  
  
  cl_mem a_buffer = clCreateBuffer(context, a_buffer_flags, a_buffer_size*sizeof(cl_int), (void *)a, &errorcode);
  checkErr(errorcode, "clCreateBuffer(a)");
  cl_mem b_buffer = clCreateBuffer(context, b_buffer_flags, b_buffer_size*sizeof(cl_int), (void *)b, &errorcode);
  checkErr(errorcode, "clCreateBuffer(b)");
  cl_mem c_buffer = clCreateBuffer(context, c_buffer_flags, c_buffer_size*sizeof(cl_int), NULL, &errorcode);
  checkErr(errorcode, "clCreateBuffer(c)");  
  cl_mem dim_buffer = clCreateBuffer(context, dim_buffer_flags, dim_buffer_size, (void *) mat_dims, &errorcode);
  checkErr(errorcode, "clCreateBuffer(dim_buffer)");
  cl_mem rowinfo_buffer = clCreateBuffer(context, rowinfo_buffer_flags, rowinfo_buffer_size, (void *) rowinfo, &errorcode);
  checkErr(errorcode, "clCreateBuffer(rowinfo_buffer)");
  
  //create kernel arguments
  clSetKernelArg(kernel, 0, sizeof(a_buffer), (void *) &a_buffer);
  clSetKernelArg(kernel, 1, sizeof(b_buffer), (void *) &b_buffer);
  clSetKernelArg(kernel, 2, sizeof(c_buffer), (void *) &c_buffer);
  clSetKernelArg(kernel, 3, sizeof(dim_buffer), (void *) &dim_buffer);
  clSetKernelArg(kernel, 4, sizeof(rowinfo_buffer), (void *) &rowinfo_buffer);
  
  //enqueue kernel for execution
  cl_uint global_work_dim;  
  cl_event addKernelEvent;
  
  global_work_dim = 1;
  errorcode = clEnqueueNDRangeKernel(queue, kernel, global_work_dim, NULL, &global_work_size, NULL, 0, NULL, &addKernelEvent);
  checkErr(errorcode, "clEnqueueNDRangeKernel");
  
  //read array c from OpenCL device memory. map buffer to host memory
  cl_event memReadEvent;
  c = (cl_int *) clEnqueueMapBuffer(queue, c_buffer, CL_TRUE, CL_MAP_READ, 0, c_buffer_size*sizeof(cl_int), 0, NULL, &memReadEvent, &errorcode);
  checkErr(errorcode, "clEnqueueMapBuffer");
  
  //waits for all the jobs in that queue to finish
  clFinish(queue);
  
  //TODO: check if c is NULL
  if(c == NULL)
  {
    printf("Error: pointer to results is NULL\nExiting...\n");
    thatsAllFolks();
    return 0;
  }
  
  printf("\nContents of array C:\n");
  for(counter=0; counter<c_buffer_size; counter++)
  { printf("%d ", c[counter]); }
  printf("\n");  
  
  thatsAllFolks();
  
  return 0;
}


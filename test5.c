#include <CL/cl.h>
#include <stdio.h>

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
  
  cl_uint numPlatforms, numEntries=1;
  cl_int errorcode;
  
  
  //get list of platforms. choose one platform
  errorcode = clGetPlatformIDs(numEntries, &platform, &numPlatforms);
  checkErr(errorcode, "clGetPlatformIDs");
  printf("Number of available OpenCL platforms is %d\n", numPlatforms);
  
  char openclVersionString[100];
  cl_platform_info param = CL_PLATFORM_VERSION;
  errorcode = clGetPlatformInfo(platform, param, sizeof(openclVersionString), openclVersionString, NULL);
  checkErr(errorcode, "clGetPlatformInfo");
  printf("OpenCL version string: %s\n", openclVersionString);
  
  cl_device_id device;
  cl_device_type deviceType;
  cl_uint numDevices;
  
  //get list of devices that can be used. device type and number can be chosen
  //get total number of all OpenCL devices
  deviceType = CL_DEVICE_TYPE_ALL;
  numEntries = 0;
  errorcode = clGetDeviceIDs(platform, deviceType, 0, NULL, &numDevices);
  checkErr(errorcode, "clGetDeviceIDs");
  printf("Total number of OpenCL devices is %d\n", numDevices);
  
  deviceType = CL_DEVICE_TYPE_GPU;
  numEntries = 1;
  errorcode = clGetDeviceIDs(platform, deviceType, 0, NULL, &numDevices);
  checkErr(errorcode, "clGetDeviceIDs");
  printf("Number of GPU devices is %d\n", numDevices);
  
  //get number of GPU devices
  deviceType = CL_DEVICE_TYPE_GPU;
  errorcode = clGetDeviceIDs(platform, deviceType, 0, NULL, &numDevices);
  checkErr(errorcode, "clGetDeviceIDs");
  printf("Number of GPU devices is %d\n", numDevices);
  
  //get GPU device info
  printf("GPU device info\n");
  deviceType = CL_DEVICE_TYPE_GPU;
  numEntries = 1;
  device = NULL;
  errorcode = clGetDeviceIDs(platform, deviceType, numEntries, &device, &numDevices);
  checkErr(errorcode, "clGetDeviceIDs");
  if(device != NULL)  
  {
    cl_uint info;
    size_t actual_return_size;
    
    //get OpenCL device vendor ID
    errorcode = clGetDeviceInfo(device, CL_DEVICE_VENDOR_ID, sizeof(info), &info, &actual_return_size);
    checkErr(errorcode, "clGetDeviceInfo");
    printf("Vendor ID: %d\n", info);
    
    //get OpenCL device vendor
    char stringinfo[1000];
    errorcode = clGetDeviceInfo(device, CL_DEVICE_VENDOR, sizeof(stringinfo), stringinfo, &actual_return_size);
    checkErr(errorcode, "clGetDeviceInfo");
    printf("Vendor: %s\n", stringinfo);
    
    //get OpenCL device name
    errorcode = clGetDeviceInfo(device, CL_DEVICE_NAME, sizeof(stringinfo), stringinfo, &actual_return_size);
    checkErr(errorcode, "clGetDeviceInfo");
    printf("Device name: %s\n", stringinfo);
    
    //get OpenCL device maximum number of compute units
    errorcode = clGetDeviceInfo(device, CL_DEVICE_MAX_COMPUTE_UNITS, sizeof(info), &info, &actual_return_size);
    checkErr(errorcode, "clGetDeviceInfo");
    printf("Maximum number of compute units: %d\n", info);
    
    //get OpenCL device extensions
    errorcode = clGetDeviceInfo(device, CL_DEVICE_EXTENSIONS, sizeof(stringinfo), stringinfo, &actual_return_size);
    checkErr(errorcode, "clGetDeviceInfo");
    printf("Device OpenCL extensions: %s\n", stringinfo);
    
    //get OpenCL device extensions
    errorcode = clGetDeviceInfo(device, CL_DEVICE_MAX_CONSTANT_ARGS, sizeof(info), &info, &actual_return_size);
    checkErr(errorcode, "clGetDeviceInfo");
    printf("Device max num constant args: %u\n", info);
  }
  
  //get GPU device info
  printf("GPU device info\n");
  deviceType = CL_DEVICE_TYPE_GPU;
  numEntries = 1;
  device = NULL;
  errorcode = clGetDeviceIDs(platform, deviceType, numEntries, &device, &numDevices);
  checkErr(errorcode, "clGetDeviceIDs");
  if(device != NULL)  
  {
    cl_uint info;
    size_t actual_return_size;
    
    //get OpenCL device vendor ID
    errorcode = clGetDeviceInfo(device, CL_DEVICE_VENDOR_ID, sizeof(info), &info, &actual_return_size);
    checkErr(errorcode, "clGetDeviceInfo");
    printf("Vendor ID: %u\n", info);
    
    //get OpenCL device vendor
    char stringinfo[1000];
    errorcode = clGetDeviceInfo(device, CL_DEVICE_VENDOR, sizeof(stringinfo), stringinfo, &actual_return_size);
    checkErr(errorcode, "clGetDeviceInfo");
    printf("Vendor: %s\n", stringinfo);
    
    //get OpenCL device name
    errorcode = clGetDeviceInfo(device, CL_DEVICE_NAME, sizeof(stringinfo), stringinfo, &actual_return_size);
    checkErr(errorcode, "clGetDeviceInfo");
    printf("Device name: %s\n", stringinfo);
    
    //get OpenCL device maximum number of compute units
    errorcode = clGetDeviceInfo(device, CL_DEVICE_MAX_COMPUTE_UNITS, sizeof(info), &info, &actual_return_size);
    checkErr(errorcode, "clGetDeviceInfo");
    printf("Maximum number of compute units: %d\n", info);    
    
    //get OpenCL device maximum workgroup size
    size_t sizeinfo;
    errorcode = clGetDeviceInfo(device, CL_DEVICE_MAX_WORK_GROUP_SIZE, sizeof(sizeinfo), &sizeinfo, &actual_return_size);
    checkErr(errorcode, "clGetDeviceInfo");
    printf("Maximum workgroup size: %d\n", sizeinfo);
    
    //get OpenCL device extensions
    errorcode = clGetDeviceInfo(device, CL_DEVICE_EXTENSIONS, sizeof(stringinfo), stringinfo, &actual_return_size);
    checkErr(errorcode, "clGetDeviceInfo");
    printf("Device OpenCL extensions: %s\n", stringinfo);
    
    //get OpenCL device extensions
    errorcode = clGetDeviceInfo(device, CL_DEVICE_MAX_CONSTANT_ARGS, sizeof(info), &info, &actual_return_size);
    checkErr(errorcode, "clGetDeviceInfo");
    printf("Device max num constant args: %u\n", info);
  }
    
  //create context
  //cl_context context = clCreateContext(NULL, );
  
  return 0;
}

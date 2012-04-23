#include <stdio.h>
#include <math.h>
#include <stdlib.h>

int main(int argc, char *argv[])
{
  unsigned int dim1,dim2,dim3, *rowinfo, *temp, startingrownum, counter;
  dim1 = dim2 = dim3 = 30;
  size_t numworkitems;
  //set global work size
  numworkitems = 10;
  
  if(argc >= 5)
  {
    numworkitems = atoi(argv[1]);
    dim1 = atoi(argv[2]);
    dim2 = atoi(argv[3]);
    dim3 = atoi(argv[4]);
  }
  
  rowinfo = (unsigned int *) calloc(numworkitems, sizeof(unsigned int)*2);
  
  unsigned int workperitem = (unsigned int) ceil(dim1/numworkitems), remainder;
  temp = rowinfo;
  startingrownum = 0;
  for(counter=0; counter<numworkitems; counter++)
  {
    *temp = startingrownum;
    temp++;
    *temp = workperitem;
    startingrownum += workperitem;
    temp++;
  }
  temp--;
  remainder = dim1 - workperitem*(numworkitems-1);
  *temp = remainder;
  temp = NULL;

  printf("\n");
  temp = rowinfo;
  for(counter=0; counter<numworkitems; counter++)
  {
    printf("Element %u: %u %u\n", counter, *temp, *(temp+1));
    temp+=2;
  }
  
  return 0;
}
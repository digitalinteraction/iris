#include <stdio.h>
#include "gray.c"
 
int main()
{
  printf("%d\n", gimp_image.pixel_data[80000]);
  return 0;
}

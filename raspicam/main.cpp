


 
#include <iostream>
#include "Image_Capture.h"
 
int main()
{
  std::cout << "Hello World!" << std::endl;
  Buffer *buffer = new Buffer(32);
  Image_Capture *cap = new Image_Capture(buffer);
  
  //cap->run();
  
  
  return 0;
}




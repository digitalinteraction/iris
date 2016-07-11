



#include <iostream>
#include <thread>
#include <chrono>
#include "Image_Capture.h"
#include "../network/NetworkControl.h"
//#include "Low_Res_Worker.h"
//#include "High_Res_Worker.h"

using namespace std;

int main() {

    cout << "OpenCV version : " << CV_VERSION << endl;

    NetworkControl *nc = new NetworkControl();
    
    Buffer *buffer = new Buffer(32);
    Low_Res_Worker *low = new Low_Res_Worker(nc->image_in, nc);
    Image_Capture *cap = new Image_Capture(buffer, low);
    High_Res_Worker *high = new High_Res_Worker(buffer, nc->image_in, nc->image_out);
    cap->capturing = 1;
    low->processing = 1;
    high->processing = 1;
    
    std::thread net_work(&NetworkControl::run, nc);
    std::thread low_work(&Low_Res_Worker::run, low);
    std::thread high_work(&High_Res_Worker::run, high);
    std::thread img_cap(&Image_Capture::run, cap);
    
    
    
    for (int i = 0; i < 1000; i++) {
        int begin = low->counter;
        std::this_thread::sleep_for(std::chrono::seconds(1));
        begin = low->counter - begin;
        printf("%d Frames per Second: %d, Buffer Size: %d\n", i,begin, buffer->curr_size);
        fflush(stdout);
    }

    cap->capturing = 0;
    low->processing = 0;
    high->processing = 0;
    img_cap.join();
    low_work.join();
    high_work.join();
    
    printf("run finished\n");


    //cap->run();

    return 0;
}




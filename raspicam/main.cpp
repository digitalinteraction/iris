



#include <iostream>
#include <thread>
#include <chrono>
#include "Image_Capture.h"
#include "../network/NetworkControl.h"
#include "Low_Res_Worker.h"
//#include "Buffer.h"
#include "High_Res_Worker.h"

using namespace std;

int main() {

    cout << "OpenCV version : " << CV_VERSION << endl;

    NetworkControl *nc = new NetworkControl();
    
    Buffer *buf_ic_hr = new Buffer(10);
    Buffer *buf_low_req = new Buffer(10);
    Buffer *buf_low_img = new Buffer(10);
    Low_Res_Worker *low = new Low_Res_Worker(nc->unrel_in, nc, buf_low_img, buf_low_req);
    Image_Capture *cap = new Image_Capture(buf_ic_hr, buf_low_img, buf_low_req);
    High_Res_Worker *high = new High_Res_Worker(buf_ic_hr, nc->unrel_in, nc->image_out, nc);
    cap->capturing = 1;
    low->processing = 1;
    high->processing = 1;
    
    std::thread net_work(&NetworkControl::run_inf, nc);
    std::thread low_work(&Low_Res_Worker::run, low);
    std::thread high_work(&High_Res_Worker::run, high);
    std::thread img_cap(&Image_Capture::run, cap);
    
    
    
    for (int i = 0; i < 1000; i++) {
        int begin = low->counter;
        std::this_thread::sleep_for(std::chrono::seconds(1));
        begin = low->counter - begin;
        printf("%d Frames per Second: %d, Buffer Size: %d\n", i,begin, buf_ic_hr->getSize());
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




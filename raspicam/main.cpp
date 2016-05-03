



#include <iostream>
#include <thread>
#include <chrono>
#include "Image_Capture.h"
#include "Low_Res_Worker.h"

int main() {
    Buffer *buffer = new Buffer(32);
    Image_Capture *cap = new Image_Capture(buffer);
    Low_Res_Worker *low = new Low_Res_Worker(buffer);
    cap->capturing = 1;
    low->processing = 1;
    std::thread img_cap(&Image_Capture::run, cap);
    std::thread low_work(&Low_Res_Worker::run, low);
    for (int i = 0; i < 60; i++) {
        int begin = low->counter;
        std::this_thread::sleep_for(std::chrono::seconds(1));
        begin = low->counter - begin;
        printf("%d Frames per Second: %d\n", i,begin);
        fflush(stdout);
    }

    cap->capturing = 0;
    low->processing = 0;
    img_cap.join();
    low_work.join();
    printf("run finished\n");


    //cap->run();

    return 0;
}




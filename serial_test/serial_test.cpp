/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */

/* 
 * File:   serial_test.cpp
 * Author: tobias
 *
 * Created on June 23, 2016, 4:09 PM
 */


#include "../network/NetworkControl.h"


using namespace std;



/*
 * REDO serial_test to be a class, maybe Control_network or something
 */
int main(int argc, char** argv) {
    
    NetworkControl *nc = new NetworkControl();
    
    while(1){
        nc->run();
    }
    return 0;
}


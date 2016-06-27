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

#include <cstdlib>
#include "../network/SerialCon.h"

using namespace std;

/*
 * 
 */
int main(int argc, char** argv) {
    SerialCon *con = new SerialCon();
    con->processing = 1;
    con->slip_run();
    return 0;
}


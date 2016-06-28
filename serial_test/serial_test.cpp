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
#include "../network/ReliableTransfer.h"

using namespace std;

/*
 * 
 */
int main(int argc, char** argv) {
    
    UnreliableTransfer *unrel;
    ReliableTransfer *rel = new ReliableTransfer(unrel);
    Topology *topo = new Topology(unrel);
    DebugTransfer *debug = new DebugTransfer();
    
    unrel = new UnreliableTransfer(rel, topo, debug); 
    
    while(true){
        rel->check_timeouts();
        topo->send();
        sleep(0.25);
    }
    return 0;
}


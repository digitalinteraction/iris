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
#include <unistd.h>

#include "../network/ReliableTransfer.h"
#include "../network/DebugTransfer.h"
#include "../network/Topology.h"

#include "../network/UnreliableTransfer.h"


using namespace std;

/*
 * 
 */
int main(int argc, char** argv) {
    UnreliableTransfer *unrel;
    ReliableTransfer *rel = new ReliableTransfer(&unrel);
    Topology *topo = new Topology(&unrel);
    DebugTransfer *debug = new DebugTransfer();
    unrel = new UnreliableTransfer(rel, topo, debug);

    sleep(2);
    while(true){
        rel->check_timeouts();
        topo->send();
        sleep(2);
    }
    
    unrel->serial_comm->join();
    return 0;
}


/* 
        ADS1115_sample.c - 12/9/2013. Written by David Purdie as part of the openlabtools initiative
        Initiates and reads a single sample from the ADS1115 (without error handling)
 */

#include <stdio.h>
#include <fcntl.h>// open
#include <inttypes.h>  // uint8_t, etc
#include <linux/i2c-dev.h> // I2C bus definitions
#include <time.h>

#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <termios.h>

uint8_t writeBuf[3]; // Buffer to store the 3 bytes that we write to the I2C device
uint8_t readBuf[2]; // 2 byte buffer to store the data read from the I2C device

struct mapping{
    float res;
    float force;
};

struct mapping map[12] = {
    {0,0},
    {20, 30},
    {50, 10},
    {100, 6},
    {250, 3.5},
    {500, 2},
    {1000, 1.2},
    {2000, 0.7},
    {4000, 0.45},
    {7000, 0.3},
    {10000, 0.25},
    {100000, 0}};


float calc_force(float res){
    
    if(res < 0) res = 0;
    
    float force1 = 0;
    float force2 = 0;
    float res1 = 0;
    float res2 = 0;
    uint8_t i = 0;
    while(i < 12){
        if(map[i].res > res){
            force1 = map[i-1].force;
            force2 = map[i].force;
            res1 = map[i-1].res;
            res2 = map[i].res;
            i = 12;
        }
        i++;
    }
    float diffres = res2 - res1;
    float factor = (res - res1)/diffres;
    float force = force1 + (force2 - force1)*factor;
    return force;
}


int16_t read_channel(int I2CFile, uint8_t sel) {

    
    int16_t val;
    writeBuf[0] = 1; 
   
    //0b1100 [001] 1 -> 001 (4.096V) -> 000 (6.144V)
    switch (sel) {
        case 0: writeBuf[1] = 0b11000011;
            break;
        case 1: writeBuf[1] = 0b11010011;
            break;
        case 2: writeBuf[1] = 0b11100011;
            break;
        case 3: writeBuf[1] = 0b11110011;
            break;
    }
    writeBuf[2] = 0b11100011; 

    readBuf[0] = 0;
    readBuf[1] = 0;

    write(I2CFile, writeBuf, 3);

    while ((readBuf[0] & 0x80) == 0) 
    {
        read(I2CFile, readBuf, 2); 
    }

    writeBuf[0] = 0; // Set pointer register to 0 to read from the conversion register
    write(I2CFile, writeBuf, 1);

    read(I2CFile, readBuf, 2); // Read the contents of the conversion register into readBuf

    val = (readBuf[0] << 8) | readBuf[1]; // Combine the two bytes of readBuf into a single 16 bit result 
    return val;

}

int main(int argc,char** argv) {

    int ADS_address = 0x48; // Address of our device on the I2C bus
    int I2CFile;


    I2CFile = open("/dev/i2c-1", O_RDWR); // Open the I2C device

    ioctl(I2CFile, I2C_SLAVE, ADS_address); // Specify the address of the I2C Slave to communicate with
    int i = 0;
    uint16_t ch0[10];
    uint16_t ch1[10];
    uint16_t ch2[10];
    uint16_t ch3[10];
    uint8_t pos = 0;

    
    
    time_t start = time(NULL);
    while (i < 400) {
        /*ch0[pos] = read_channel(I2CFile, 0);
        ch1[pos] = read_channel(I2CFile, 1);
        ch2[pos] = read_channel(I2CFile, 2);
        ch3[pos] = read_channel(I2CFile, 3);*/
        uint16_t sum0=0, sum1=0, sum2=0, sum3=0;

        sum0 = read_channel(I2CFile, 0);
        sum1 = read_channel(I2CFile, 1);
        sum2 = read_channel(I2CFile, 2);
        sum3 = read_channel(I2CFile, 3);
        //pos++; if(pos == 10) pos = 0;
        
        float mvol0 = (sum0*0.09375);
        float mvol1 = (sum1*0.09375);
        float mvol2 = (sum2*0.09375);
        float mvol3 = (sum3*0.09375);
        
        float res0 = calc_force((10000*(5000 - mvol0))/mvol0);
        float res1 = calc_force((10000*(5000 - mvol1))/mvol1);
        float res2 = calc_force((10000*(5000 - mvol2))/mvol2);
        float res3 = calc_force((10000*(5000 - mvol3))/mvol3);
        
        
        
        
        
        /*int j=0;
        uint32_t sum0=0, sum1=0, sum2=0, sum3=0;
        for(j=0;j<10;j++){
            sum0 += ch0[j];
            sum1 += ch1[j];
            sum2 += ch2[j];
            sum3 += ch3[j];
        }
        sum0 = sum0/10;
        sum1 = sum1/10;
        sum2 = sum2/10;
        sum3 = sum3/10;*/
        
        if(i > 20){
            //printf("%d;%d;%d;%d\n", sum0, sum1, sum2, sum3);
            //printf("%08d;%08d;%08d;%08d\n", sum0, sum1, sum2, sum3);
            printf("%08f::%08f::%08f::%08f\n", res0/9.81, res1/9.81, res2/9.81, res3/9.81);

        }
        i++;
        usleep(100000);
    }
    time_t end = time(NULL);
    printf("time passed: %f\n", (double)(end-start));

    close(I2CFile);

    return 0;

}




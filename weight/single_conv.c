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

    uint32_t  sum0=0, sum1=0, sum2=0, sum3=0;
	int j = 0;
    for(j = 0; j < 5; j++){
    while (i < 64) {
        sum0 += read_channel(I2CFile, 0);
        sum1 += read_channel(I2CFile, 1);
        sum2 += read_channel(I2CFile, 2);
        sum3 += read_channel(I2CFile, 3);

        i++;
        usleep(40000);
    }
	sum0 = sum0/64;
	sum1 = sum1/64;
	sum2 = sum2/64;
	sum3 = sum3/64;
	printf("%d,%d,%d,%d,%d\n", sum0+sum1+sum2+sum3, sum0, sum1, sum2, sum3);
	sum0=0;sum1=0;sum2=0;sum3=0; i=0;
}

    close(I2CFile);

    return 0;

}





#ifndef __I2C_h__
#define __I2C_h__

void I2C_Start();
void I2C_Stop();
void I2C_SendByte(unsigned char Byte);
unsigned char I2C_ReceiveByte();
void I2C_SendACK(unsigned char ACK);
unsigned char I2C_ReceiveACK();

#endif
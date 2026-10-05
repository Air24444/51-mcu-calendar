#ifndef __AT24C512_H__
#define __AT24C512_H__

#include <REGX52.H>

void AT24C512_Init(void);
void AT24C512_WriteDate(unsigned int WriteAdress, unsigned char Date);
unsigned char AT24C512_ReceiveDate(unsigned int ReadAdress);

#endif
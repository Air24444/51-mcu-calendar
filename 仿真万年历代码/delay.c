#include <intrins.h>

void Delay(unsigned int x)	//@11.0592MHz
{
	unsigned char data i, j;

	while(x--){
			_nop_();
			i = 2;
			j = 199;
			do
			{
				while (--j);
			} while (--i);
	  }
}
	
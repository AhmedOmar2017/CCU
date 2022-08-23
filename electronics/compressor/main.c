/*
 */
#define F_CPU 16000000UL
#include <avr/io.h>

int main(void)
{

    DDRB = 0XFF;

    while(1)
    {
        PORTB = 0XFF;
    }

    return 0;
}

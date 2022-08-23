/*
 * all_ports.c
 *
 * Created: 3/13/2022 9:28:48 AM
 * Author : Maintenance
 */ 
//#define F_CPU 8000000UL
#include "lcd.h"
#include <avr/io.h>
#include <stdio.h>
#include <stdlib.h>
#include <util/delay.h>
#include <avr/interrupt.h>


#define STAR			PINB0
#define DELTA			PINB1
#define FAN				PINB2
#define VALVE			PINB3

char line[4];
char base[4];
uint8_t static volatile temp1 = 0;
uint8_t static volatile temp2 = 0;
uint8_t static volatile temp3 = 0;
uint8_t static volatile press1 = 0;
uint8_t static volatile press2 = 0;
uint8_t static volatile press_Oil = 0;
uint8_t TSignal = 0;
uint8_t PSignal = 0;
uint8_t press = 0;
char Ovheat = 0;
char approve_valve = 0;
char Ovpress = 0;
char Oil_Filter = 0;
char ShutDown = 0;

int main(void)
{
	
	/* External interrupt */
    DDRB  = 0x0F;
	GICR |= (1<<INT0) | (1<<INT1);
	MCUCR |= (1<<ISC00) | (1<<ISC01) | (1<<ISC11) | (1<<ISC10);
	
	
	
	LCDInit(); //Initialize LCD Display

	sprintf(line,"T:");
	LCDGotoXY(1,1);
	LCDString(line);
	
	sprintf(line,"P:");
	LCDGotoXY(13,1);
	LCDString(line);
	
	sprintf(line,"OFF");
	LCDGotoXY(7,2);
	LCDString(line);
	
	/* ADC interrupt */
	ADCSRA |= 1<<ADPS2;
	ADMUX |= (1<<REFS0);//| (1<<REFS1);
	ADCSRA |= 1<<ADIE;
	ADCSRA |= 1<<ADEN;
	sei();

	ADCSRA |= 1<<ADSC;

	while (1)
	{
	
		
		/* condition for temperature */
		if (temp1 > 130 || temp2 > 25 || temp3 > 140) Ovheat = 1;
		if (temp1 < 130 && temp2 < 25 && temp3 < 140) Ovheat = 0;
		
		/* condition for pressure */
		if (press1 > 7 || press2 > 3) Ovpress = 1;
		if (press1 < 7 && press2 < 3) Ovpress = 0;
		
		/* condition Oil pressure */
		if (press_Oil > 2) Oil_Filter = 1;
		if (press_Oil < 2) Oil_Filter = 0;
		
		if (Oil_Filter == 1)
		{
			sprintf(line,"OilF");
			LCDGotoXY(13,2);
			LCDString(line);
		}
		if (Oil_Filter == 0)
		{
			sprintf(line,"    ");
			LCDGotoXY(13,2);
			LCDString(line);
		}
		
		
		/* Turn on the fan, notfaction  is overheat and display temperature. */ 
		if (Ovheat == 1)
		{
			PORTB |= 1<< FAN;
			sprintf(line,"HT");
			LCDGotoXY(1,2);
			LCDString(line);
			itoa(temp1,base,10);
			LCDGotoXY(3,1);
			LCDString(base);
			approve_valve = 0;
			
		}
		/* Display temperature and turn off the fan */
		else if (Ovheat == 0)
		{
			PORTB &= ~(1<< FAN);
			itoa(temp1,line,10);
			LCDGotoXY(3,1);
			LCDString(line);
			sprintf(line,"  ");
			LCDGotoXY(1,2);
			LCDString(line);
			approve_valve = 1;
			
		}
		/* Turn on the valve, notfaction  is overheat and display pressure. */ 
		if (Ovpress == 0 && approve_valve == 1)
		{
			itoa(press1,base,10);
			LCDGotoXY(15,1);
			LCDString(base);
			if (bit_is_clear(PINB, DELTA))
			{
				sprintf(line,"OFF");
				LCDGotoXY(7,2);
				LCDString(line);	
			}
			
			if(approve_valve == 1 && bit_is_set(PINB, DELTA))
			{
				_delay_ms(200);
				PORTB |= 1<< VALVE;
				sprintf(line,"LOAD");
				LCDGotoXY(7,2);
				LCDString(line);	
			}
		}
		
		/* Display pressure and turn off the valve*/
		else if (Ovpress == 1 || approve_valve == 0)
		{
			itoa(press1,base,10);
			LCDGotoXY(15,1);
			LCDString(base);
			PORTB &= ~(1<< VALVE);
			if (bit_is_set(PINB, DELTA))
			{
				sprintf(line,"Idle");
				LCDGotoXY(7,2);
				LCDString(line);	
			}	
			if (bit_is_clear(PINB, DELTA))
			{
				sprintf(line,"OFF");
				LCDGotoXY(9,2);
				LCDString(line);
			}
		}
		
		/* Display Idle 
		if (bit_is_set(PINB, DELTA) && approve_valve == 0)
		{
			
			sprintf(line,"Idle");
			LCDGotoXY(7, 2);
			LCDString(line);
		}*/
		
		/* Shut down Display */
		/*if (ShutDown == 1)
		{
			sprintf(line,"OFF");
			LCDGotoXY(7,2);
			LCDString(line);
		}*/
	}
	
}


ISR(ADC_vect)
{
	uint8_t theLowADC = ADCL;
	uint16_t theTenBitResults = ADCH<<8 | theLowADC;
	switch(ADMUX)
	{
		/* temperature sensor-1 */
		
		case 0x40:
			temp1 = (theTenBitResults/2);
			ADMUX = 0x41;
			break;
		/* temperature sensor-2 */
		case 0x41:
			temp2 = (theTenBitResults/2);
			ADMUX = 0x42;
			break;
		/* temperature sensor-3 */
		
		case 0x42:
			temp3 = (theTenBitResults/2);
			ADMUX = 0x43;
			break;	
		/* pressure sensor- 1 */
		case 0x43:
			press1 = (theTenBitResults/2);
			/*itoa(press1,line,10);
			LCDGotoXY(12,1);
			LCDString(line);*/
			ADMUX = 0x44;
			break;
			
		/* pressure sensor- 2 */
		case 0x44:
			press2 = (theTenBitResults/2);
			ADMUX = 0x45;
			break;
		/* pressure sensor- 3 */
		case 0x45:
			press_Oil = (theTenBitResults/2);
			ADMUX = 0x40;
			break;
		default:
			break;
	}
	ADCSRA |= 1<<ADSC;
}

		/* shutdown */
		
ISR(INT0_vect)
{
	PORTB &= ~(1<<DELTA);
	PORTB &= ~(1 << STAR);
	ShutDown = 1;
}

		/* Turn On */
		
ISR(INT1_vect)
{
	if(bit_is_set(PIND, PD3) && bit_is_clear(PINB, PB1))
	{
		ShutDown = 0;
		_delay_ms(100);
		PORTB |= 1<<STAR;
		_delay_ms(100);
		PORTB |= 1<<DELTA;
		PORTB &= ~(1 << STAR);
		
	}
	
}

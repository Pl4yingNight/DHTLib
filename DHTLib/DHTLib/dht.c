/*
 * dht.c
 *
 * Created: 14.05.2019
 * Author: Pl4yingNight
 */ 
#include "dht.h"
#include <avr/io.h>
#include <stdio.h>
#include <util/delay.h>

/*
** constants/macros
*/
#define DDR(x) (*(&x - 1))						/* address of input register of port x */
#define PIN(x) (*(&x - 2))						/* address of data direction register of port x */

#define dht11_DDR_in() DDR(DHT_PORT) &= ~(1<<DHT_PIN)
#define dht11_DDR_out() DDR(DHT_PORT) |= (1<<DHT_PIN)
#define dht11_PIN_low() DHT_PORT &= ~(1<<DHT_PIN)
#define dht11_PIN_high() DHT_PORT |= (1<<DHT_PIN)

uint8_t I_RH, D_RH, I_Temp, D_Temp, CheckSum;

void Trigger()									/* micro controller send start pulse/request */
{
	dht11_PIN_high();
	while(PIN(DHT_PORT) & (1<<DHT_PIN));		/* check if sensor has been reset */
	
	dht11_DDR_out();
	dht11_PIN_low();							/* set to low pin */
	_delay_ms(20);								/* wait for 20ms */
	dht11_PIN_high();							/* set to high pin */

	dht11_DDR_in();
	while(PIN(DHT_PORT) & (1<<DHT_PIN));
	while((PIN(DHT_PORT) & (1<<DHT_PIN))==0);
	while(PIN(DHT_PORT) & (1<<DHT_PIN));
}

uint8_t Receive_data()							/* receive data */
{
	uint8_t c = 0;
	for (int q=0; q<8; q++)
	{
		while((PIN(DHT_PORT) & (1<<DHT_PIN)) == 0);	/* check received bit 0 or 1 */
		_delay_us(30);
		if(PIN(DHT_PORT) & (1<<DHT_PIN))		/* if high pulse is greater than 30ms */
			c = (c<<1)|(0x01);					/* then its logic HIGH */
		else
			c = (c<<1);							/* otherwise its logic LOW */
		while(PIN(DHT_PORT) & (1<<DHT_PIN));
	}
	return c;
}

extern uint8_t UpdateData(){
	Trigger();									/* receive response */
	I_RH=Receive_data();						/* store first eight bit in I_RH */
	D_RH=Receive_data();						/* store next eight bit in D_RH */
	I_Temp=Receive_data();						/* store next eight bit in I_Temp */
	D_Temp=Receive_data();						/* store next eight bit in D_Temp */
	CheckSum=Receive_data();					/* store next eight bit in CheckSum */

	if ((I_RH + D_RH + I_Temp + D_Temp) != CheckSum)
	{
		return 1;
	}
	return 0;
}

extern uint8_t dht11_rh_high(){
	return I_RH;
}
extern uint8_t dht11_rh_low(){
	return D_RH;
}
extern uint8_t dht11_temp_high(){
	return I_Temp;
}
extern uint8_t dht11_temp_low(){
	return D_Temp;
}
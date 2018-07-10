/*
  
  
 
 */ 
#define F_CPU 1000000UL					/* Define CPU clock Frequency e.g. here its 8MHz */
#include <avr/io.h>	
#include <util/delay.h>		
#include <avr/interrupt.h>				/* Include AVR std. library file */
#include <stdbool.h>
#include <stdlib.h>
#define BAUD_PRESCALE (((F_CPU / (BAUDRATE * 2UL))) - 1)   /* Baud Prescale has to be precise otherwise we get garbage value when receiving and transmiting byte */

#define  RedLed  PA0

void motoMode(char input);
#define SET_BIT(p,i) ((p) |=  (1 << (i)))
#define CLR_BIT(p,i) ((p) &= ~(1 << (i)))
#define GET_BIT(p,i) ((p) &   (1 << (i)))
static volatile int pulse = 0;
static volatile int i = 0;
volatile uint8_t count;
void blinkRedLight()
{
	PORTA |= (1 << RedLed);
	wait_avr(50);
	PORTA &= (~(1 << RedLed));
}

void motorMode(char Data_in)
{
	
						
	if(Data_in == '1')  // forward
	{
		wait_avr(20);
		PORTC = 0x0A; 						/* Turn ON LED */
		USART_SendString(" Going forward\n"); 
					
		
	}
	else if(Data_in == '2')  // backward
	{
		wait_avr(20);
		PORTC = 0x05; 							
		USART_SendString(" Going backward\n"); 				
	}
	else if(Data_in == '0')
	{
		wait_avr(20);
		PORTC = 0x00; 						
		USART_SendString(" Stopped\n");
	}
	else if ( Data_in == '4')  // left turn
	{
		
		PORTC = 0x02;   
		wait_avr(2000);
		PORTC = 0x0A;
		USART_SendString(" Going Right \n");
	}
	
	else if ( Data_in == '3')   // right turn
	{
		
		PORTC = 0x08;   
		wait_avr(2000);
		PORTC = 0x0A;
		USART_SendString(" Going Left\n");
		
	}

}
void USART_Init(unsigned long BAUDRATE)				
{
	UCSRB |= (1 << RXEN) | (1 << TXEN);				
	UCSRC |= (1 << URSEL)| (1 << UCSZ0) | (1 << UCSZ1);	
	UBRRL = BAUD_PRESCALE;							
	UBRRH = (BAUD_PRESCALE >> 8);					
}

char USART_RxChar()									
{
	while (!(UCSRA & (1 << RXC)));					
	return(UDR);									
}

void USART_TxChar(char data)						
{
	UDR = data;										
	while (!(UCSRA & (1<<UDRE)));					
}

void USART_SendString(char *str)					
{
	int i=0;
	while (str[i]!=0)
	{
		USART_TxChar(str[i]);						
		i++;
	}
}

void
wait_avr(unsigned short msec)
{
	TCCR0 = 3;
	while (msec--) {
		TCNT0 = (unsigned char)(256 - (F_CPU / 8) * 0.001);
		SET_BIT(TIFR, TOV0);
		while (!GET_BIT(TIFR, TOV0));
	}
	TCCR0 = 0;
}


int main(void)
{
	
	  MCUCSR=(1<<JTD);
	  MCUCSR=(1<<JTD);
    DDRC = 0xff;							/* make PORT as output port */
	USART_Init(9600);						

	DDRA = 0xFF;		/* make PORTA as output */
	DDRB = 0xFF;		
	DDRD = 0b11111011;
	MCUCR|=(1<<ISC00) ;
	GICR|=(1<<INT0);
	TCCR1A = 0;
	int16_t COUNTA = 0;
	_delay_ms(50);
	
	UCSRB |= (1 << RXCIE);  
	
	while(1)
	{
		
		/* Every 15 us, we take sample by giving PIND6 high */
		PORTD|=(1<<PIND6);
		_delay_us(15);
		PORTD &=~(1<<PIND6);
		
		_delay_us(2);
		
		char string[11];
		dtostrf(pulse/58, 2, 2, string);
		strcat(string, " cm\n");
		USART_SendString(string);  
			if( ( int )(pulse/58) < 22)		/* motor stops when object is  less than 22cm */
			{
				blinkRedLight();
				PORTC = 0xFF;
	
			}
			
	
	}
	
	return 0;
}
ISR(INT0_vect)		

{
	if (i==1)
	{
		TCCR1B=0;
		pulse=TCNT1;		
		TCNT1=0;
		i=0;
	}
	if (i==0)
	{
		TCCR1B|=(1<<CS11);     
		i=1;
	}
}

ISR(USART_RXC_vect)					
{
	char ReceivedByte;
	ReceivedByte = UDR;					
	
	motorMode(ReceivedByte);		
}




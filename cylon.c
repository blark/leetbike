/*
 * Author: Blark - blark at pwnp.al
 *  - some of this code is ripped from Atmel's software PWM example
 */
 
#include <avr/io.h>
#include <util/delay.h>
#include <avr/interrupt.h>
#include <stdbool.h>

#define CHMAX 7 // CHMAX should equal the number of LEDs
#define PWMDEFAULT 0x00 // default PWM value at start up for all channels
#define CYCLEDEL 1550 // delay between each cycle of the LEDS
#define FADEDEL 10 // delay between updates to PWM

#define LED0 PA0 // assign LED names to output ports on the attiny84
#define LED1 PA1
#define LED2 PA2
#define LED3 PA3
#define LED4 PA4
#define LED5 PA5
#define LED6 PA6
 
#define LED0_CLEAR (pin_level &= ~(1 << LED0))
#define LED1_CLEAR (pin_level &= ~(1 << LED1)) 
#define LED2_CLEAR (pin_level &= ~(1 << LED2))
#define LED3_CLEAR (pin_level &= ~(1 << LED3))
#define LED4_CLEAR (pin_level &= ~(1 << LED4))
#define LED5_CLEAR (pin_level &= ~(1 << LED5))
#define LED6_CLEAR (pin_level &= ~(1 << LED6))
 
// Set bits corresponding to pin usage above
#define PORT_MASK  (1 << PA0)|(1 << PA1)|(1 << PA2)|(1 << PA3)|(1 << PA4)|(1 << PA5)|(1 << PA6)
 
#define set(x) |= (1<<x)
#define clr(x) &=~(1<<x)
#define inv(x) ^=(1<<x)
 
#define LED_PORT PORTA
#define LED_DDR DDRA

void delay_ms(uint16_t ms);
void init();

unsigned char max_bright = 254; // maximum brightness to go to is 254
unsigned char start_bright = 84; // turn on the next led when we reach 1/3 of the max_bright of the previous one
unsigned char compare[CHMAX] = {0};  // from ATMEL PWM code
unsigned char led_bright[CHMAX] = {0}; // led brightness array holds current brightness for each led (volatile?)
signed char fade_inc = 2; // altering this value will increase/decrease fade speed
signed char led_increment[CHMAX] = {2,0}; // if 0 fade is off, if positive fading up, if negative fading down
bool reverse_leds = 0; // 0 means leds are going 0 -> CHMAX, 1 means leds are going CHMAX -> 0.

char wtf = 1; //if you delete this the program stops working... WTF!?

int main(void)
{
    init();	
	while(1)
	{ 
		unsigned char i;
		for(i=0; i<CHMAX; i++)
		{
			// check if the led has reached max_bright
			// if it has, change the fade_inc to negative so it goes back down.
			if (led_bright[i] == max_bright) 
			{ 
				led_increment[i] = -led_increment[i];
			}
			
			// the next led should start fading if the current one has reached the start_bright threshold
			// and it's fading in, two if statements one for each direction.
			if ((led_bright[i] == start_bright) && (led_increment[i] > 0) && (reverse_leds==0))
			{
				led_increment[(i+1)] = fade_inc;
			}
			if ((led_bright[i] == start_bright) && (led_increment[i] > 0) && (reverse_leds)) 
			{
				led_increment[(i-1)] = fade_inc;
			}

			// if the current led brightness is 0 and the fade_inc is a negative then the led cycle
			// is complete - we need to turn it off.
			if((led_bright[i] == 0) && (led_increment[i] < 0))
			{
				// set led_increment to 0 to turn led updates off
				led_increment[i] = 0;

				// if we're at the last led, reverse the fade
				if((i == (CHMAX-1)) && (reverse_leds==0))
				{
					reverse_leds=1;
					led_increment[CHMAX-1]=fade_inc; // set the last led to start
					delay_ms(CYCLEDEL);	  
				}
				
				// if we're at the first led, reverse the fade
				if ((i == 0) && (reverse_leds))
				{
					reverse_leds=0; 
					led_increment[0]=fade_inc; // sets the first led to start 
					delay_ms(CYCLEDEL);
				}
			}

			// this is where the led_increment actually gets added 
			led_bright[i] += led_increment[i];
			
		}			

		// if this delay isn't here the leds race across so fast you can hardly see the fade.
		delay_ms(FADEDEL); 
	}
}
 
void delay_ms(uint16_t ms)
{
        while (ms)
        {
                _delay_ms(1);
                ms--;
        }
}
 
void init(void)
{
    LED_DDR set(LED0);      // set the direction of the ports
	LED_DDR set(LED1);
	LED_DDR set(LED2);
	LED_DDR set(LED3);
	LED_DDR set(LED4);
	LED_DDR set(LED5);
	LED_DDR set(LED6);
	CLKPR = (1 << CLKPCE);        // enable clock prescaler update
	CLKPR = 0;                    // set clock to maximum (= crystal)
    TIFR0 = (1 << TOV0);          // clear interrupt flag
    TIMSK0 = (1 << TOIE0);        // enable overflow interrupt
    TCCR0B = (1 << CS00);         // start timer, no prescale
    sei();
}
 
ISR (TIM0_OVF_vect)
{
        static unsigned char pin_level=PORT_MASK;
        static unsigned char softcount=0xFF;
        LED_PORT = pin_level; // update outputs
        if(++softcount == 0)  // increment modulo 256 counter and update the compare values only when counter = 0.
        {        
                // old code used to manually assign compare[x] to led_bright[x], and led_bright was voliatile.. 
                // but it seems to work this way too. not sure if this is smart or dumb.
                memcpy(compare, led_bright, CHMAX*sizeof(int)); // copies contents of led_bright array to compare
                pin_level = PORT_MASK;     // set all port pins high
        }

        // clear port pin on compare match (executed on next interrupt)
        if(compare[0] == softcount) { LED0_CLEAR; }
        if(compare[1] == softcount) { LED1_CLEAR; }
        if(compare[2] == softcount) { LED2_CLEAR; }
        if(compare[3] == softcount) { LED3_CLEAR; }
        if(compare[4] == softcount) { LED4_CLEAR; }
		if(compare[5] == softcount) { LED5_CLEAR; }
		if(compare[6] == softcount) { LED6_CLEAR; }
}

#include "o3.h"
#include "gpio.h"
#include "systick.h"

/**************************************************************************//**
 * @brief Konverterer nummer til string
 * Konverterer et nummer mellom 0 og 99 til string
 *****************************************************************************/
void int_to_string(char *timestamp, unsigned int offset, int i) {
    if (i > 99) {
        timestamp[offset]   = '9';
        timestamp[offset+1] = '9';
        return;
    }

    while (i > 0) {
	    if (i >= 10) {
		    i -= 10;
		    timestamp[offset]++;

	    } else {
		    timestamp[offset+1] = '0' + i;
		    i=0;
	    }
    }
}

/**************************************************************************//**
 * @brief Konverterer 3 tall til en timestamp-string
 * timestamp-argumentet må være et array med plass til (minst) 7 elementer.
 * Det kan deklareres i funksjonen som kaller som "char timestamp[7];"
 * Kallet blir dermed:
 * char timestamp[7];
 * time_to_string(timestamp, h, m, s);
 *****************************************************************************/
void time_to_string(char *timestamp, int h, int m, int s) {
    timestamp[0] = '0';
    timestamp[1] = '0';
    timestamp[2] = '0';
    timestamp[3] = '0';
    timestamp[4] = '0';
    timestamp[5] = '0';
    timestamp[6] = '\0';

    int_to_string(timestamp, 0, h);
    int_to_string(timestamp, 2, m);
    int_to_string(timestamp, 4, s);
}


//Define pointers
#define LED_0_PIN 2
#define LED_PORT GPIO_PORT_E

#define BUTTON0_PIN 9
#define BUTTON1_PIN 10
#define BUTTON_PORT GPIO_PORT_B

volatile int timeV[3];
volatile char timestamp[7];

volatile gpio_map_t* GPIO_map;
volatile systick_map_t* SYSTICK_map;
volatile port_pin_t led0;
volatile port_pin_t button0;
volatile port_pin_t button1;

enum {STILL_SEKUNDER, STILL_MINUTTER, STILL_TIMER, TELL_NED, ALARM} state;


//SETTER riktig mode på pinsa:
void setGPIOregister(volatile gpio_map_t* adress, volatile port_pin_t* func, byte mode){
    adress->ports[func->port].DOUT &= (0 << func->pin);

    volatile word adr;
    if (func->pin > 7){
        adr = adress->ports[func->port].MODEH & ~(0b1111 << 4*(func->pin - 8));
        adress->ports[func->port].MODEH = adr | (mode << 4*(func->pin - 8));
    }
    else {
        adr = adress->ports[func->port].MODEL & ~(0b1111 << 4*func->pin);
        adress->ports[func->port].MODEL = adr | (mode << 4*func->pin);
    }

}


//Enable GPIO-interrupts
void enableGPIOinterrupt(volatile gpio_map_t* adress, volatile port_pin_t* button){
    if (button->pin > 7){
        adress->EXTIPSELH &= ~(0b1111 << 4 * (button->pin - 8));
        adress->EXTIPSELH |= (1 << 4 * (button->pin - 8));
    }else {
        adress->EXTIPSELL &= ~(0b1111 << (4 * button->pin));
        adress->EXTIPSELL |= (1 << (4 * button->pin));
    }
    adress->EXTIFALL |= (0b1 << button->pin);
    adress->IFC |= (0b1 << button->pin);
    adress->IEN |= (0b1 << button->pin);
}


//Enable Systick-interrupts
void enableSYSTICKinterrupts(volatile systick_map_t* adress){
    adress->CTRL |= 0b111;
    adress->LOAD = (FREQUENCY);
    adress->VAL = 0;
}



//Interrupt-vektorer
void SysTick_Handler(void){
    if (state == TELL_NED) {
        if (timeV[0] == 0) {
            timeV[0] = 59;
            if (timeV[1] == 0) {
                timeV[1] = 59;
                timeV[2]--;
            }
            else {
                timeV[1]--;
            }
        }
        else {
            timeV[0]--;

        }
        if (timeV[2] == 0 && timeV[1] == 0 && timeV[0] == 0) {
        state = ALARM;
        GPIO_map->ports[LED_PORT].DOUTSET |= ((0b1 << LED_0_PIN));
        }
    }
}




void GPIO_ODD_IRQHandler(void){
    if (state == STILL_TIMER) {
        timeV[state]++;
        if (timeV[state] == 99) {
            timeV[state] = 0;
        }
    }
    else if (state <= STILL_MINUTTER) {
        timeV[state]++;
        if (timeV[state] == 59) {
            timeV[state] = 0;
        }
    }

    //sett IF tilbake
    GPIO_map->IFC |= (0b1 << BUTTON0_PIN);
}



void GPIO_EVEN_IRQHandler(void){
    //Skifter state så lenge state != TELL_NED:
    if (state == ALARM) {
        GPIO_map->ports[LED_PORT].DOUTCLR ^= ((0b1 << LED_0_PIN));
        state = STILL_SEKUNDER;

    }
    else if (state < TELL_NED) {
        state++;
    }

    //Sett IF tilbake
    GPIO_map->IFC |= (0b1 << BUTTON1_PIN);
}




int main(void) {
    init();

    GPIO_map = (gpio_map_t*) GPIO_BASE;
    SYSTICK_map = (systick_map_t*) SYSTICK_BASE;

    led0.pin = LED_0_PIN;
    led0.port = LED_PORT;

    button0.port = BUTTON_PORT;
    button0.pin = BUTTON0_PIN;

    button1.port = BUTTON_PORT;
    button1.pin = BUTTON1_PIN;

    timeV[0] = 0;
    timeV[1] = 0;
    timeV[2] = 0;


    //Set input og output
    setGPIOregister(GPIO_map, &led0, GPIO_MODE_OUTPUT);
    setGPIOregister(GPIO_map, &button0, GPIO_MODE_INPUT);
    setGPIOregister(GPIO_map, &button1, GPIO_MODE_INPUT);

    //Enabler interrupts
    enableGPIOinterrupt(GPIO_map, &button0);
    enableGPIOinterrupt(GPIO_map, &button1);
    enableSYSTICKinterrupts(SYSTICK_map);

    //Skrur på LED
    //GPIO_map->ports[LED_PORT].DOUTSET |= ((1 << LED_0_PIN));


    state = STILL_SEKUNDER;
    while (true) {

        time_to_string(timestamp, timeV[2], timeV[1], timeV[0]);

        if (state == ALARM) {
            lcd_write("ALARM");
        }
        else{
            lcd_write(timestamp);
        }
    }

    return 0;
}

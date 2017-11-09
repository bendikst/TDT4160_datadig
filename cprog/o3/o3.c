#include "o3.h"
#include "gpio.h"
#include "systick.h"

int main(void) {
    int i;
    port_pin_t led;

    init();

    // LED0 er pinne 2 på Port E
    led.port = GPIO_PORT_E;
    led.pin = 2;

    // Regn ut adressen til LED porten (se kompendiet)
    // NB: parentes rundt regnestykket for adressen!
    gpio_port_map_t *led_port = (gpio_port_map_t *)(GPIO_BASE + 9 * 4 * led.port);

    // Konfigurer LED-pinnen som output (se kompendiet)
    led_port->DOUTCLR = (1 << led.pin);
    led_port->MODEL &= ~(0b1111 << (led.pin * 4));
    led_port->MODEL |= GPIO_MODE_OUTPUT << (led.pin * 4);

    while (1) {
        // Blink LED
        led_port->DOUTTGL = (1 << led.pin);
        // Vent litt...
        for (i = 0; i < 100000; i++) { }
    }

    return 0;
}

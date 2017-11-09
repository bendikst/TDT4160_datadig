.thumb
.syntax unified

.include "gpio_constants.s"     // Register-adresser og konstanter for GPIO

.text
	.global Start

Start:

    LDR R0, =GPIO_BASE //Leser inn GPIO adressen til R0

	//Finner LED-adressen, laster inn til R1
		MOV R1, #LED_PORT
		MOV R2, #PORT_SIZE
		MUL R1, R1, R2
		LDR R2, =GPIO_PORT_DOUT
		ADD R1, R1, R2

//Finner PB0-knappens adresse, laster inn til R7
		MOV R7,#BUTTON_PORT
		MOV R8,#PORT_SIZE
		MUL R7, R7, R8
		LDR R8, =GPIO_PORT_DIN
		ADD R7, R7, R8

		//skrur på LED:
		MOV R3, #0
		LSL R3, R3, #LED_PIN
		LDR R4, [R0, R1]
		ORR R5, R4, R3
		STR R5, [R0, R1]


NOP // Behold denne på bunnen av fila

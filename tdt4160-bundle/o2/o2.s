.thumb
.syntax unified

.include "gpio_constants.s"     // Register-adresser og konstanter for GPIO
.include "sys-tick_constants.s" // Register-adresser og konstanter for SysTick

.text
	.global Start

.global SysTick_Handler
.thumb_func
SysTick_Handler:

	LDR R7, [R0] //Laster inn nåverdi for tenths til R7
	CMP  R7, R4
	BEQ tenths_full

	//Inkrementerer tiendeler som vanlig:
		ADD R7, R7, R3
		STR R7, [R0]
		B end_tenths_full

tenths_full:
//Her settes tenths til null, og ...
	MOV R7, #0
	STR R7, [R0]

//Skrur av/på LED0 for hvert sekund:
MOV R7, #1
LSL R7, #LED_PIN //Maske med 1 på riktig plass
LDR R10, =GPIO_BASE
LDR R8, [R10, R11]

AND R9, R7, R8
CMP R9, #0 //1 hvis LED er av, 0 hvis LED er på
BEQ led_is_off
	//Hvis på: Skrur av
	MVN R7, R7
	AND R9, R8, R7
	STR R9, [R10, R11]
	B led_done

led_is_off: //Hvis altså av, skru på...
	ORR R9, R8, R7
	STR R9, [R10, R11]

led_done:

//Så må sekunder handles
LDR R7, [R1] //Laster inn nåverdi for seconds
CMP R7, R5
BEQ secs_full

//Her inkrementeres sekunder som vanlig
ADD R7, R7, R3
STR R7, [R1]
B end_secs_full

secs_full: //sec har kommet til 59
	MOV R7, #0
	STR R7, [R1]

	//Her må også minutter inkrementeres:
	LDR R7, [R2]
	CMP R7, R5
	BEQ min_full

	//Inkrementerer minutes som vanlig
	ADD R7, R7, R3
	STR R7, [R2]
	B end_minutes_full

	min_full: //Her settes alt til 0, vi har reacha max
		MOV R9, #0
		STR R9, [R0]
		STR R9, [R1]
		STR R9, [R2]

	end_minutes_full:

end_secs_full:

end_tenths_full:

	BX LR

//Interrupts for knappen PB0:
.global GPIO_ODD_IRQHandler
.thumb_func
GPIO_ODD_IRQHandler:
	LDR R10, =SYSTICK_BASE
	MOV R9, #SYSTICK_CTRL
	LDR R6, [R10, R9]
	MOV R7, #1
	EOR R8, R6, R7
	STR R8, [R10, R9]

	//Interrupt er behandlet, sett IF tilbake
	//Skjønner ikke helt hvordan IFC fungerer. Spør studass???
	LDR R10, =GPIO_BASE
	MOV R9, #GPIO_IFC
	LDR R6, [R10, R9]
	MOV R7, #1
	LSL R7, R7, #9
	ORR R8, R6, R7
	STR R8, [R10, R9]


//Må du setter tilbake riktige verdier i registrene?
BX LR


Start:
//FASTE REGISTRE OG VERDIER ETTER OPPSETT AV KLOKKA:
//R0 : Adress tenths
//R1 : Adress secs
//R2 : Adress mins
//R3 : Increment value
//R4 : Max tenths
//R5 : Max secs/mins
//R6 : Free
//R7 : Free
//R8 : Free
//R9 : Free
//R10: Free
//R11: Adressen til LED0
//R12: Adressen til PB0

		//Setter opp SysTick til å enable, generere input, og bruke core clock
		LDR R0, =SYSTICK_BASE
		MOV R1, #SYSTICK_CTRL
		MOV R2, 0b110
		STR R2, [R0, R1]

		//Setter interruptfrekvens ved å bestemme hvilket tall load skal resettes
		MOV R1, #SYSTICK_LOAD
		LDR R2, =FREQUENCY/10
		STR R2, [R0, R1]

		//Setter VAL(current value register) til det samme som frekvensen
		MOV R1, #SYSTICK_VAL
		STR R2, [R0, R1]

		//Setter opp interrup for knappen PB0:
		LDR R0, =GPIO_BASE
		MOV R1, #GPIO_EXTIPSELH

		//Setter opp EXTIPSELH
		MOV R2, 0b1111
		LSL R3, R2, #4
		MVN R3, R3
		LDR R4, [R0, R1]
		AND R5, R3, R4
		MOV R2, #PORT_B
		LSL R2, #4
		ORR R6, R5, R2
		STR R6, [R0, R1]

		//Aktiverer EXTIFALL, bit 9 skal setter til 1
		MOV R1, #GPIO_EXTIFALL
		MOV R2, #1
		LSL R2, R2, #9
		LDR R3, [R0, R1]
		ORR R4, R3, R2
		STR R4, [R0, R1]

		//Skriver IFC (interrupt flag clear)
		MOV R1, #GPIO_IFC
		MOV R2, #1
		LSL R2, R2, #9
		LDR R3, [R0, R1] // Kan det ha sneket seg inne feil her? sjekk med..
		ORR R4, R3, R2
		STR R4, [R0, R1]

		//IEN- interrupt enable:
		MOV R1, #GPIO_IEN
		LDR R3, [R0, R1]
		ORR R4, R3, R2
		STR R4, [R0, R1]

		//Finner adressen til LED0 til R11
		MOV R11, #LED_PORT
		MOV R12, #PORT_SIZE
		MUL R11, R11, R12
		ADD R11, R11, #GPIO_PORT_DOUT

		//Finner adressen til PB0, og lagrer i R12
		MOV R12, #BUTTON_PORT
		MOV R10, #PORT_SIZE
		MUL R12, R12, R10
		ADD R12, R12, #GPIO_PORT_DIN

		//Setter verdien 1 til tenths og laster inn andressene til variable:
		MOV R7, #0
		LDR R0, =tenths
		STR R7, [R0]

		LDR R1, =seconds
		LDR R2, =minutes


		//Setter Increment value til 1, og maksverdi for tenths, secs og mins
		MOV R3, #1
		MOV R4, #9
		MOV R5, #59

		loop:
		B loop


NOP // Behold denne på bunnen av fila

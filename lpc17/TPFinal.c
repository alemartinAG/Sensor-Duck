/*
===============================================================================
 Name        : TPFinal.c
 Author      : Arce Giacobbe Alejandro, Drudi Leandro
 Version     :
 Copyright   : $(copyright)
 Description : main definition
===============================================================================
*/

#ifdef __USE_CMSIS
#include "LPC17xx.h"
#endif

#include <cr_section_macros.h>
#include <stdio.h>
#include <stdlib.h>

#define addrFIO0DIR 0x2009c000;
#define addrFIO0SET 0x2009c018;
unsigned int volatile *const FIO0DIR = (unsigned int*) addrFIO0DIR;
unsigned int volatile *const FIO0SET = (unsigned int*) addrFIO0SET;
unsigned int volatile *const FIO0CLR = (unsigned int*) 0x2009c01c;


void TIMER0_IRQHandler(void);
void TIMER2_IRQHandler(void);
void UART3_IRQHandler(void);

void mandarPulso();
void calcularDist();
void enviarPalabra();
void configUART();
void configADC();
void newHighscore();
void delay(int);
void configTIMERS();
void chequearADC();

int counter = 0;
int auxCap1 = 0;
int auxCap2 = 0;

int tiempo = 0;
int dist_cm = 0;
char palabra;
int recibido = 0;

const int VELMIN = 2;
const int VELMED = 4;
const int VELMAX = 6;

const int SEC0 = 3;
const int SEC1 = 6;
const int SEC2 = 10;
const int SEC3 = 14;

uint16_t conversion = 0;
uint16_t conversion_prev = 0;
uint16_t conv_final;

int highscore = 49;
uint8_t flagHigh = 0;

int main(void) {

	//LPC_GPIO0 -> FIODIR0 |= (1<<22);
	*FIO0DIR |= (1<<5); //Puerto 0.5 como salida para encender lampara
	*FIO0CLR |= (1<<5); //Apago lampara

	LPC_GPIO0 -> FIODIR0 |= (1<<2); //Puerto 0.2 como salida Trigger

	configTIMERS();
	configUART();
	configADC();

	//mandarPulso();

    while(1) {

    	mandarPulso();

    	delay(3000000);

    	//mandarPulso();
    	enviarPalabra();

    	chequearADC();
    }

    return 0 ;
}

void mandarPulso(){

	counter = 0;

	//interrupt y reset con MR0
	LPC_TIM0 -> MCR |= (1<<0)|(1<<1);

	//reset y start timer
	LPC_TIM0 -> TCR |= (1<<1);
	LPC_TIM0 -> TCR |= (1<<0);
	LPC_TIM0 -> TCR &= ~(1<<1);

	return;
}

void calcularDist(){

	tiempo = auxCap2 - auxCap1; //tiempo del pulso
	tiempo = tiempo * 25; //lo paso a us

	dist_cm = tiempo*10/292/2; //calculo de distancia en cm
	dist_cm = dist_cm/1000;

	palabra = (uint8_t) dist_cm;

	//Elijo valor de sección de acuerdo a distancia
	if(palabra <= SEC0){
		palabra = 0;
	}
	else if(palabra <= SEC1){
		palabra = 1;
	}
	else if(palabra <= SEC2){
		palabra = 2;
	}
	else if(palabra <= SEC3){
		palabra = 3;
	}
	else{
		palabra = 4;
	}

	enviarPalabra();

	//reinicio variables
	auxCap1 = 0;
	auxCap2 = 0;

	return;
}

void delay(int demora){
	for(int i=0; i<demora; i++){
		//delay entre mediciones del sensor
	}
	return;
}

void chequearADC(){

	conversion = (LPC_ADC -> ADDR0 >> 4); //valor de conversion del adc

	if (conversion < 1365){
		//Desde 0x000 hasta 0x555
		conv_final = VELMIN;
	}
	else if (conversion > 1365 && conversion < 2730){
		//Desde 0x555 hasta 0xAAA
		conv_final = VELMED;
	}
	else if (conversion > 2730){
		//Desde 0xAAA hasta 0xFFF
		conv_final = VELMAX;
	}

	//Si cambia el valor de la conversion respecto al anterior
	if(conv_final != conversion_prev){
		//Mando conversion +100 para diferenciarlo de otros msj
		palabra = (uint8_t) 100+conv_final;
		enviarPalabra();

		conversion_prev = conv_final;
	}
	return;
}

void newHighscore(){

	palabra = (uint8_t) 200;
	enviarPalabra();
	palabra = 0;

	for(int i=0; i<3; i++){
		*FIO0SET |= (1<<5); //Enciendo lampara
		delay(10000000);
		*FIO0CLR |= (1<<5); //Apago lampara
		delay(10000000);
	}

	/*if (flagHigh == 0){
		*FIO0CLR |= (1<<22); //Enciendo lampara
		flagHigh++;
	}else {
		*FIO0SET |= (1<<22); //Apago lampara
		flagHigh = 0;
	}*/

	return;
}

void enviarPalabra(){

	//if(LPC_UART3 -> LSR & (1<<5)){
		LPC_UART3 -> THR = palabra;
	//}

	return;
}

void TIMER0_IRQHandler(void){

		counter++;

		//genero el pulso de trigger
		if(counter == 1 || counter == 3){
			LPC_GPIO0 -> FIOCLR0 |= (1<<2);
		}
		else{
			LPC_GPIO0 -> FIOSET0 |= (1<<2);
		}

		LPC_TIM0 -> IR |= (1<<0); //limpio bandera

		if(counter == 3){
			//deshabilito interrupt y reset con MR0
			LPC_TIM0 -> MCR &= ~(1<<0);
			LPC_TIM0 -> MCR &= ~(1<<1);
		}

}

void TIMER2_IRQHandler(void){

	//guardo tiempos en los registros
	if(auxCap1 == 0){
		auxCap1 = (int) LPC_TIM2 -> CR0;
	} else{
		auxCap2 = (int) LPC_TIM2 -> CR0;
		calcularDist();
	}

	LPC_TIM2 -> IR |= (1<<4); //limpio flag

}

void UART3_IRQHandler(void){

	recibido = LPC_UART3 ->RBR;

	if(recibido > highscore){
		highscore = recibido;
		newHighscore();
	}
}

void configTIMERS(){

	LPC_TIM0 -> MR0 = 250; //10us

	LPC_SC -> PCONP |= (1<<22); //Enciendo Timer2

	LPC_PINCON -> PINSEL0 |= (1<<9) | (1<<8); //Puerto de CAP2.0 (Echo)
	LPC_TIM2 -> CCR |= (1<<0) | (1<<1) | (1<<2); //CAP0 en Rising y Falling Edge y habilito interrupt

	LPC_TIM2 -> TCR |= (1<<0); //Inicio TMR2

	NVIC_EnableIRQ(TIMER0_IRQn);
	NVIC_EnableIRQ(TIMER2_IRQn);
	return;
}

void configUART(){

	LPC_SC -> PCONP |= (1<<25); //Encendemos UART3

	LPC_UART3 -> LCR |= (1<<0) | (1<<1); //Word-Lenght = 8 bits
	LPC_UART3 -> LCR |= (1<<7); //Pongo el DLAB en 1 para configurar DLL y DLM

	LPC_UART3 -> DLL = 162; //Valor que satisface la ecuacion para 9600 baudios
	LPC_UART3 -> DLM = 0; //Valor que satisface la ecuacion para 9600 baudios

	LPC_UART3 -> LCR &= ~(1<<7); //Pongo el DLAB en 0 nuevamente

	LPC_PINCON -> PINSEL0 |= (1<<1); //Configuro P0.0 como TX
	LPC_PINCON -> PINSEL0 |= (1<<3); //Configuro P0.1 como RX
	LPC_PINCON -> PINMODE0 |= (1<<1) | (1<<3); //Elimino Pull-Ups y Pull-Downs de los puertos de transmisión

	LPC_UART3 -> IER |= (1<<0); //Habilito interrupcion por recepcion
	NVIC_EnableIRQ(UART3_IRQn); //Habilito Interrupcion UART3 en NVIC

	return;
}

void configADC(){

	//Enciendo ADC
	LPC_SC -> PCONP |= (1<<12);

	//Enciendo PDN
	LPC_ADC -> ADCR |= (1<<21);

	//P0.23 como input 0 del ADC
	LPC_PINCON -> PINSEL1 |= (1<<14);

	//Selecciono puerto 0 | CLKDIV en 1 (25MHz/2 = 12.5 MHz) | Burst
	LPC_ADC -> ADCR |= (1<<0) | (1<<8) | (1<<16);

	return;
}





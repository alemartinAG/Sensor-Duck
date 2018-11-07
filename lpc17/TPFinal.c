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

typedef unsigned char bool;
#define true 1;
#define false 0;


void TIMER0_IRQHandler(void);
void TIMER2_IRQHandler(void);
void UART3_IRQHandler(void);

void mandarPulso();
void calcularDist();
void enviarPalabra();
void configUART();
void configADC();

int counter = 0;
int auxCap1 = 0;
int auxCap2 = 0;

int tiempo = 0;
int dist_cm = 0;
char palabra;
int recibido = 4;

uint16_t conversion = 0;
uint16_t conversion_prev = 0;
uint16_t conv_final;

int highscore = 0;

int main(void) {

	LPC_SC -> PCONP |= (1<<22); //Timer2 activado

	LPC_GPIO0 -> FIODIR0 |= (1<<2); //Puerto 0.2 como salida Trigger

	NVIC_EnableIRQ(TIMER0_IRQn);
	NVIC_EnableIRQ(TIMER2_IRQn);

	configUART();
	configADC();

	LPC_TIM0 -> MR0 = 250; //10us

	LPC_PINCON -> PINSEL0 |= (1<<9) | (1<<8); //Puerto de CAP2.0

	LPC_TIM2 -> CCR |= (1<<0) | (1<<1) | (1<<2); //CAP0 en Rising y Falling Edge y habilito interrupt

	LPC_TIM2 -> TCR |= (1<<0); //Inicio TMR2



	mandarPulso();

    while(1) {

    	for(int i=0; i<3000000; i++){

    	}
    	enviarPalabra();

    	mandarPulso();

    	//Valores de 0 a 7 en binario dependiendo la conversion
    	conversion = (LPC_ADC -> ADDR0 >> 4);
    	/*conversion = (uint16_t) (LPC_ADC -> ADDR0 & 0x1000);
    	conversion += (uint16_t) (LPC_ADC -> ADDR0 & 0x2000);
    	conversion += (uint16_t) (LPC_ADC -> ADDR0 & 0x4000);
    	*/

    	if (conversion < 1365){ //Es igual al valor máximo de ADC/3 (4096/3) = 0x555
    		conv_final = 1;
    	}
    	else if (conversion > 1365 && conversion < 2730){ // Desde 0x555 hasta 0xAAA
    		conv_final = 2;
    	}
    	else if (conversion > 2730){ //Desde 0xAAA hasta 0xFFF
    		conv_final = 3;
    	}

    	//Si cambia el valor de la conversion respecto al anterior envio
    	if(conversion != conversion_prev){
    		//Mandar mensajes
    		palabra = 100+conversion;
    		enviarPalabra();
    		conversion_prev = conversion;
    	}

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
		//PRENDO LAMPARA
	}

}

void calcularDist(){

	tiempo = auxCap2 - auxCap1; //tiempo del pulso
	tiempo = tiempo * 25; //lo paso a us

	dist_cm = tiempo*10/292/2;

	//printf("distancia: %i\n", dist_cm);

	dist_cm = dist_cm/1000;

	palabra = (uint8_t) dist_cm;

	if(palabra > 90){
		palabra = 60;
	}

	palabra = palabra/10;
	enviarPalabra();


	auxCap1 = 0;
	auxCap2 = 0;

	return;
}


void enviarPalabra(){

	//itoa(palabra, &string, 10);

	LPC_UART3 -> THR = palabra;

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

	//Interrupcion de ADC0.0
	//LPC_ADC0 -> ADINTEN |= (1<<0);

	//Habilito Interrupcion ADC en NVIC
	//NVIC_EnableIRQ(ADC_IRQn);

	return;
}



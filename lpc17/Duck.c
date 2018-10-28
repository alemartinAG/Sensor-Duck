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

void mandarPulso();
void calcularDist();
void enviarPalabra();
void configUART();

int counter = 0;
int auxCap1 = 0;
int auxCap2 = 0;

int tiempo = 0;
int dist_cm = 0;
char palabra;

int main(void) {

	LPC_SC -> PCONP |= (1<<22); //Timer2 activado

	LPC_GPIO0 -> FIODIR0 |= (1<<2); //Puerto 0.2 como salida Trigger

	NVIC_EnableIRQ(TIMER0_IRQn);
	NVIC_EnableIRQ(TIMER2_IRQn);

	configUART();

	LPC_TIM0 -> MR0 = 250; //10us

	LPC_PINCON -> PINSEL0 |= (1<<9) | (1<<8); //Puerto de CAP2.0

	LPC_TIM2 -> CCR |= (1<<0) | (1<<1) | (1<<2); //CAP0 en Rising y Falling Edge y habilito interrupt

	LPC_TIM2 -> TCR |= (1<<0); //Inicio TMR2



	mandarPulso();

    while(1) {

    	for(int i=0; i<25000000; i++){

    	}

    	mandarPulso();

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

void calcularDist(){

	tiempo = auxCap2 - auxCap1; //tiempo del pulso
	tiempo = tiempo * 25; //lo paso a us

	dist_cm = tiempo*10/292/2;

	//printf("distancia: %i\n", dist_cm);

	dist_cm = dist_cm/1000;

	if(dist_cm > 10){
		palabra = (dist_cm/10)+'0';
		enviarPalabra();
		palabra = (dist_cm%10)+'0';
		enviarPalabra();
	}
	else{
		palabra = dist_cm+'0';
		enviarPalabra();
	}

	palabra = 'R';
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

	//LPC_UART3 -> IER |= (1<<0); //Habilito Interrupcion de transmision y recepcion

	LPC_PINCON -> PINSEL0 |= (1<<1); //Configuro P0.0 como TX
	LPC_PINCON -> PINSEL0 |= (1<<3); //Configuro P0.1 como RX

	//NVIC_EnableIRQ(UART3_IRQn); //Habilito Interrupcion UART en NVIC

	return;
}

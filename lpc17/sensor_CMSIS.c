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

void TIMER0_IRQHandler(void);

void mandarPulso();
void calcularDist();
void enviarDistancia();

int counter = 0;
int auxCap1 = 0;
int auxCap2 = 0;

int tiempo = 0;
int dist_cm = 0;

int main(void) {

	LPC_SC -> PCONP |= (1<<22);

	LPC_GPIO0 -> FIODIR0 |= (1<<0);

	NVIC_EnableIRQ(TIMER0_IRQn);
	NVIC_EnableIRQ(TIMER2_IRQn);

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
			LPC_GPIO0 -> FIOCLR0 |= (1<<0);
		}
		else{
			LPC_GPIO0 -> FIOSET0 |= (1<<0);
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

	enviarDistancia();

	auxCap1 = 0;
	auxCap2 = 0;

	return;
}

void enviarDistancia(){

	//Enviar por uart

	return;
}

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
#define addrFIO0CLR	0x2009c01c;
#define addrFIO0PIN 0x2009c014;

#define addrFIO3DIR 0x2009C060;
#define addrFIO3SET 0x2009C078;
#define addrFIO3CLR	0x2009C07C;
#define addrFIO3PIN 0x2009C074;

#define addrPINSEL4	0x4002C010;

#define addrISER0	0xE000E100;

#define addrEXTINT 	0x400FC140;

#define addrIO0INTENR 	0x40028090;
#define addrIO0INTSTATR 0x40028084;
#define addrIO0INTCLR	0x4002808C;

#define addrT0MCR  0x40004014;
#define addrT0MR0 0x40004018;
#define addrT0IR  0x40004000;
#define addrT0CR  0x40004004;
#define addrT0CCR 0x40004028;

unsigned int volatile *const T0MCR    = (unsigned int*) addrT0MCR;
unsigned int volatile *const T0MR0   = (unsigned int*) addrT0MR0;
unsigned int volatile *const T0IR    = (unsigned int*) addrT0IR;
unsigned int volatile *const T0CR    = (unsigned int*) addrT0CR;
unsigned int volatile *const T0CCR    = (unsigned int*) addrT0CCR;
unsigned int volatile *const T0CR0    = (unsigned int*) 0x4000402C;

unsigned int volatile *const FIO0DIR = (unsigned int*) addrFIO0DIR;
unsigned int volatile *const FIO0SET = (unsigned int*) addrFIO0SET;
unsigned int volatile *const FIO0CLR = (unsigned int*) addrFIO0CLR;
unsigned int volatile *const FIO0PIN = (unsigned int*) addrFIO0PIN;

unsigned int volatile *const FIO3DIR = (unsigned int*) addrFIO3DIR;
unsigned int volatile *const FIO3SET = (unsigned int*) addrFIO3SET;
unsigned int volatile *const FIO3CLR = (unsigned int*) addrFIO3CLR;
unsigned int volatile *const FIO3PIN = (unsigned int*) addrFIO3PIN;

unsigned int volatile *const IO0INTENR = (unsigned int*) addrIO0INTENR;
unsigned int volatile *const IO0INTSTATR = (unsigned int*) addrIO0INTSTATR;
unsigned int volatile *const IO0INTCLR = (unsigned int*) addrIO0INTCLR;

unsigned int volatile *const PINSEL4 = (unsigned int*) addrPINSEL4;
unsigned int volatile *const PINSEL3 = (unsigned int*) 0x4002C00C;

unsigned int volatile *const ISER0 	= (unsigned int *) addrISER0;

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

	*FIO0DIR |= (1<<0);

	*ISER0 |= (1<<1);  //TMR0
	*T0MR0 = 250; //10us

	*PINSEL3 |= (1<<20) | (1<<21); //Puerto de CAP0.0
	*T0CCR |= (1<<0) | (1<<1) | (1<<2); //CAP0 en Rising y Falling Edge y habilito interrupt

	mandarPulso();

    while(1) {

    }

    return 0 ;
}

void mandarPulso(){

	counter = 0;

	//interrupt y reset con MR0
	*T0MCR |= (1<<0)|(1<<1);

	//reset y start timer
	*T0CR |= (1<<1);
	*T0CR |= (1<<0);
	*T0CR &= (0<<1);

	return;
}

void TIMER0_IRQHandler(void){

	if(*T0IR & (1<<0)){

		counter++;

		//genero el pulso de trigger
		if(counter == 1 || counter == 3){
			*FIO0CLR |= (1<<0);
		}
		else{
			*FIO0SET |= (1<<0);
		}

		*T0IR |= (1<<0); //limpio bandera

		if(counter == 3){
			*T0MCR &= (0<<0)|(0<<1); //deshabilito interrupt y reset con MR0
		}

	} else{

		//guardo tiempos en los registros
		if(auxCap1 == 0){
			auxCap1 = (int) *T0CR0;
		} else{
			auxCap2 = (int) *T0CR0;
			calcularDist();
		}

		*T0IR |= (1<<4); //limpio flag
	}

	return;
}

void calcularDist(){

	tiempo = auxCap2 - auxCap1; //tiempo del pulso
	tiempo = tiempo * 25; //lo paso a us

	dist_cm = tiempo*10/292/2;

	printf("distancia: %i\n", dist_cm);

	enviarDistancia();

	return;
}

void enviarDistancia(){

	//Enviar por uart

	return;
}

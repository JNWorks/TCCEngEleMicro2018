#include <stdint.h>
#include <stdbool.h>
#include <inttypes.h>
#include <math.h>
#include "tm4c123gh6pm.h"

int execucoes = 0, flag = 0;
int pico = 0, picoanterior = 0;
int i = 0 ;
int x = 0 ;
int y = 0 ;
int pos = 0 ;
int envia = 0;
int pontos = 0;
int flad = 0;
char dado1 = 0;
char dado2 = 0;
char dado3 = 0;
char dado4 = 0;
char dado5 = 0;
char dado6 = 0;
char aux[4] = {0, 0, 0, 0};
float pulsosentre = 0;
float tempo_em_u_seg;
float max = 0;


void UART_OutChar (char data)
{
    while((UART0_FR_R&0x0020) != 0); // wait until TXFF is 0
    UART0_DR_R = data;
}

void inter (void)
{

    if((GPIO_PORTD_RIS_R & (1 << 2)) == 1 << 2)
    {
        x = NVIC_ST_CURRENT_R;
        y = NVIC_ST_CURRENT_R - 7;
        pos++;
    }

    if((GPIO_PORTD_RIS_R & (1 << 3)) == 1 << 3)
    {
        y = NVIC_ST_CURRENT_R;

        if (pos<7) pontos  =  pontos + pow(2,pos);
        if (pos==7) dado2  = 1;
        if (pos==8) dado2  += 2;

        pulsosentre = x - y;
    }


    GPIO_PORTD_ICR_R = 1 << 2 | 1 << 3 ;
}

void SysTick_isr (void)
{
    if (GPIO_PORTE_DATA_R == 0 << 4)
    {
        flag = 1;

        GPIO_PORTB_DATA_R++;
        if (GPIO_PORTB_DATA_R == 3)
        {
            GPIO_PORTB_DATA_R = 0;
            GPIO_PORTD_DATA_R++;
        }
        if (GPIO_PORTD_DATA_R == 3){
            GPIO_PORTD_DATA_R = 0 << 0 | 0 << 1;
        }
        GPIO_PORTE_DATA_R = 1 << 4;
    }



    else
    {
        GPIO_PORTE_DATA_R = 0 << 4;

    }
}


void main()
{

    SYSCTL_RCC_R = 0x078E3AD1;
    SYSCTL_RCGCGPIO_R = 1 << 0 | 1 << 1 | 1 << 3 | 1 << 4 ; // Liga o clock do port de GPI
    SYSCTL_RCGCADC_R = 1 << 0; // Liga o clock no módulo ADC
    SYSCTL_RCGCUART_R = 0x01;   // Liga a UART0

    NVIC_ST_CTRL_R = 0x00;
    NVIC_ST_RELOAD_R = 32000;
    NVIC_ST_CURRENT_R = 0x00;
    NVIC_ST_CTRL_R = 0x07;

    GPIO_PORTB_DIR_R = 1 << 0 | 1 << 1;
    GPIO_PORTB_DEN_R = 1 << 0 | 1 << 1;

    GPIO_PORTE_DIR_R = 1 << 4;
    GPIO_PORTE_DEN_R = 1 << 4;

    GPIO_PORTD_DIR_R = 1 << 0 | 1 << 1 ;
    GPIO_PORTD_DEN_R = 1 << 0 | 1 << 1 | 1 << 2 | 1 << 3;
    GPIO_PORTD_IS_R = 0 << 2 | 0 << 3; // 0 para borda e 1 para nivel
    GPIO_PORTD_IEV_R = 1 << 2 | 1 << 3; // 1 - para borda de subida e 0 - descida
    GPIO_PORTD_IM_R = 1 << 2 | 1 << 3; // habilita a função

    UART0_CTL_R = 0x00;         // Desabilita a UART0
    UART0_IBRD_R = 8;           // IBRD=int(16000000/(16*115200)) = int(8.6805)
    UART0_FBRD_R = 43;          // FBRD = round(0.6805 * 64) = 43
    UART0_LCRH_R = 0x0070;      // 8-bit word length, enable FIFO
    UART0_CTL_R = 0x0301;       // liga tx rx e a UART

    GPIO_PORTA_AFSEL_R = 0x03;
    GPIO_PORTA_PCTL_R = 0x11;   // habilita usar como UART e não GPIO
    GPIO_PORTA_DEN_R = 0x03;

    //Configuração do ADC PE5
    ADC0_ACTSS_R = 0; // Ativa o sample sequencer (desligar para configurar)
    ADC0_EMUX_R = 15; // define a frequencia de amostragem (1111 - continuously sample)
    ADC0_SSMUX0_R = 8; // valor do canal (AIN8 no datasheet)
    ADC0_SSCTL0_R = 1 << 1; // canal 8 lê por primeiro e define como o ultimo
    ADC0_ACTSS_R = 1 << 0; // Ativa o sample sequencer 0
    ADC0_PSSI_R = 1 << 0; // inicializa o sample sequencer 0

    NVIC_EN0_R = 1 << 3; // INTERRUPÇÃO NO PORT D, POSIÇÃO 3 DO VETOR DE INTER.

    while(1)
    {
        pico = ADC0_SSFIFO0_R;


        //if((GPIO_PORTE_DATA_R == 1 << 4 && NVIC_ST_CURRENT_R > 2000) || (NVIC_ST_CURRENT_R < 6000 && GPIO_PORTE_DATA_R == 1 << 4))
            if (pico > max && picoanterior < pico) max = pico;

        picoanterior = pico;


        if (flag)
        {
            execucoes ++;

            flag = 0;

            if (execucoes >= 9)
            {
                tempo_em_u_seg = (pulsosentre / 16);

                dado1 = pontos;

                dado3 = tempo_em_u_seg;
                dado4 = (tempo_em_u_seg - dado3) * 100;

                dado5 = max / 40;
                dado6 = (max/40-(dado5))*100;
                /////////////////////////////////////
                /*if (envia==50){
                    if (aux[0]<127) aux[0]++;
                    else{
                        if (aux[1]<3) aux[1]++;
                        else aux[1] = 0;
                        aux[0] = 0;
                    }
                    aux[2] = ~(aux[0]+128);
                    aux[3] = ~(aux[1]+128);
                    envia = 0;
                }
                envia++;*/
                ////////////////////////////////////

                if (dado1>127) dado1 = dado1-128;
                UART_OutChar(55);
                UART_OutChar(45);
                //UART_OutChar(aux[0]);
                UART_OutChar(dado1);
                //UART_OutChar(aux[1]);
                UART_OutChar(dado2);
                UART_OutChar(dado3);// tempo = (dado3 + dado 4 / 100)
                UART_OutChar(dado4);
                UART_OutChar(dado5);// pico = (dado4 * 255 + dado5)
                UART_OutChar(dado6);



                pulsosentre = 0;
                execucoes = 0;
                max = 0;
                dado2 = 0;
                pontos = 0;
                pos = -1;
            }
        }
    }
}

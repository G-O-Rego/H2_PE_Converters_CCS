/*
 * functions.c
 *
 *  Created on: 25 Sep 2024
 *      Author: gonca
 */
#include "f28x_project.h"
#include "f28002x_Device.h"
#include "stdlib.h"
#include "math.h"
#include "functions.h"

// global variables

    Uint16 Vsen1;
    Uint16 Vsen1_med[100];
    Uint16 Isen1;
    Uint16 Isen1_med[100];
    Uint16 Vsen2;
    Uint16 Vsen2_med[100];
    Uint16 Isen2;
    Uint16 Isen2_med[100];
    Uint16 Vsen3;
    Uint16 Vsen3_med[100];
    Uint16 Isen3;
    Uint16 Isen3_med[100];
    Uint16 Vsen4;
    Uint16 Vsen4_med[100];
    //Uint16 SensorX[5];
    float Vref = 0;
    float Ppv = 0, Ppv_ant = 0, Vpv_ant = 0;
    float Vdclink, Vh2, Vev, Vpv, Ih2, Iev, Ipv;
    double VpvMed, IpvMed, cont;

    int count = 0;
    int flag_timer_0 = 0;
    int flag_ADC = 0;
    int Mode = OFF;

    // Current Control Variables
    float temp = 0.0;
    float integral = 0.0;
    ////////////////////////////


void SCIA_config(void)
{
    SciaRegs.SCICCR.all = 0x7;
    SciaRegs.SCICTL1.all = 0x0003; // enable TX, RX, internal SCICLK,

    SciaRegs.SCICTL2.bit.TXINTENA = 1;          // Enable TXRDY interrupt
    SciaRegs.SCICTL2.bit.RXBKINTENA = 1;        // Enable RXRDY/BRKDT interrupt

    SciaRegs.SCIHBAUD.all= 0x01; // Ideal Baud=x ##
    SciaRegs.SCILBAUD.all = 0x3B;

    SciaRegs.SCIFFTX.all=   0xC001;
    SciaRegs.SCIFFRX.all=   0x0021;
    SciaRegs.SCIFFCT.all=   0x00;

    SciaRegs.SCICTL1.all = 0x0023; // Relinquish SCI from Reset

    SciaRegs.SCIFFTX.bit.TXFIFORESET=   1;
    SciaRegs.SCIFFRX.bit.RXFIFORESET=   1;

    EALLOW;
    GpioCtrlRegs.GPAMUX2.bit.GPIO28 = 1;   // Configure GPIO28 for SCIRXDA operation
    GpioCtrlRegs.GPAMUX2.bit.GPIO19 = 2;   // Configure GPIO19 for SCIRXDA operation
    GpioCtrlRegs.GPAMUX1.bit.GPIO7 = 2;    // Configure GPIO7  for SCIRXDA operation

    GpioCtrlRegs.GPAMUX2.bit.GPIO29 = 1;   // Configure GPIO29 for SCITXDA operation
    GpioCtrlRegs.GPAMUX2.bit.GPIO18 = 2;   // Configure GPIO18 for SCITXDA operation
    GpioCtrlRegs.GPAMUX1.bit.GPIO12 = 2;   // Configure GPIO12 for SCITXDA operation

    EDIS;
}

void H2_config_GPIO(void)
{
    EALLOW;
//####################  USER LED ######################################
    GpioCtrlRegs.GPAMUX2.bit.GPIO31 = 0;     //CONFIG MUX2 PIN 31 TO GPIO PIN
    GpioCtrlRegs.GPADIR.bit.GPIO31 = 1;    // 0-> Input; 1-> Output
    GpioDataRegs.GPACLEAR.bit.GPIO31 = 1;  // turn off port

//####################  TOP_1 PWM1A ######################################
    GpioCtrlRegs.GPAMUX1.bit.GPIO0 = 1;     //CONFIG MUX2 PIN 40 TO PWM PIN

//####################  BOT_1 PWM1B ######################################
    GpioCtrlRegs.GPAMUX1.bit.GPIO1 = 1;     //CONFIG MUX2 PIN 39 TO PWM PIN

//####################  TOP_2 PWM2A ######################################
    GpioCtrlRegs.GPAMUX1.bit.GPIO2 = 1;     //CONFIG MUX2 PIN 38 TO PWM PIN

//####################  BOT_2 PWM2B ######################################
    GpioCtrlRegs.GPAMUX1.bit.GPIO3 = 1;     //CONFIG MUX2 PIN 37 TO PWM PIN

//####################  TOP_3 PWM4A ######################################
    GpioCtrlRegs.GPAMUX1.bit.GPIO6 = 1;     //CONFIG MUX2 PIN 78 TO PWM PIN

//####################  BOT_3 PWM4B ######################################
    GpioCtrlRegs.GPAMUX1.bit.GPIO7 = 1;     //CONFIG MUX2 PIN 77 TO PWM PIN

//####################  TOP_4 PWM7A ######################################
    GpioCtrlRegs.GPAMUX1.bit.GPIO12 = 1;     //CONFIG MUX2 PIN 80 TO PWM PIN

//####################  BOT_4 PWM7B ######################################
    GpioCtrlRegs.GPAMUX1.bit.GPIO13 = 1;     //CONFIG MUX2 PIN 79 TO PWM PIN


    EDIS;
}

void timer0_config(void)
{
    EALLOW;

    CpuTimer0Regs.PRD.all = 5000;     //SET THE PERIOD 32 BITS

    CpuTimer0Regs.TCR.bit.TRB = 0; //RELOADS ALL TIMER REGISTERS WITH PERIOD VALUE
    CpuTimer0Regs.TPR.bit.PSC = 0; //SETS THE PRESCALER TO 1 (SYSCLOCK)


    CpuTimer0Regs.TCR.bit.FREE = 0; // Stop after the TIMH:TIM decrements to 0 (soft stop)
    CpuTimer0Regs.TCR.bit.SOFT = 1;

    CpuTimer0Regs.TCR.bit.TSS = 1; // Stop Timer
    CpuTimer0Regs.TCR.bit.TIE = 1; //ENABLES INTERRUPT
    CpuTimer0Regs.TIM.all = 0;     //CLEAR COUNTER REGISTER
    EDIS;
}

void adc_config(void)
{
    // Sequencing Power Up
    EALLOW;

    // Power ADC BG
    AdccRegs.ADCCTL1.bit.ADCPWDNZ = 1;
    // ADCINT1 trips after AdcResults latch
    AdccRegs.ADCCTL1.bit.INTPULSEPOS = 1;


    DELAY_US(1000);

    // Configure ADCSOC0
    AdccRegs.ADCSOC0CTL.bit.CHSEL = 6;      // Convert pin C6 (ADCINC6)
    AdccRegs.ADCSOC0CTL.bit.ACQPS = 25;     // Sample window 7 cycles
    AdccRegs.ADCSOC0CTL.bit.TRIGSEL = 0;    // Trigger selection: Software

    // Configure ADCSOC1
    AdccRegs.ADCSOC1CTL.bit.CHSEL = 5;      // Convert pin C5 (ADCINC5)
    AdccRegs.ADCSOC1CTL.bit.ACQPS = 10;     // Sample window 7 cycles
    AdccRegs.ADCSOC1CTL.bit.TRIGSEL = 0;    // Trigger selection: Software

    // Configure ADCSOC2
    AdccRegs.ADCSOC2CTL.bit.CHSEL = 7;      // Convert pin C7 (ADCINC7)
    AdccRegs.ADCSOC2CTL.bit.ACQPS = 10;     // Sample window 7 cycles
    AdccRegs.ADCSOC2CTL.bit.TRIGSEL = 0;    // Trigger selection: Software

    // Configure ADCSOC3
    AdccRegs.ADCSOC3CTL.bit.CHSEL = 0;      // Convert pin C0 (ADCINC0)
    AdccRegs.ADCSOC3CTL.bit.ACQPS = 10;     // Sample window 7 cycles
    AdccRegs.ADCSOC3CTL.bit.TRIGSEL = 0;    // Trigger selection: Software

    // Configure ADCSOC4
    AdccRegs.ADCSOC4CTL.bit.CHSEL = 2;      // Convert pin C2 (ADCINC2)
    AdccRegs.ADCSOC4CTL.bit.ACQPS = 10;     // Sample window 7 cycles
    AdccRegs.ADCSOC4CTL.bit.TRIGSEL = 0;    // Trigger selection: Software

    // Configure ADCSOC5
    AdccRegs.ADCSOC5CTL.bit.CHSEL = 11;      // Convert pin C11 (ADCINC11)
    AdccRegs.ADCSOC5CTL.bit.ACQPS = 10;     // Sample window 7 cycles
    AdccRegs.ADCSOC5CTL.bit.TRIGSEL = 0;    // Trigger selection: Software

    // Configure ADCSOC6
    AdccRegs.ADCSOC6CTL.bit.CHSEL = 8;      // Convert pin C8 (ADCINC8)
    AdccRegs.ADCSOC6CTL.bit.ACQPS = 10;     // Sample window 7 cycles
    AdccRegs.ADCSOC6CTL.bit.TRIGSEL = 0;    // Trigger selection: Software

    // Configure ADCSOC7
    AdccRegs.ADCSOC7CTL.bit.CHSEL = 15;      // Convert pin C15 (ADCINC15)
    AdccRegs.ADCSOC7CTL.bit.ACQPS = 25;     // Sample window 7 cycles
    AdccRegs.ADCSOC7CTL.bit.TRIGSEL = 0;    // Trigger selection: Software


    AdccRegs.ADCINTSEL1N2.bit.INT1CONT = 0; //disable ADCINT1 Continuous mode
    AdccRegs.ADCINTSEL1N2.bit.INT1SEL = 1;    // EOC1 is trigger for ADCINT1
    AdccRegs.ADCINTSEL1N2.bit.INT1E = 1;    //enables ADCINT1 interrupt

    EDIS;
}

void update_ADC(void)
{
    while(flag_timer_0 == 1)
    {
        Isen1_med[count] = AdccResultRegs.ADCRESULT0;
        //Isen2_med[count] = AdccResultRegs.ADCRESULT2;
        //Isen3_med[count] = AdccResultRegs.ADCRESULT4;
        Vsen1_med[count] = AdccResultRegs.ADCRESULT1;
        Vsen2_med[count] = AdccResultRegs.ADCRESULT3;
        //Vsen3_med[count] = AdccResultRegs.ADCRESULT5;
        Vsen4_med[count] = AdccResultRegs.ADCRESULT7;

        if(count < 100)
        {
            Isen1 += Isen1_med[count];
            //Isen2 += Isen2_med[count];
            Isen2 = AdccResultRegs.ADCRESULT2;
            //Isen3 += Isen3_med[count];
            Isen3 = AdccResultRegs.ADCRESULT4;
            Vsen1 += Vsen1_med[count];
            Vsen2 += Vsen2_med[count];
            Vsen3 += Vsen3_med[count];
            Vsen3 = AdccResultRegs.ADCRESULT5;
            Vsen4 += Vsen4_med[count];

            count++;
        }
        else
        {
            Isen1 = Isen1/count;
            //Isen2 = Isen2/count;
            //Isen3 = Isen3/100.0;
            Vsen1 = Vsen1/count;
            Vsen2 = Vsen2/count;
            //Vsen3 = Vsen3/count;
            Vsen4 = Vsen4/count;


            Ih2         = (Isen1 * 0.004473) + 0.02195;
            Ipv         = (Isen2 * 0.004469) - 0.007601;
            if(Mode == BUCK)
                Iev         = (Isen3 * 0.004573) - 8.95;
            else if(Mode == BOOST)
                Iev         = (Isen3 - 2021.0) / 220.7;


            Vdclink     = (Vsen1 * 0.2719) + 1.173;
            Vh2         = (Vsen2 * 0.05118) + 0.3033;
            Vpv         = (Vsen3 * 0.1539) - 2.109;
            Vev         = (Vsen4 * 0.1558) - 0.117;

            count = 0;
            flag_ADC = 1;
            flag_timer_0 = 0;
        }
    }
}

void lock_pwm_pin(void)
{
    EPwm1Regs.AQCSFRC.bit.CSFA= 0x01;                                     //Forces a continuous low on output A
    EPwm1Regs.AQCSFRC.bit.CSFB= 0x01;                                     //Forces a continuous low on output B
    EPwm2Regs.AQCSFRC.bit.CSFA= 0x01;                                     //Forces a continuous low on output A
    EPwm2Regs.AQCSFRC.bit.CSFB= 0x01;                                     //Forces a continuous low on output B
    EPwm4Regs.AQCSFRC.bit.CSFA= 0x01;                                     //Forces a continuous low on output A
    EPwm4Regs.AQCSFRC.bit.CSFB= 0x01;                                     //Forces a continuous low on output B
    EPwm7Regs.AQCSFRC.bit.CSFA= 0x01;                                     //Forces a continuous low on output A
    EPwm7Regs.AQCSFRC.bit.CSFB= 0x01;                                     //Forces a continuous low on output B
}

void pwm_compare_1_config(void)
{
    //Set compare values
    EPwm1Regs.CMPA.bit.CMPA = 0;                    //CompA_StepUp;
    EPwm1Regs.AQCTLA.bit.CAU = AQ_CLEAR;            // clear PWM2A on event a, up count
    EPwm1Regs.AQCTLA.bit.ZRO = AQ_SET;              // set PWM2A on event ZRO, up count
    EPwm1Regs.AQCTLB.bit.CAU = AQ_SET;              // clear PWM2A on event a, up count
    EPwm1Regs.AQCTLB.bit.ZRO = AQ_CLEAR;            // set PWM2A on event ZRO, up count

    //Set compare values
    EPwm2Regs.CMPA.bit.CMPA = 0;             //CompA_StepUp;
    EPwm2Regs.AQCTLA.bit.CAU = AQ_CLEAR;        // clear PWM2A on event a, up count
    EPwm2Regs.AQCTLA.bit.ZRO = AQ_SET;          // set PWM2A on event ZRO, up count
    EPwm2Regs.AQCTLB.bit.CAU = AQ_SET;        // clear PWM2A on event a, up count
    EPwm2Regs.AQCTLB.bit.ZRO = AQ_CLEAR;          // set PWM2A on event ZRO, up count

    //Set compare values for the buck mode, by default
    EPwm4Regs.CMPA.bit.CMPA = 0;             //CompA_StepUp;
    EPwm4Regs.AQCTLA.bit.CAU = AQ_CLEAR;        // clear PWM2A on event a, up count
    EPwm4Regs.AQCTLA.bit.ZRO = AQ_SET;          // set PWM2A on event ZRO, up count
    EPwm4Regs.AQCTLB.bit.CAU = AQ_SET;        // set PWM2B on event ZRO, up count
    EPwm4Regs.AQCTLB.bit.ZRO = AQ_CLEAR;          // clear PWM2B on event b, up count

    //Set compare values
    EPwm7Regs.CMPA.bit.CMPA = 0;             //CompA_StepUp;
    EPwm7Regs.AQCTLA.bit.CAU = AQ_CLEAR;        // clear PWM2A on event a, up count
    EPwm7Regs.AQCTLA.bit.ZRO = AQ_SET;          // set PWM2A on event ZRO, up count
    EPwm7Regs.AQCTLB.bit.CAU = AQ_SET;        // clear PWM2A on event a, up count
    EPwm7Regs.AQCTLB.bit.ZRO = AQ_CLEAR;          // set PWM2A on event ZRO, up count

}

void pwm_compare_2_config(void)
{
    //Set compare values
    EPwm1Regs.CMPA.bit.CMPA = 0;             //CompA_StepUp;
    EPwm1Regs.AQCTLB.bit.CAU = AQ_SET;        // clear PWM2A on event a, up count
    EPwm1Regs.AQCTLB.bit.ZRO = AQ_CLEAR;          // set PWM2A on event ZRO, up count
    EPwm1Regs.AQCTLA.bit.CAU = AQ_CLEAR;        // clear PWM2A on event a, up count
    EPwm1Regs.AQCTLA.bit.ZRO = AQ_SET;          // set PWM2A on event ZRO, up count

    //Set compare values
    EPwm2Regs.CMPA.bit.CMPA = 0;             //CompA_StepUp;
    EPwm2Regs.AQCTLB.bit.CAU = AQ_SET;        // clear PWM2A on event a, up count
    EPwm2Regs.AQCTLB.bit.ZRO = AQ_CLEAR;          // set PWM2A on event ZRO, up count
    EPwm2Regs.AQCTLA.bit.CAU = AQ_CLEAR;        // clear PWM2A on event a, up count
    EPwm2Regs.AQCTLA.bit.ZRO = AQ_SET;          // set PWM2A on event ZRO, up count

    //Set compare values
    EPwm4Regs.CMPA.bit.CMPA = 0;             //CompA_StepUp;
    EPwm4Regs.AQCTLB.bit.CAU = AQ_SET;        // clear PWM2A on event a, up count
    EPwm4Regs.AQCTLB.bit.ZRO = AQ_CLEAR;          // set PWM2A on event ZRO, up count
    EPwm4Regs.AQCTLA.bit.CAU = AQ_CLEAR;        // clear PWM2A on event a, up count
    EPwm4Regs.AQCTLA.bit.ZRO = AQ_SET;          // set PWM2A on event ZRO, up count

    //Set compare values
    EPwm7Regs.CMPA.bit.CMPA = 0;             //CompA_StepUp;
    EPwm7Regs.AQCTLB.bit.CAU = AQ_SET;        // clear PWM2A on event a, up count
    EPwm7Regs.AQCTLB.bit.ZRO = AQ_CLEAR;          // set PWM2A on event ZRO, up count
    EPwm7Regs.AQCTLA.bit.CAU = AQ_CLEAR;        // clear PWM2A on event a, up count
    EPwm7Regs.AQCTLA.bit.ZRO = AQ_SET;          // set PWM2A on event ZRO, up count
}

void pwm_config(void)
{
    EALLOW;
    CpuSysRegs.PCLKCR2.bit.EPWM1 = 1;
    CpuSysRegs.PCLKCR2.bit.EPWM2 = 1;
    CpuSysRegs.PCLKCR2.bit.EPWM4 = 1;
    CpuSysRegs.PCLKCR2.bit.EPWM7 = 1;


    //EPW1 CONFIG 100Mhz / 20khz =5000
    EPwm1Regs.TBPRD = 5000;                     //SET THE PERIOD OF THE PWM SIGNAL
    EPwm1Regs.TBPHS.bit.TBPHS = 0;              // Set Phase register to zero
    EPwm1Regs.TBCTR = 0;                        //CLEAR PWM COUNTER
    EPwm1Regs.TBCTL.bit.CTRMODE = 0;            //COUNTMODE_UP
    EPwm1Regs.TBCTL.bit.PHSEN = 1;              //PHASE REGISTER

    EPwm1Regs.TBCTL.bit.CLKDIV = 0;             //These bits select the time base clock pre-scale value (TBCLK = EPWMCLK/(HSPCLKDIV * CLKDIV)
    EPwm1Regs.TBCTL.bit.HSPCLKDIV = 0;          //EPWMCLK = SYSCLOCK
    EPwm1Regs.TBCTL.bit.PRDLD = 0;              // period autoreload

    EPwm1Regs.CMPCTL.bit.SHDWAMODE = 0;
    EPwm1Regs.CMPCTL.bit.SHDWBMODE = 0;
    EPwm1Regs.CMPCTL.bit.LOADAMODE = 0;
    EPwm1Regs.CMPCTL.bit.LOADBMODE = 0;

    //EPW2 CONFIG 100Mhz / 20khz =5000
    EPwm2Regs.TBPRD = 5000;                     //SET THE PERIOD OF THE PWM SIGNAL
    EPwm2Regs.TBPHS.bit.TBPHS = 2500;           // Set Phase register
    EPwm2Regs.TBCTR = 0;                        //CLEAR PWM COUNTER
    EPwm2Regs.TBCTL.bit.CTRMODE = 0;            //COUNTMODE_UP
    EPwm2Regs.TBCTL.bit.PHSEN = 1;              //PHASE REGISTER

    EPwm2Regs.TBCTL.bit.CLKDIV = 0;             //These bits select the time base clock pre-scale value (TBCLK = EPWMCLK/(HSPCLKDIV * CLKDIV)
    EPwm2Regs.TBCTL.bit.HSPCLKDIV = 0;          //EPWMCLK = SYSCLOCK
    EPwm2Regs.TBCTL.bit.PRDLD = 0;              // period autoreload

    EPwm2Regs.CMPCTL.bit.SHDWAMODE = 0;
    EPwm2Regs.CMPCTL.bit.SHDWBMODE = 0;
    EPwm2Regs.CMPCTL.bit.LOADAMODE = 0;
    EPwm2Regs.CMPCTL.bit.LOADBMODE = 0;

    //EPW4 CONFIG 100Mhz / 20khz =5000
    EPwm4Regs.TBPRD = 5000;                     //SET THE PERIOD OF THE PWM SIGNAL
    EPwm4Regs.TBPHS.bit.TBPHS = 2500;           // Set Phase register to zero
    EPwm4Regs.TBCTR = 0;                        //CLEAR PWM COUNTER
    EPwm4Regs.TBCTL.bit.CTRMODE = 0;            //COUNTMODE_UP
    EPwm4Regs.TBCTL.bit.PHSEN = 1;              //PHASE REGISTER

    EPwm4Regs.TBCTL.bit.CLKDIV = 0;             //These bits select the time base clock pre-scale value (TBCLK = EPWMCLK/(HSPCLKDIV * CLKDIV)
    EPwm4Regs.TBCTL.bit.HSPCLKDIV = 0;          //EPWMCLK = SYSCLOCK
    EPwm4Regs.TBCTL.bit.PRDLD = 0;              // period autoreload

    EPwm4Regs.CMPCTL.bit.SHDWAMODE = 0;
    EPwm4Regs.CMPCTL.bit.SHDWBMODE = 0;
    EPwm4Regs.CMPCTL.bit.LOADAMODE = 0;
    EPwm4Regs.CMPCTL.bit.LOADBMODE = 0;

    //EPW7 CONFIG 100Mhz / 20khz =5000
    EPwm7Regs.TBPRD = 5000;                     //SET THE PERIOD OF THE PWM SIGNAL
    EPwm7Regs.TBPHS.bit.TBPHS = 0;           // Set Phase register to zero
    EPwm7Regs.TBCTR = 0;                        //CLEAR PWM COUNTER
    EPwm7Regs.TBCTL.bit.CTRMODE = 0;            //COUNTMODE_UP
    EPwm7Regs.TBCTL.bit.PHSEN = 1;              //PHASE REGISTER

    EPwm7Regs.TBCTL.bit.CLKDIV = 0;             //These bits select the time base clock pre-scale value (TBCLK = EPWMCLK/(HSPCLKDIV * CLKDIV)
    EPwm7Regs.TBCTL.bit.HSPCLKDIV = 0;          //EPWMCLK = SYSCLOCK
    EPwm7Regs.TBCTL.bit.PRDLD = 0;              // period autoreload

    EPwm7Regs.CMPCTL.bit.SHDWAMODE = 0;
    EPwm7Regs.CMPCTL.bit.SHDWBMODE = 0;
    EPwm7Regs.CMPCTL.bit.LOADAMODE = 0;
    EPwm7Regs.CMPCTL.bit.LOADBMODE = 0;



    EPwm1Regs.TBCTL.bit.SWFSYNC = 1;            //ENABLE SYNC (1)
    EPwm2Regs.TBCTL.bit.SWFSYNC = 1;            //ENABLE SYNC (1)
    EPwm4Regs.TBCTL.bit.SWFSYNC = 1;            //ENABLE SYNC (1)
    EPwm7Regs.TBCTL.bit.SWFSYNC = 1;            //ENABLE SYNC (1)

    EDIS;
}

void MPPT_algorithm(double Vpv_media, double Ipv_media)
{
    //static Uint16 duty = 2500;

    int increment = 50;
    float deltaV, deltaP;
    Uint16 epwm_reg = EPwm7Regs.CMPA.bit.CMPA;

    Ppv = Vpv_media * Ipv_media;

    deltaV = Vpv_media - Vpv_ant;
    deltaP = Ppv - Ppv_ant;


    if (deltaP < 0)
    {
        if (deltaV > 0)
            epwm_reg -= increment;
        else
            epwm_reg += increment;
    }
    else
    {
        if (deltaV > 0)
            epwm_reg += increment;
        else
            epwm_reg -= increment;
    }

    if(epwm_reg > 4900)
    {
        epwm_reg = 4900;
    }
    else if(epwm_reg < 100)
    {
        epwm_reg = 100;
    }

    EPwm7Regs.CMPA.bit.CMPA = epwm_reg;


    Ppv_ant = Ppv;
    Vpv_ant = Vpv_media;
    flag_ADC = 0;
}

void Select_Mode(void)
{
    //First the IGBTs are turned OFF for security reasons
    //EV
    // Top IGBT ON & Bot OFF -> Buck
    // Top IGBT OFF & Bot ON -> Boost
    EPwm4Regs.AQCSFRC.bit.CSFA= 0x01;           // IGBT 4 - Top
    EPwm4Regs.AQCSFRC.bit.CSFB= 0x01;           // IGBT 4 - Bot
    //H2
    EPwm1Regs.AQCSFRC.bit.CSFA= 0x01;           // IGBT 1 - Top
    EPwm1Regs.AQCSFRC.bit.CSFB= 0x01;           // IGBT 1 - Bot
    EPwm2Regs.AQCSFRC.bit.CSFA= 0x01;           // IGBT 2 - Top
    EPwm2Regs.AQCSFRC.bit.CSFB= 0x01;           // IGBT 2 - Bot
    // Solar PVs
    EPwm7Regs.AQCSFRC.bit.CSFA= 0x01;         // IGBT 3 - Top (Do NOT change)
    EPwm7Regs.AQCSFRC.bit.CSFB= 0x01;         // IGBT 3 - Bot

    if(Mode == OFF)
    {
        //EV
        EPwm4Regs.AQCSFRC.bit.CSFA= 0x01;           // IGBT 4 - Top
        EPwm4Regs.AQCSFRC.bit.CSFB= 0x01;           // IGBT 4 - Bot
        //H2
        EPwm1Regs.AQCSFRC.bit.CSFA= 0x01;           // IGBT 1 - Top
        EPwm1Regs.AQCSFRC.bit.CSFB= 0x01;           // IGBT 1 - Bot
        EPwm2Regs.AQCSFRC.bit.CSFA= 0x01;           // IGBT 2 - Top
        EPwm2Regs.AQCSFRC.bit.CSFB= 0x01;           // IGBT 2 - Bot
        // Solar PVs
        EPwm7Regs.AQCSFRC.bit.CSFA= 0x01;         // IGBT 3 - Top (Do NOT change)
        EPwm7Regs.AQCSFRC.bit.CSFB= 0x01;         // IGBT 3 - Bot
    }
    else if(Mode == BUCK)                             // -> Batteries charging
    {   // EV Batteries

        //Set compare values
        EPwm4Regs.CMPA.bit.CMPA = 0;
        EPwm4Regs.AQCTLA.bit.CAU = AQ_CLEAR;        // clear PWM2A on event a, up count
        EPwm4Regs.AQCTLA.bit.ZRO = AQ_SET;          // set PWM2A on event ZRO, up count
        EPwm4Regs.AQCTLB.bit.CAU = AQ_SET;        // set PWM2B on event ZRO, up count
        EPwm4Regs.AQCTLB.bit.ZRO = AQ_CLEAR;          // clear PWM2B on event b, up count

        EPwm4Regs.AQCSFRC.bit.CSFA= 0x00;           // IGBT 4 - Top
        EPwm4Regs.AQCSFRC.bit.CSFB= 0x01;           // IGBT 4 - Bot
    }
    else if(Mode == BOOST)                            // -> Batteries discharging
    {   // EV Batteries

        //Set compare values
        EPwm4Regs.AQCTLA.bit.CAU = AQ_SET;        // clear PWM2A on event a, up count
        EPwm4Regs.AQCTLA.bit.ZRO = AQ_CLEAR;          // set PWM2A on event ZRO, up count
        EPwm4Regs.AQCTLB.bit.CAU = AQ_CLEAR;        // clear PWM2A on event a, up count
        EPwm4Regs.AQCTLB.bit.ZRO = AQ_SET;          // set PWM2A on event ZRO, up count

        EPwm4Regs.AQCSFRC.bit.CSFA= 0x01;           // IGBT 4 - Top
        EPwm4Regs.AQCSFRC.bit.CSFB= 0x00;           // IGBT 4 - Bot
    }
    else if(Mode == H2)
    {   // H2 Electrolyser
        EPwm1Regs.AQCSFRC.bit.CSFA= 0x00;           // IGBT 1 - Top
        EPwm1Regs.AQCSFRC.bit.CSFB= 0x00;           // IGBT 1 - Bot
        EPwm2Regs.AQCSFRC.bit.CSFA= 0x00;           // IGBT 2 - Top
        EPwm2Regs.AQCSFRC.bit.CSFB= 0x00;           // IGBT 2 - Bot
    }
    else if(Mode == PV)
    {   // PV modules
        //Set compare values
        EPwm7Regs.AQCTLA.bit.CAU = AQ_CLEAR;
        EPwm7Regs.AQCTLA.bit.ZRO = AQ_CLEAR;
        EPwm7Regs.AQCTLB.bit.CBU = AQ_CLEAR;
        EPwm7Regs.AQCTLB.bit.ZRO = AQ_SET;

        EPwm7Regs.AQCSFRC.bit.CSFA= 0x01;         // IGBT 3 - Top (Do NOT change)
        EPwm7Regs.AQCSFRC.bit.CSFB= 0x00;         // IGBT 3 - Bot
    }
    else if(Mode == PV_2_H2)
    {
        // PV modules
        //Set compare values
        EPwm7Regs.AQCTLA.bit.CAU = AQ_CLEAR;
        EPwm7Regs.AQCTLA.bit.ZRO = AQ_CLEAR;
        EPwm7Regs.AQCTLB.bit.CBU = AQ_CLEAR;
        EPwm7Regs.AQCTLB.bit.ZRO = AQ_SET;

        EPwm7Regs.AQCSFRC.bit.CSFA= 0x01;         // IGBT 3 - Top (Do NOT change)
        EPwm7Regs.AQCSFRC.bit.CSFB= 0x00;         // IGBT 3 - Bot

        // H2 Electrolyser
        EPwm1Regs.AQCSFRC.bit.CSFA= 0x00;           // IGBT 1 - Top
        EPwm1Regs.AQCSFRC.bit.CSFB= 0x00;           // IGBT 1 - Bot
        EPwm2Regs.AQCSFRC.bit.CSFA= 0x00;           // IGBT 2 - Top
        EPwm2Regs.AQCSFRC.bit.CSFB= 0x00;           // IGBT 2 - Bot
    }
    else if(Mode == ESS_2_H2)
    {
        //Set compare values
        EPwm4Regs.AQCTLA.bit.CAU = AQ_SET;        // clear PWM2A on event a, up count
        EPwm4Regs.AQCTLA.bit.ZRO = AQ_CLEAR;          // set PWM2A on event ZRO, up count
        EPwm4Regs.AQCTLB.bit.CAU = AQ_CLEAR;        // clear PWM2A on event a, up count
        EPwm4Regs.AQCTLB.bit.ZRO = AQ_SET;          // set PWM2A on event ZRO, up count

        EPwm4Regs.AQCSFRC.bit.CSFA= 0x01;           // IGBT 4 - Top
        EPwm4Regs.AQCSFRC.bit.CSFB= 0x00;           // IGBT 4 - Bot

        // H2 Electrolyser
        EPwm1Regs.AQCSFRC.bit.CSFA= 0x00;           // IGBT 1 - Top
        EPwm1Regs.AQCSFRC.bit.CSFB= 0x00;           // IGBT 1 - Bot
        EPwm2Regs.AQCSFRC.bit.CSFA= 0x00;           // IGBT 2 - Top
        EPwm2Regs.AQCSFRC.bit.CSFB= 0x00;           // IGBT 2 - Bot
    }

}

// EV Buck
Uint16 Current_Control_Buck(float I_ref, float Kp, float Ki, float Im, Uint16 epwm_reg)
{
    integral += (I_ref - Im);

    //temp = (I_ref - Im) * Kp;
    epwm_reg = ((I_ref - Im) * Kp) + (Ki * integral);

    //if(epwm_reg < 0)
        //epwm_reg *= -1;

    epwm_reg = temp;

    if(epwm_reg > 4900)
    {
        epwm_reg = 4900;
    }
    else if(temp < 100)
    {
        epwm_reg = 100;
    }

    flag_ADC = 0;
    return epwm_reg;
}

// EV Boost
Uint16 Current_Control_Boost(float I_ref, float Kp, float Ki, float Im, Uint16 epwm_reg)
{
    Im = Im * (-1);

    integral += (I_ref - Im);

    //temp = (I_ref - Im) * Kp;
    temp = ((I_ref - Im) * Kp) + (Ki * integral);

    epwm_reg = temp;

    if(epwm_reg > 4900)
    {
        epwm_reg = 4900;
    }
    else if(temp < 100)
    {
        epwm_reg = 100;
    }

    flag_ADC = 0;
    return epwm_reg;
}

// H2 Stacked Buck
Uint16 Current_Control_H2(float I_ref, float Kp, float Ki, float Im, Uint16 epwm_reg)
{
    integral += (I_ref - Im);
    temp = (I_ref - Im) * Kp + integral * Ki;

    if(temp >= 4900)
    {
        epwm_reg = 4900;
    }
    else if(temp <= 100)
    {
        epwm_reg = 100;
    }
    else
    {
        epwm_reg = temp;
    }

    flag_ADC = 0;
    return epwm_reg;
}


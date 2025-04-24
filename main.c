#include "f28x_project.h"
#include "f28002x_Device.h"
#include "stdlib.h"
#include "math.h"
#include "functions.h"

    //float Kp_ev = 20;              //EV Buck mode
    //float Ki_ev = 0.0001;          //EV Buck mode
    //float Iev_ref_POSITIVE = 1;     //EV Buck mode

    float Kp_ev = 10;             //EV Boost mode
    float Ki_ev = 0.01;           //EV Boost mode
    float Iev_ref_NEGATIVE = 1.0;  //EV Boost mode

    float Kp_h2 = 10.0;
    float Ki_h2 = 0.01;
    float Ih2_ref = 1.0;

// main
void main(void)
{
    // Step 1. Initialize System Control:
    // PLL, WatchDog, enable Peripheral Clocks
    InitSysCtrl();

    // Step 2. Initialize Funcs
    InitGpio();

    // Step 3. Clear all interrupts and initialize PIE vector table:
    // Disable CPU interrupts
    DINT;

    // Initialize PIE control registers to their default state.
    // The default state is all PIE interrupts disabled and flags are cleared.
    InitPieCtrl();

    // Disable CPU interrupts and clear all CPU interrupt flags:
    IER = 0x0000;
    IFR = 0x0000;

    // Initialize the PIE vector table with pointers to the shell Interrupt Service Routines (ISR).
    InitPieVectTable();

    EALLOW;
    PieVectTable.SCIA_RX_INT = &SCIA_RX_ISR; // Group 9 PIE Peripheral Vectors:
    PieVectTable.TIMER0_INT  = &TIMER0_ISR;
    //PieVectTable.ADCC1_INT   = &ADCC1_ISR;
    EDIS;

    // Enable interrupts required for this example
    PieCtrlRegs.PIECTRL.bit.ENPIE = 1; // enable pie block
    PieCtrlRegs.PIEIER9.bit.INTx1 = 1; // PIE Group 9, SCI-A
    PieCtrlRegs.PIEIER1.bit.INTx7 = 1; // PIE Group 1, TIMER0
    //PieCtrlRegs.PIEIER1.bit.INTx3 = 1; // PIE Group 1, ADCAIN1

    IER = 0x101; // Enable CPU INT
    EINT;
    ERTM; // Enable Global realtime interrupt DBGM

    //Step 5. User Code
    SCIA_config();
    timer0_config();
    adc_config();
    lock_pwm_pin();
    H2_config_GPIO();
    pwm_config();
    pwm_compare_1_config();

    CpuTimer0Regs.TCR.bit.TSS = 0; //START TIMER

    EPwm1Regs.CMPA.bit.CMPA = 2500;
    EPwm2Regs.CMPA.bit.CMPA = 2500;
    EPwm4Regs.CMPA.bit.CMPA = 2500;
    EPwm7Regs.CMPA.bit.CMPA = 100;

    Mode = H2;
    Select_Mode();
    cont = 0;

    while(1)
    {
    	update_ADC();
    	if(flag_ADC == 1)
    	{
////////////////////////////////////////////////////////////////////////////////////////////////////////////

    	    //  Current control algorithm for the EV BUCK   //
    	    //EPwm4Regs.CMPA.bit.CMPA = Current_Control_Buck(Iev_ref_POSITIVE, Kp_ev, Ki_ev, Iev, EPwm4Regs.CMPA.bit.CMPA);

            //  Current control algorithm for the EV BOOST  //
    	    //Pwm4Regs.CMPA.bit.CMPA = Current_Control_Boost(Iev_ref_NEGATIVE, Kp_ev, Ki_ev, Iev, EPwm4Regs.CMPA.bit.CMPA);

////////////////////////////////////////////////////////////////////////////////////////////////////////////

    	    // MPPT control algorithm
    	    /*VpvMed += Vpv/5000;
    	    IpvMed += Ipv/5000;
            cont++;

            if(cont > 5000)
            {
                MPPT_algorithm(VpvMed, IpvMed);
                cont = 0;
                VpvMed = 0;
                IpvMed = 0;
            }
            */


////////////////////////////////////////////////////////////////////////////////////////////////////////////

    	    //  Current control algorithm for the H2 Stacked Buck   //
    	    //EPwm2Regs.CMPA.bit.CMPA = Current_Control_H2(Ih2_ref, Kp_h2, Ki_h2, Ih2, EPwm2Regs.CMPA.bit.CMPA);
            //EPwm1Regs.CMPA.bit.CMPA = EPwm2Regs.CMPA.bit.CMPA;

////////////////////////////////////////////////////////////////////////////////////////////////////////////

            //  Current control algorithm for the ESS 2 H2 mode
    	    //EPwm4Regs.CMPA.bit.CMPA = Current_Control_Boost(Iev_ref_NEGATIVE, Kp_ev, Ki_ev, Iev, EPwm4Regs.CMPA.bit.CMPA);
            EPwm2Regs.CMPA.bit.CMPA = Current_Control_H2(Ih2_ref, Kp_h2, Ki_h2, Ih2, EPwm2Regs.CMPA.bit.CMPA);
            EPwm1Regs.CMPA.bit.CMPA = EPwm2Regs.CMPA.bit.CMPA;
    	}
    }

}


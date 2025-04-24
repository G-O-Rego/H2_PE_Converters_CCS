/*
 * functions.h
 *
 *  Created on: 25 Sep 2024
 *      Author: gonca
 */

#ifndef FUNCTIONS_H_
#define FUNCTIONS_H_

#define OFF 0
#define BUCK 1
#define BOOST 2
#define H2 3
#define PV 4
#define PV_2_H2 5
#define ESS_2_H2 6

//functions
void SCIA_config(void);
void H2_config_GPIO(void);
void timer0_config(void);
//void H2_init_ePWM(void);
void adc_config(void);
void update_ADC(void);
void lock_pwm_pin(void);
void pwm_compare_1_config(void);
void pwm_compare_2_config(void);
void pwm_config(void);
void MPPT_algorithm(double, double);
void Select_Mode(void);
Uint16 Current_Control_Boost(float, float, float, float, Uint16);
Uint16 Current_Control_Buck(float, float, float, float, Uint16);
Uint16 Current_Control_H2(float, float, float, float, Uint16);
//Uint16 current_control_PV(float, float, float, float, Uint16);


extern int count;
extern int flag_timer_0;
extern int flag_ADC;
extern int Mode;
extern float Vdclink, Vh2, Vev, Vpv, Ih2, Iev, Ipv;
extern double VpvMed, IpvMed, cont;


#endif /* FUNCTIONS_H_ */


//  HOW TO TURN EACH CONVERTER IGBTS ON OR OFF

// Turn ON (0x00) or OFF (0x01) the PWM
// CAUTION WITH THE IGBT MODULE NUMBER AND THE EPWM REGISTER IN USE

// Electrolyser -> All ON
//EPwm1Regs.AQCSFRC.bit.CSFA= 0x01;         // IGBT 1 - Top
//EPwm1Regs.AQCSFRC.bit.CSFB= 0x01;         // IGBT 1 - Bot
//EPwm2Regs.AQCSFRC.bit.CSFA= 0x01;         // IGBT 2 - Top
//EPwm2Regs.AQCSFRC.bit.CSFB= 0x01;         // IGBT 2 - Bot

// EV Batteries
// Top IGBT ON & Bot OFF -> Buck
// Top IGBT OFF & Bot ON -> Boost
//EPwm4Regs.AQCSFRC.bit.CSFA= 0x01;         // IGBT 4 - Top
//EPwm4Regs.AQCSFRC.bit.CSFB= 0x01;         // IGBT 4 - Bot

// Solar PVs -> Works in Boost mode, therefore the top IGBT is
// ALWAYS *OFF* (0x01)
//EPwm7Regs.AQCSFRC.bit.CSFA= 0x01;         // IGBT 3 - Top (Do NOT change)
//EPwm7Regs.AQCSFRC.bit.CSFB= 0x01;         // IGBT 3 - Bot

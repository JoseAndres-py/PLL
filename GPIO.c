/*
 * GPIO.c
 *
 *  Created on: 1 sept. 2019
 *      Author: JOSEC
 */

#include "DSP2833x_Device.h"     // Headerfile Include File
#include "DSP2833x_Examples.h"   // Examples Include File
#include "GPIO.h"

//---------------------------------------------------------------------------
// Initialize bits DAC (GPIO):
//---------------------------------------------------------------------------
void Init_R2R()
{
    EALLOW;                                     //Enable Write Regs
    GpioCtrlRegs.GPAMUX1.all = 0x00000000;      //Sel GPIO General Port
    GpioCtrlRegs.GPAMUX2.all = 0x00000000;      //Sel GPIO General Port
    GpioCtrlRegs.GPADIR.all  = 0xFFFFFFFF;      //Sel GPIO Dir = Ouput
    GpioCtrlRegs.GPAPUD.all  = 0xFFFFFFFF;      //Enable Pull Up
    EDIS;                                       //Disable Write Regs

}


void Organize_Bits(Reg_32* Port,Reg_8 data)
{
    Port->bit.b14 = data.bit.b0;
    Port->bit.b13 = data.bit.b1;
    Port->bit.b11 = data.bit.b2;
    Port->bit.b9 = data.bit.b3;
    Port->bit.b7 = data.bit.b4;
    Port->bit.b5 = data.bit.b5;
    Port->bit.b3 = data.bit.b6;
    Port->bit.b1 = data.bit.b7;
}
void Send_Port(Reg_32 Port)
{
    GpioDataRegs.GPADAT.all = Port.all;
}




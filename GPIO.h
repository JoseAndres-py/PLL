/*
 * GPIO.h
 *
 *  Created on: 23/08/2017
 *      Author: yeisonint
 */

#ifndef GPIO_H_
#define GPIO_H_

//---------------------------------------------------------------------------
// Structures (GPIO):
//---------------------------------------------------------------------------


typedef struct
{
	Uint8 b0:1;
	Uint8 b1:1;
	Uint8 b2:1;
	Uint8 b3:1;
	Uint8 b4:1;
	Uint8 b5:1;
	Uint8 b6:1;
	Uint8 b7:1;
}B_8;

typedef union
{
	char all;
	B_8 bit;
}Reg_8;

typedef struct
{
    Uint8 b0:1;
    Uint8 b1:1;
    Uint8 b2:1;
    Uint8 b3:1;
    Uint8 b4:1;
    Uint8 b5:1;
    Uint8 b6:1;
    Uint8 b7:1;
    Uint8 b8:1;
    Uint8 b9:1;
    Uint8 b10:1;
    Uint8 b11:1;
    Uint8 b12:1;
    Uint8 b13:1;
    Uint8 b14:1;
    Uint8 b15:1;
    Uint8 b16:1;
    Uint8 b17:1;
    Uint8 b18:1;
    Uint8 b19:1;
    Uint8 b20:1;
    Uint8 b21:1;
    Uint8 b22:1;
    Uint8 b23:1;
    Uint8 b24:1;
    Uint8 b25:1;
    Uint8 b26:1;
    Uint8 b27:1;
    Uint8 b28:1;
    Uint8 b29:1;
    Uint8 b30:1;
    Uint8 b31:1;
}B_32;

typedef union
{
    Uint32 all;
    B_32 bit;
}Reg_32;


//---------------------------------------------------------------------------
// Functions GPIO(Register):
//---------------------------------------------------------------------------
void Init_R2R();
void Organize_Bits(Reg_32* Port,Reg_8 data);
void Send_Port(Reg_32 Port);

#endif /* GPIO_H_ */

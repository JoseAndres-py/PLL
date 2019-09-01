#include "DSP28x_Project.h"     // Device Headerfile and Examples Include File
#include "Solar_F.h"
#include "GPIO.h"

//---------------------------------------------------------------------------
// Prototype statements for functions (definitions):
//---------------------------------------------------------------------------
interrupt void adc_isr(void);
void Init_ADC(void);
void Init_PWM1(void);

//---------------------------------------------------------------------------
// ADC start parameters (definitions):
//---------------------------------------------------------------------------

#define ADC_MODCLK 0x3      // HSPCLK = SYSCLKOUT/2*ADC_MODCLK2 = 150/(2*3)   = 25.0 MHz
#define ADC_CKPS   0x1      // ADC module clock = HSPCLK/2*ADC_CKPS   = 25.0MHz/(1*2) = 12.5MHz
#define ADC_SHCLK  0xf      // S/H width in ADC module periods                        = 16 ADC clocks
#define BUF_SIZE   1        // Sample buffer size
#define TBPRD_usr  4882

float32 SampleTable[1024];
float32 PLLTable[1024];

//---------------------------------------------------------------------------
// SPLL_1ph (Solar Library):
//---------------------------------------------------------------------------
SPLL_1ph_F spll1;
SPLL_1ph_F_NOTCH_COEFF spll_notch_coef1;
#define ISR_FREQUENCY   30725
#define GRID_FREQ       60
#define pi              3.141592653589793238462643383279502884

void main()
{
   InitSysCtrl();           // Initialize System Control

   DINT;                    // Disable CPU interrupts

   InitPieCtrl();           // Initialize the PIE control registers to their default state.

   IER = 0x0000;            // Disable CPU interrupts
   IFR = 0x0000;            // Clear all CPU interrupt flags

   InitPieVectTable();      // Initialize the PIE vector table with pointers to the shell Interrupt
   InitAdc();               // Initialize ADC

   // Interrupts that are used ADC
   EALLOW;                  // Enable Write Regs
   PieVectTable.ADCINT = &adc_isr;
   EDIS;                    // Disable Write Regs

   // Enable ADCINT in PIE
   PieCtrlRegs.PIEIER1.bit.INTx6 = 1;
   IER |= M_INT1;           // Enable CPU Interrupt 1

   Init_R2R();
   Init_ADC();
   Init_PWM1();

   EINT;                    // Enable Global interrupt INTM
   ERTM;                    // Enable Global realtime interrupt DBGM

   //---------------------------------------------------------------------------
   // Pll routine (1ph):
   //---------------------------------------------------------------------------
   SPLL_1ph_F_init(GRID_FREQ,((float)(1.0/ISR_FREQUENCY)), &spll1);
   SPLL_1ph_F_notch_coeff_update(((float)(1.0/ISR_FREQUENCY)), (float)(2*pi*GRID_FREQ*2),(float)0.00001,(float)0.1, &spll1);

   for(;;){}
}

void Init_ADC()
{
    //---------------------------------------------------------------------------
    // Specific clock (setting for ADC):
    //---------------------------------------------------------------------------
    EALLOW;                  // Enable Write Regs
    SysCtrlRegs.HISPCP.all = ADC_MODCLK; // HSPCLK = SYSCLKOUT/ADC_MODCLK
    EDIS;                    // Disable Write Regs

    //---------------------------------------------------------------------------
    // Configure ADC(Register):
    //---------------------------------------------------------------------------
    AdcRegs.ADCMAXCONV.all = 0x0001;                            // Setup 4 conv's on SEQ1
    AdcRegs.ADCTRL1.bit.ACQ_PS = ADC_SHCLK;
    AdcRegs.ADCTRL3.bit.ADCCLKPS = ADC_CKPS;
    AdcRegs.ADCTRL1.bit.SEQ_CASC = CT_SEQ_CASQ;                 // Cascaded mode
    AdcRegs.ADCTRL1.bit.CONT_RUN = CT_CONT;                     // Disable continuous run
    AdcRegs.ADCTRL2.bit.EPWM_SOCA_SEQ1 = CT_SOCA_PWM_ENABLE;    // Enable SOCA from ePWM to start SEQ1
    AdcRegs.ADCTRL2.bit.INT_ENA_SEQ1 = CT_INT_ENABLE;           // Enable SEQ1 interrupt (every EOS)
    AdcRegs.ADCTRL2.bit.INT_MOD_SEQ1 = CT_INT_EOS;              // Enable Interrupt

    //---------------------------------------------------------------------------
    // Configure ADC Sequencer (Pin ADC):
    //---------------------------------------------------------------------------
    AdcRegs.ADCCHSELSEQ1.bit.CONV00 = ADCINA0;   // Setup ADCINA0 as 1st SEQ1 conv

    //---------------------------------------------------------------------------
    // Frecuencia de Muestreo (GPIO Toggle):
    //---------------------------------------------------------------------------
    EALLOW;
    GpioCtrlRegs.GPAPUD.bit.GPIO2= 0;        // Enable pull-up on GPIO0 (EPWM1A)
    GpioCtrlRegs.GPAMUX1.bit.GPIO2 = 0;      // Configure GPIO1 as EPWM1B
    GpioCtrlRegs.GPADIR.bit.GPIO2 = 1;
    EDIS;
}

void Init_PWM1()
{
    //InitEPwm1Gpio();

    //---------------------------------------------------------------------------
    // Configure PWM(Register):
    //---------------------------------------------------------------------------
    EPwm1Regs.TBPRD = TBPRD_usr;                // Set period for ePWM1
    EPwm1Regs.TBCTL.bit.CTRMODE = TB_COUNT_UP;  //Forma del conteo
    EPwm1Regs.TBCTL.bit.PRDLD = TB_SHADOW;      // Registro auxiliar oara el periodo PLD
    EPwm1Regs.TBCTL.bit.HSPCLKDIV = TB_DIV1;    // Divisor de frecuencia para PWM
    EPwm1Regs.TBCTL.bit.CLKDIV = TB_DIV1;       // Divisor de frecuencia para PWM
    EPwm1Regs.CMPA.half.CMPA = TBPRD_usr/2;

    //---------------------------------------------------------------------------
    // Duty Cycle(Register):
    //---------------------------------------------------------------------------
    EPwm1Regs.AQCTLA.bit.PRD = AQ_CLEAR;
    EPwm1Regs.AQCTLA.bit.CAU = AQ_SET;

    //---------------------------------------------------------------------------
    // ADC triger whit PWM ():
    //---------------------------------------------------------------------------
    EPwm1Regs.ETSEL.bit.SOCAEN = ET_1ST;        // Enable SOC on A group
    EPwm1Regs.ETSEL.bit.SOCASEL = ET_CTR_ZERO;  // Select SOC from zero
    EPwm1Regs.ETPS.bit.SOCAPRD = ET_1ST;        // Generate pulse on 1st event

}

interrupt void  adc_isr(void)
{
    static int i = 0;
    //---------------------------------------------------------------------------
    // Medida ADC (Diferencial):
    //---------------------------------------------------------------------------

    int32 GridMeas=((int32)(AdcMirror.ADCRESULT0))-((int32)(AdcMirror.ADCRESULT1));

    //---------------------------------------------------------------------------
    // PLL (Libreria SolarLib):
    //---------------------------------------------------------------------------
    spll1.AC_input = (float32)(GridMeas)/3200;
    SPLL_1ph_F_FUNC(&spll1);

    //---------------------------------------------------------------------------
    // Datos de verificacion (Toggle,Buffer,R2R):
    //---------------------------------------------------------------------------
    if(spll1.theta[0] <= 0.001){GpioDataRegs.GPATOGGLE.bit.GPIO2 = 1;};
    SampleTable[i] = (float32)spll1.AC_input;
    PLLTable[i] = spll1.theta[0];

    Reg_32 GPIO_org; Reg_8 theta;
    theta.all = (Uint8)(spll1.sin[0]*56);
    Organize_Bits(&GPIO_org, theta);
    Send_Port(GPIO_org);

    //---------------------------------------------------------------------------
    // Terminar Interrupcion (Clear Interrupt):
    //---------------------------------------------------------------------------
    i++;
    if(i>=1024){i=0;}
    // Reinitialize for next ADC sequence
    AdcRegs.ADCTRL2.bit.RST_SEQ1 = 1;         // Reset SEQ1
    AdcRegs.ADCST.bit.INT_SEQ1_CLR = 1;       // Clear INT SEQ1 bit
    PieCtrlRegs.PIEACK.all = PIEACK_GROUP1;   // Acknowledge interrupt to PIE

    return;
}

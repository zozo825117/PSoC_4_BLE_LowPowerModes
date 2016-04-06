/******************************************************************************
* Project Name		: PSoC_4_BLE_LowPowerModes
* File Name			: main.c
* Version 			: 1.0
* Device Used		: CY8C4247LQI-BL483
* Software Used		: PSoC Creator 3.1
* Compiler    		: ARM GCC 4.8.4
* Related Hardware	: CY8CKIT-042-BLE Bluetooth Low Energy Pioneer Kit 
* Owner             : ANKU
*
********************************************************************************
* Copyright (2014-15), Cypress Semiconductor Corporation. All Rights Reserved.
********************************************************************************
* This software is owned by Cypress Semiconductor Corporation (Cypress)
* and is protected by and subject to worldwide patent protection (United
* States and foreign), United States copyright laws and international treaty
* provisions. Cypress hereby grants to licensee a personal, non-exclusive,
* non-transferable license to copy, use, modify, create derivative works of,
* and compile the Cypress Source Code and derivative works for the sole
* purpose of creating custom software in support of licensee product to be
* used only in conjunction with a Cypress integrated circuit as specified in
* the applicable agreement. Any reproduction, modification, translation,
* compilation, or representation of this software except as specified above 
* is prohibited without the express written permission of Cypress.
*
* Disclaimer: CYPRESS MAKES NO WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, WITH 
* REGARD TO THIS MATERIAL, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED 
* WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
* Cypress reserves the right to make changes without further notice to the 
* materials described herein. Cypress does not assume any liability arising out 
* of the application or use of any product or circuit described herein. Cypress 
* does not authorize its products for use as critical components in life-support 
* systems where a malfunction or failure may reasonably be expected to result in 
* significant injury to the user. The inclusion of Cypress' product in a life-
* support systems application implies that the manufacturer assumes all risk of 
* such use and in doing so indemnifies Cypress against all charges. 
*
* Use of this Software may be limited by and subject to the applicable Cypress
* software license agreement. 
*******************************************************************************/

/******************************************************************************
*                           THEORY OF OPERATION
*******************************************************************************
* This project will showcase the capability of PSoC 4 BLE to communicate 
* bi-directionally with a BLE Central device over custom services. 
* The LPM SEL custom service allows 
* read and write of attributes under the LPM characteristics.
* This allows the selection of the low power mode operation, ideal for battery operated 
* devices. The project utlizes Deep Sleep feature of both BLESS and CPU to remain 
* in low power mode as much as possible, while maintaining the BLE connection and  
* data transfer. This allows the device to run on coin cell battery for long time.
*
* Note:
* The programming pins have been configured as GPIO, and not SWD. This is because 
* when programming pins are configured for SWD, then the silicon consumes extra
* power through the pins. To prevent the leakage of power, the pins have been set 
* to GPIO. With this setting, the kit can still be acquired by PSoC Creator or
* PSoC Programmer software tools for programming, but the project cannot be 
* externally debugged. To re-enable debugging, go to PSoC_4_BLE_CapSense_Slider_LED.cydwr 
* from Workspace Explorer, go to Systems tab, and set the Debug Select option to 'SWD'.
* Build and program this project to enable external Debugging.
*
* Refer to BLE Pioneer Kit user guide for details.
*******************************************************************************
* Hardware connection required for testing -
* R-G-B LED 	- P2[6], P3[6] and P3[7] (hard-wired on the BLE Pioneer kit)
* User Switch	- P2[7] (hard-wired on the BLE Pioneer kit)
******************************************************************************/
#include <main.h>

/* This flag is used by application to know whether a Central 
* device has been connected. This is updated in BLE event callback 
* function*/
extern uint8 deviceConnected;

/* 'restartAdvertisement' flag is used to restart advertisement */
extern uint8 restartAdvertisement;

uint8 p_state = 0;


void TxCW(void)
{
	uint32 cfg2,cfgctrl,sy;

    //// set CW mode, To prevent first time TX  frequency offset before CW mode enabled in case.
   CY_SET_XTND_REG32((void CYFAR *)(CYREG_BLE_BLERD_MODEM), 0x96EC);        
   
   // Configure DSM as first order PLL
   CY_SET_XTND_REG32((void CYFAR *)(CYREG_BLE_BLERD_CFGCTRL),0x0008);

   // Enable Transmit at the required TX frequency
   CY_SET_XTND_REG32((void CYFAR *)(CYREG_BLE_BLERD_CFG1), 0xBB48);
   CY_SET_XTND_REG32((void CYFAR *)(CYREG_BLE_BLERD_DBUS), 0xE962);   // 0x992 is channel frequency in MHz


    // Synchronisation delay for PA to ramp
    CyDelayUs(120);
    
    // Disable modulation port
    cfg2= CY_GET_XTND_REG32((void CYFAR *)(CYREG_BLE_BLERD_CFG2));
	cfg2 |=0x1000;    
	cfg2 &=~0x03FF;                     
	cfg2 |=0x0200;    
     
    // Enable test mode
    cfgctrl= CY_GET_XTND_REG32((void CYFAR *)(CYREG_BLE_BLERD_CFGCTRL));
    cfgctrl |=0x8000; 

    // Close the loop
    sy= CY_GET_XTND_REG32((void CYFAR *)(CYREG_BLE_BLERD_SY));
    sy |=0x4000;      

    CY_SET_XTND_REG32((void CYFAR *)(CYREG_BLE_BLERD_CFG2), cfg2);   
    CY_SET_XTND_REG32((void CYFAR *)(CYREG_BLE_BLERD_CFGCTRL), cfgctrl); 
    CY_SET_XTND_REG32((void CYFAR *)(CYREG_BLE_BLERD_SY),sy);   
    
}

/*******************************************************************************
* Function Name: main
********************************************************************************
* Summary:
*        System entrance point. This calls the initializing function and
* continuously process BLE and CapSense events.
*
* Parameters:
*  void
*
* Return:
*  int
*

*******************************************************************************/
int main()
{
    uint8 lpmSel = DEEPSLEEP; 
	/* This function will initialize the system resources such as BLE and CapSense */
    //InitializeSystem();
  #if 1
	/* Enable global interrupt mask */
	CyGlobalIntEnable; 
	#endif
	/* Start BLE component and register the CustomEventHandler function. This 
	* function exposes the events from BLE component for application use */
    CyBle_Start(CustomEventHandler);
    
    for(;;)
    {
        /*Process event callback to handle BLE events. The events generated and 
		* used for this application are inside the 'CustomEventHandler' routine*/
        CyBle_ProcessEvents();
		
      if(p_state == 1)
      {
        TxCW();
        p_state = 0;
        
        while(User_Button_Read())
        {
          	CyDelay(200);
						RED_Write(~RED_Read());
        }
        RED_Write(1);
      }


			HandleLowPowerMode(lpmSel);




    }	/* End of for(;;) */
}

/*******************************************************************************
* Function Name: InitializeSystem
********************************************************************************
* Summary:
*        Start the components and initialize system 
*
* Parameters:
*  void
*
* Return:
*  void
*
*******************************************************************************/
void InitializeSystem(void)
{

	
#if 0
	/* Start the Button ISR to allow wakeup from sleep */
	isr_button_StartEx(MyISR);
	
	/* Set the Watchdog Interrupt vector to the address of Interrupt routine 
	* WDT_INT_Handler. This routine counts the 3 seconds for LED ON state during
	* connection. */
	CyIntSetVector(WATCHDOG_INT_VEC_NUM, &WDT_INT_Handler);
  #endif
}

/* [] END OF FILE */

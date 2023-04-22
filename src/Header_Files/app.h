//***********************************************************************************
// Include files
//***********************************************************************************
#ifndef	APP_HG
#define	APP_HG

/* System include statements */


/* Silicon Labs include statements */
#include "em_cmu.h"
#include "em_assert.h"

/* The developer's include statements */
#include "cmu.h"
#include "gpio.h"
#include "letimer.h"
#include "brd_config.h"
#include "scheduler.h"
#include "SI7021.h"
#include "ble.h"
#include "HW_delay.h"
#include "veml6030.h"

#include "stdio.h"
#include "string.h"
#include "stdlib.h"



//***********************************************************************************
// defined files
//***********************************************************************************
#define		PWM_PER				1		// PWM period in seconds
#define		PWM_ACT_PER			0.25	// PWM active period in seconds

#define		SYSTEM_BLOCK_EM		EM3

// Application scheduled events
#define 	LETIMER0_COMP0_CB 0x00000001 //0b0001
#define 	LETIMER0_COMP1_CB 0x00000002 //0b0010
#define 	LETIMER0_UF_CB    0x00000004 //0b0100

#define 	SI7021_H_READ_CB  0x00000008 //0b1000

#define		BOOT_UP_CB		  0x00000010 //0b10000

#define		BLE_TX_DONE_CB	  0x00000040 //0b100000
#define 	BLE_RX_DONE_CB	  0x00000080 //0b1000000

#define		VEML6030_READ_CB  0x00000100 //0b10000000

#define		SI7021_T_READ_CB  0x00000200



//***********************************************************************************
// global variables
//***********************************************************************************


//***********************************************************************************
// function prototypes
//***********************************************************************************
void app_peripheral_setup(void);
void scheduled_letimer0_uf_cb (void);
void scheduled_letimer0_comp0_cb (void);
void scheduled_letimer0_comp1_cb (void);
void humidity_done_cb(void);
void temp_done_cb(void);
void light_done_cb(void);
void scheduled_boot_up_cb(void);
void scheduled_ble_tx_done_cb(void);

#endif

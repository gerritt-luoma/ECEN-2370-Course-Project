/*
 * ble.h
 *
 *      Updated on: 4/21/23
 *      Author: Gerritt Luoma
 */
//***********************************************************************************
// Include files
//***********************************************************************************
#ifndef	BLE_GUARD_H
#define	BLE_GUARD_H

//** Standard Libraries
#include <stdbool.h>
#include <stdint.h>

// Driver functions
#include "leuart.h"
#include "gpio.h"


//***********************************************************************************
// defined files
//***********************************************************************************

#define HM10_LEUART0		LEUART0
#define HM10_BAUDRATE		9600
#define	HM10_DATABITS		leuartDatabits8 // 8 data bits
#define HM10_ENABLE			leuartEnable // Enable RX/TX when init completed
#define HM10_PARITY			leuartNoParity // No parity bits in use
#define HM10_REFFREQ		0				// use reference clock
#define HM10_STOPBITS		leuartStopbits1 // 1 stop bit

// Route to location 18 (expansion header)
#define LEUART0_TX_ROUTE	LEUART_ROUTELOC0_TXLOC_LOC18   	// Route to PD11
#define LEUART0_RX_ROUTE	LEUART_ROUTELOC0_RXLOC_LOC18   	// Route to PD10


//***********************************************************************************
// global variables
//***********************************************************************************


//***********************************************************************************
// function prototypes
//***********************************************************************************
void ble_open(uint32_t tx_event, uint32_t rx_event);
void ble_write(char *string);

bool ble_test(char *mod_name);

#endif

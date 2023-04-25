/**
 * @file leuart.c
 * @author Gerritt Luoma
 * @date 3/16/2021
 * @brief Contains all the functions of the LEUART peripheral
 *
 */

//***********************************************************************************
// Include files
//***********************************************************************************

//** Standard Library includes
#include <string.h>

//** Silicon Labs include files
#include "em_gpio.h"
#include "em_cmu.h"

//** Developer/user include files
#include "leuart.h"
#include "scheduler.h"

//***********************************************************************************
// defined files
//***********************************************************************************


//***********************************************************************************
// private variables
//***********************************************************************************
uint32_t	rx_done_evt;
uint32_t	tx_done_evt;
bool		leuart0_tx_busy;

static LEUART_STATE_MACHINE 	 leuart_state;

/***************************************************************************//**
 * @brief LEUART driver
 * @details
 *  This module contains all the functions to support the driver's state
 *  machine to transmit a string of data across the LEUART bus.  There are
 *  additional functions to support the Test Driven Development test that
 *  is used to validate the basic set up of the LEUART peripheral.  The
 *  TDD test for this class assumes that the LEUART is connected to the HM-18
 *  BLE module.  These TDD support functions could be used for any TDD test
 *  to validate the correct setup of the LEUART.
 *
 ******************************************************************************/

//***********************************************************************************
// Private functions
//***********************************************************************************

static void leuart_txbl(LEUART_STATE_MACHINE *leuart_state);
static void leuart_txc(LEUART_STATE_MACHINE *leuart_state);

//***********************************************************************************
// Global functions
//***********************************************************************************

/***************************************************************************//**
 * @brief
 *   Function to open and set a leuart peripheral
 *
 * @details
 * 	 This routine opens one of the LEUART peripherals be enabling the correct clock,
 * 	 verifying correct clock operation, initializing the LEUART, and enabling interrupts
 *
 * @note
 *   This function is called once in the beginning, from ble_open
 *
 * @param[in] *leuart
 *   Pointer to the base peripheral address of the leuart peripheral being opened
 *
 * @param[in] *leuart_settings
 *   Pointer to the struct used to set up this leuart
 *
 ******************************************************************************/

void leuart_open(LEUART_TypeDef *leuart, LEUART_OPEN_STRUCT *leuart_settings){
	// Enable the clock for the selected LEUART
	if(leuart == LEUART0) {
		CMU_ClockEnable(cmuClock_LEUART0, true);
	}
	else {
		// We are only using LEUART0
		EFM_ASSERT(false);
	}

	// verify proper clock operation
	if ((leuart->STARTFRAME & 0x01) == 0) {
		leuart->STARTFRAME = 0x01;
		while(leuart->SYNCBUSY);
	}
	EFM_ASSERT(leuart->STARTFRAME & 0x01);
	leuart->STARTFRAME = 0x00;
	while(leuart->SYNCBUSY);

	LEUART_Init_TypeDef leuart_in;
	leuart_in.baudrate = leuart_settings->baudrate;
	leuart_in.databits = leuart_settings->databits;
	leuart_in.parity = leuart_settings->parity;
	leuart_in.stopbits = leuart_settings->stopbits;
	leuart_in.refFreq = HM10_REFFREQ;
	leuart_in.enable = HM10_ENABLE;

	LEUART_Init(leuart, &leuart_in);

	while(leuart->SYNCBUSY);

	// Set the route location and enable pins
	leuart->ROUTELOC0 = leuart_settings->rx_loc | leuart_settings->tx_loc;
	leuart->ROUTEPEN = (leuart_settings->rx_pin_en * leuart_settings->rx_en) | (leuart_settings->tx_pin_en * leuart_settings->tx_en);

	// Clear rx and tx buffers
	leuart->CMD = LEUART_CMD_CLEARRX | LEUART_CMD_CLEARTX;
	while(leuart->SYNCBUSY);

	// Enable rx and tx
	if(leuart_settings->enable) {
		leuart->CMD = LEUART_CMD_TXEN;
		while(!(leuart->STATUS & LEUART_STATUS_TXENS));
	}
	if(leuart_settings->enable) {
		leuart->CMD = LEUART_CMD_RXEN;
		while(!(leuart->STATUS & LEUART_STATUS_RXENS));
	}

	// Enable leuart
	LEUART_Enable(leuart, leuart_settings->enable);

	// clear TXBL interrupts
	LEUART0->IFC = LEUART_IF_TXBL;


	NVIC_EnableIRQ(LEUART0_IRQn);

}

/***************************************************************************//**
 * @brief
 *   Function to handle interrupts for LEUART0
 *
 * @details
 * 	 This routine checks whether there is an interrupt, and whether it is TXBL or TXC
 * 	 Then calls the function for the specific interrupt
 *
 * @note
 *   This function is called to any time there is an interrupt of any type
 *
 ******************************************************************************/

void LEUART0_IRQHandler(void){
	 uint32_t int_flag; // store source interrupts

	 //AND the interrupt source (IF), with the interrupt enable register (IEN)
	 // your interrupt source variable will only contain interrupts of interest
	 int_flag = LEUART0->IF & LEUART0->IEN;

	 //clear interrupt flag register
	 LEUART0->IFC = int_flag;

	 if (int_flag & LEUART_IF_TXBL){
		 leuart_txbl(&leuart_state);
	 }
	 if (int_flag & LEUART_IF_TXC){
		 leuart_txc(&leuart_state);
	 }
}

/***************************************************************************//**
 * @brief
 *   Function that handles the TXBL interrupt
 *
 * @details
 * 	 This routine completes different actions depending on the state of the state
 * 	 machine when the TXBL interrupt is received
 *
 * @note
 *   This function is called when an TXBL interrupt is received
 *
 ******************************************************************************/

static void leuart_txbl(LEUART_STATE_MACHINE *leuart_state) {
	switch(leuart_state->state) {
		case EnableTransfer: {
			//enable txbl
			LEUART0->IFC = LEUART_IF_TXBL;
			LEUART0->IEN |= LEUART_IF_TXBL;
			leuart_state->state = TransferCharacters;
			break;
		}
		case TransferCharacters: {
			//increment count
			leuart_app_transmit_byte(LEUART0, leuart_state->string[leuart_state->count]);
			leuart_state->count++;
			if (leuart_state->count == leuart_state->length){
				LEUART0->IEN &= ~LEUART_IF_TXBL;
				//enable txc
				LEUART0->IFC = LEUART_IF_TXC;
				LEUART0->IEN |= LEUART_IF_TXC;
				leuart_state->state = EndTransfer;
			}
			break;
		}
		case EndTransfer: {
			EFM_ASSERT(false);
			break;
		}
		default: {
			EFM_ASSERT(false);
			break;
		}
	}
}

/***************************************************************************//**
 * @brief
 *   Function that handles the TXC interrupt
 *
 * @details
 * 	 This routine completes different actions depending on the state of the state
 * 	 machine when the TXC interrupt is received
 *
 * @note
 *   This function is called when an TXC interrupt is received
 *
 ******************************************************************************/

static void leuart_txc(LEUART_STATE_MACHINE *leuart_state) {
	switch(leuart_state->state) {
		case EnableTransfer: {
			EFM_ASSERT(false);
			break;
		}
		case TransferCharacters: {
			EFM_ASSERT(false);
			break;
		}
		case EndTransfer: {
			//unblock sleep mode
			//set done event
			sleep_unblock_mode(LEUART_TX_EM);
			add_scheduled_event(leuart_state->callback);
			leuart_state->state = EnableTransfer;
			LEUART0->IEN &= ~LEUART_IF_TXC;
			leuart_state->tx_busy = false;
			break;
		}
		default: {
			EFM_ASSERT(false);
			break;
		}
	}
}

/***************************************************************************//**
 * @brief
 *   Function to start LEUART
 *
 * @details
 * 	 This routine sets up the LEUART state machine struct to initialize the state machine
 *
 * @note
 *   This function is called from ble_write at every humidity done event
 *
 * @param[in] *leuart
 *   Pointer to the base peripheral address of the leuart peripheral being opened
 *
 * @param[in] *string
 *   Is the string being written
 *
 * @param[in] string_len
 *   Is the length of the string being written
 *
 ******************************************************************************/

void leuart_start(LEUART_TypeDef *leuart, char *string, uint32_t string_len){
	// triggers if i2c peripheral has not finished pervious i2c operation
	sleep_block_mode(LEUART_TX_EM);

	leuart_state.count = 0;
	leuart_state.length = string_len;
	strcpy(leuart_state.string, string);
	leuart_state.leuart = leuart;
	leuart_state.state = EnableTransfer;
	leuart_state.callback = BLE_TX_DONE_CB;
	leuart_state.tx_busy = true;

	LEUART0->IEN |= LEUART_IF_TXBL;
}

/***************************************************************************//**
 * @brief
 *   Not used
 *
 ******************************************************************************/

bool leuart_tx_busy(LEUART_TypeDef *leuart){
	return leuart_state.tx_busy;
}

/***************************************************************************//**
 * @brief
 *   LEUART STATUS function returns the STATUS of the peripheral for the
 *   TDD test
 *
 * @details
 * 	 This function enables the LEUART STATUS register to be provided to
 * 	 a function outside this .c module.
 *
 * @param[in] *leuart
 *   Defines the LEUART peripheral to access.
 *
 * @return
 * 	 Returns the STATUS register value as an uint32_t value
 *
 ******************************************************************************/

uint32_t leuart_status(LEUART_TypeDef *leuart){
	uint32_t	status_reg;
	status_reg = leuart->STATUS;
	return status_reg;
}

/***************************************************************************//**
 * @brief
 *   LEUART CMD Write sends a command to the CMD register
 *
 * @details
 * 	 This function is used by the TDD test function to program the LEUART
 * 	 for the TDD tests.
 *
 * @note
 *   Before exiting this function to update  the CMD register, it must
 *   perform a SYNCBUSY while loop to ensure that the CMD has by synchronized
 *   to the lower frequency LEUART domain.
 *
 * @param[in] *leuart
 *   Defines the LEUART peripheral to access.
 *
 * @param[in] cmd_update
 * 	 The value to write into the CMD register
 *
 ******************************************************************************/

void leuart_cmd_write(LEUART_TypeDef *leuart, uint32_t cmd_update){

	leuart->CMD = cmd_update;
	while(leuart->SYNCBUSY);
}

/***************************************************************************//**
 * @brief
 *   LEUART IF Reset resets all interrupt flag bits that can be cleared
 *   through the Interrupt Flag Clear register
 *
 * @details
 * 	 This function is used by the TDD test program to clear interrupts before
 * 	 the TDD tests and to reset the LEUART interrupts before the TDD
 * 	 exits
 *
 * @param[in] *leuart
 *   Defines the LEUART peripheral to access.
 *
 ******************************************************************************/

void leuart_if_reset(LEUART_TypeDef *leuart){
	leuart->IFC = 0xffffffff;
}

/***************************************************************************//**
 * @brief
 *   LEUART App Transmit Byte transmits a byte for the LEUART TDD test
 *
 * @details
 * 	 The BLE module will respond to AT commands if the BLE module is not
 * 	 connected to the phone app.  To validate the minimal functionality
 * 	 of the LEUART peripheral, write and reads to the LEUART will be
 * 	 performed by polling and not interrupts.
 *
 * @note
 *   In polling a transmit byte, a while statement checking for the TXBL
 *   bit in the Interrupt Flag register is required before writing the
 *   TXDATA register.
 *
 * @param[in] *leuart
 *   Defines the LEUART peripheral to access.
 *
 * @param[in] data_out
 *   Byte to be transmitted by the LEUART peripheral
 *
 ******************************************************************************/

void leuart_app_transmit_byte(LEUART_TypeDef *leuart, uint8_t data_out){
	while (!(leuart->IF & LEUART_IF_TXBL));
	leuart->TXDATA = data_out;
}


/***************************************************************************//**
 * @brief
 *   LEUART App Receive Byte polls a receive byte for the LEUART TDD test
 *
 * @details
 * 	 The BLE module will respond to AT commands if the BLE module is not
 * 	 connected to the phone app.  To validate the minimal functionality
 * 	 of the LEUART peripheral, write and reads to the LEUART will be
 * 	 performed by polling and not interrupts.
 *
 * @note
 *   In polling a receive byte, a while statement checking for the RXDATAV
 *   bit in the Interrupt Flag register is required before reading the
 *   RXDATA register.
 *
 * @param[in] leuart
 *   Defines the LEUART peripheral to access.
 *
 * @return
 * 	 Returns the byte read from the LEUART peripheral
 *
 ******************************************************************************/

uint8_t leuart_app_receive_byte(LEUART_TypeDef *leuart){
	uint8_t leuart_data;
	while (!(leuart->IF & LEUART_IF_RXDATAV));
	leuart_data = leuart->RXDATA;
	return leuart_data;
}

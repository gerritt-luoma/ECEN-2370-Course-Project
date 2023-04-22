/**
 * @file sleep_routines.c
 * @author Gerritt Luoma
 * @date February 18th, 2021
 * @brief Contains all the sleep routine functions
 *
 */

/**************************************************************************
* @file sleep.c
***************************************************************************
* @section License
* <b>(C) Copyright 2015 Silicon Labs, http://www.silabs.com</b>
***************************************************************************
*
* Permission is granted to anyone to use this software for any purpose,
* including commercial applications, and to alter it and redistribute it
* freely, subject to the following restrictions:
*
* 1. The origin of this software must not be misrepresented; you must not
* claim that you wrote the original software.
* 2. Altered source versions must be plainly marked as such, and must not be
* misrepresented as being the original software.
* 3. This notice may not be removed or altered from any source distribution.
*
* DISCLAIMER OF WARRANTY/LIMITATION OF REMEDIES: Silicon Labs has no
* obligation to support this Software. Silicon Labs is providing the
* Software "AS IS", with no express or implied warranties of any kind,
* including, but not limited to, any implied warranties of merchantability
* or fitness for any particular purpose or warranties against infringement
* of any proprietary rights of a third party.
*
* Silicon Labs will not be liable for any consequential, incidental, or
* special damages, or any other relief, or for any claim by any third party,
* arising from your use of this Software.
*
**************************************************************************/

//***********************************************************************************
// Include files
//***********************************************************************************

#include "sleep_routines.h"

//***********************************************************************************
// Private variables
//***********************************************************************************

static int lowest_energy_mode[MAX_ENERGY_MODES];

//***********************************************************************************
// Functions
//***********************************************************************************

/***************************************************************************//**
 * @brief
 *   Function to initialize sleep routines
 *
 * @details
 * 	 This routine initializes the array of all energy modes to 0
 *
 * @note
 *   This function is one in the beginning, in app_peripheral_setup
 *
 ******************************************************************************/

void sleep_open(void) {
	//Initialize the sleep_routines static /private variable, lowest_energy_mode[]
	for(int i = 0; i < MAX_ENERGY_MODES; i++) {
		lowest_energy_mode[i] = 0;
	}
}

/***************************************************************************//**
 * @brief
 *   Function to block a sleep mode
 *
 * @details
 * 	 This routine blocks the energy mode passed as a parameter
 *
 * @note
 *   This function is called to block an energy mode
 *
 * @param[in] EM
 *   Is the energy mode to be blocked
 *
 ******************************************************************************/

void sleep_block_mode(uint32_t EM) {
	//Utilized by a peripheral to prevent the Pearl Gecko going into that sleep mode while the peripheral is active.
	CORE_DECLARE_IRQ_STATE;
	CORE_ENTER_CRITICAL();

	lowest_energy_mode[EM]++;
	EFM_ASSERT(lowest_energy_mode[EM] < 5);
	CORE_EXIT_CRITICAL();
}

/***************************************************************************//**
 * @brief
 *   Function to unblock a sleep mode
 *
 * @details
 * 	 This routine unblocks the energy mode passed as a parameter
 *
 * @note
 *   This function is called to unblock an energy mode
 *
 * @param[in] EM
 *   Is the energy mode to be unblocked
 *
 ******************************************************************************/

void sleep_unblock_mode(uint32_t EM) {
	//Utilized to release the processor from going into a sleep mode with a peripheral that is no longer active.
	CORE_DECLARE_IRQ_STATE;
	CORE_ENTER_CRITICAL();

	lowest_energy_mode[EM]--;
	EFM_ASSERT(lowest_energy_mode[EM] >= 0);
	CORE_EXIT_CRITICAL();
}

/***************************************************************************//**
 * @brief
 *   Function to enter sleep modes
 *
 * @details
 * 	 This routine checks which sleep mode is the lowest, and enters energy modes based on that
 *
 * @note
 *   This function is called to enter sleep
 *
 ******************************************************************************/

void enter_sleep(void) {
	//Function to enter sleep
	CORE_DECLARE_IRQ_STATE;
	CORE_ENTER_CRITICAL();
	if (lowest_energy_mode[EM0] > 0) {
	}
	else if (lowest_energy_mode[EM1] > 0) {
	}
	else if (lowest_energy_mode[EM2] > 0) {
		EMU_EnterEM1();
	}
	else if (lowest_energy_mode[EM3] > 0) {
		EMU_EnterEM2(1);
	}
	else {
		EMU_EnterEM3(1);
	}

	CORE_EXIT_CRITICAL();
	return;
}

/***************************************************************************//**
 * @brief
 *   Function to find the currently blocked energy mode
 *
 * @details
 * 	 This routine returns the energy mode that is currently blocked
 *
 * @note
 *   This function is called to find the current energy mode
 *
 ******************************************************************************/

uint32_t current_block_energy_mode(void) {
	//Function that returns which energy mode that the current system cannot enter.
	for(int i = 0; i < MAX_ENERGY_MODES; i++) {
		if(lowest_energy_mode[i] != 0) {
			return i;
		}
	}
	return MAX_ENERGY_MODES - 1;
}


/**
 * @file cmu.c
 * @author Gerritt Luoma
 * @date February 18th, 2021
 * @brief Sets up clock
 *
 */

//***********************************************************************************
// Include files
//***********************************************************************************
#include "cmu.h"

//***********************************************************************************
// defined files
//***********************************************************************************


//***********************************************************************************
// Private variables
//***********************************************************************************


//***********************************************************************************
// Private functions
//***********************************************************************************


//***********************************************************************************
// Global functions
//***********************************************************************************

/***************************************************************************//**
 * @brief
 *	Selects and enables clock
 *
 * @details
 *	First enables HF clock
 *	Then enables LFRCO and LFXO oscillators
 *	Then selects the correct clock tree for LETIMER0
 *	Finally, enables LF
 *
 * @note
 *	Called once at the beginning
 *
 ******************************************************************************/

void cmu_open(void){

		// Enable the High Frequency Peripheral Clock
		CMU_ClockEnable(cmuClock_HFPER, true);

		// By default, Low Frequency Resistor Capacitor Oscillator, LFRCO, is enabled,
		// Disable the LFRCO oscillator
		CMU_OscillatorEnable(cmuOsc_LFRCO , false, false);

		// Enable the Low Frequency Crystal Oscillator, LFXO
		CMU_OscillatorEnable(cmuOsc_LFXO, true, true);

		// Select the LFXO as the clock source for the LFB clock tree for LEUART0
		CMU_ClockSelectSet(cmuClock_LFB, cmuSelect_LFXO);

		// No requirement to enable the ULFRCO oscillator.  It is always enabled in EM0-4H

		// Select the ULFRCO as the clock source for the LFA clock tree for LETIMER0
		CMU_ClockSelectSet(cmuClock_LFA , cmuSelect_ULFRCO);

		// Now, you must ensure that the global Low Frequency is enabled
		CMU_ClockEnable(cmuClock_CORELE, true);

}


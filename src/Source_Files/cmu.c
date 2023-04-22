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

		CMU_ClockEnable(cmuClock_HFPER, true);

		// By default, Low Frequency Resistor Capacitor Oscillator, LFRCO, is enabled,
		// Disable the LFRCO oscillator
		CMU_OscillatorEnable(cmuOsc_LFRCO , false, false);	 // What is the enumeration required for LFRCO?

		// Enable the Low Frequency Crystal Oscillator, LFXO
		CMU_OscillatorEnable(cmuOsc_LFXO, true, true);	// What is the enumeration required for LFXO?
		CMU_ClockSelectSet(cmuClock_LFB, cmuSelect_LFXO);

		// No requirement to enable the ULFRCO oscillator.  It is always enabled in EM0-4H

		// Route LF clock to LETIMER0 clock tree
		CMU_ClockSelectSet(cmuClock_LFA , cmuSelect_ULFRCO);	// What clock tree does the LETIMER0 reside on?

		// Now, you must ensure that the global Low Frequency is enabled
		CMU_ClockEnable(cmuClock_CORELE, true);	//This enumeration is found in the Lab 2 assignment

}


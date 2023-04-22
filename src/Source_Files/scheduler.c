/**
 * @file scheduler.c
 * @author Gerritt Luoma
 * @date February 18th, 2021
 * @brief Contains all the scheduler functions
 *
 */

//***********************************************************************************
// Include files
//***********************************************************************************

#include "scheduler.h"
#include "em_assert.h"
#include "em_core.h"
#include "em_emu.h"

//***********************************************************************************
// Private variables
//***********************************************************************************

static unsigned int event_scheduled;

//***********************************************************************************
// Functions
//***********************************************************************************

/***************************************************************************//**
 * @brief
 *   Function to initialize the event_scheduled to none
 *
 * @details
 * 	 This routine (atomic) opens the scheduler by setting no events scheduled
 *
 * @note
 *   This function is called once in the beginning, in app_peripheral_setup
 *
 ******************************************************************************/

void scheduler_open(void) {
	CORE_DECLARE_IRQ_STATE;
	CORE_ENTER_CRITICAL();
	event_scheduled = 0;
	CORE_EXIT_CRITICAL();
}

/***************************************************************************//**
 * @brief
 *   Function to add a scheduled event
 *
 * @details
 * 	 This routine (atomic) adds the parameter event to event_scheduled
 *
 * @note
 *   This function is called when a new interrupt of any type is raised
 *
 * @param[in] event
 *   Is the event to be added
 *
 ******************************************************************************/

void add_scheduled_event(uint32_t event) {
	CORE_DECLARE_IRQ_STATE;
	CORE_ENTER_CRITICAL();
	event_scheduled |= event;
	CORE_EXIT_CRITICAL();
}

/***************************************************************************//**
 * @brief
 *   Function to remove a scheduled event
 *
 * @details
 * 	 This routine (atomic) removes the parameter event from event_scheduled
 *
 * @note
 *   This function is called when an interrupt of any type is done being handled
 *
 * @param[in] event
 *   Is the event to be removed
 *
 ******************************************************************************/

void remove_scheduled_event(uint32_t event) {
	CORE_DECLARE_IRQ_STATE;
	CORE_ENTER_CRITICAL();
	event_scheduled &= ~event;
	CORE_EXIT_CRITICAL();
}

/***************************************************************************//**
 * @brief
 *   Function to get the currently scheduled events
 *
 * @details
 * 	 This routine returns the currently scheduled events
 *
 * @note
 *   This function is called at many times to see if there are any interrupts
 *
 ******************************************************************************/

uint32_t get_scheduled_events(void) {
	return event_scheduled;
}

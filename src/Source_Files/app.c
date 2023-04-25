/**
 * @file app.c
 * @author Gerritt Luoma
 * @date February 18th, 2021
 * @brief Contains initialization functions
 *
 */

//***********************************************************************************
// Include files
//***********************************************************************************
#include "app.h"


//***********************************************************************************
// defined files
//***********************************************************************************

//#define BLE_TEST_ENABLED
#define TDD_TEST_ENABLED

//***********************************************************************************
// Static / Private Variables
//***********************************************************************************

static uint32_t i2c_counter;

//***********************************************************************************
// Private functions
//***********************************************************************************

static void app_letimer_pwm_open(float period, float act_period, uint32_t out0_route, uint32_t out1_route);

//***********************************************************************************
// Global functions
//***********************************************************************************

/***************************************************************************//**
 * @brief
 *	Sets up LETIMER0 peripheral
 *
 * @details
 *	First calls CMU open to enable the clock
 *	Calls GPIO open to enable the LEDs
 *	Calls below app_letimer_pwm_open function
 *	Starts the LETimer
 *
 * @note
 *	Called once to set up the peripheral
 *
 ******************************************************************************/

void app_peripheral_setup(void){
	// Configure and open the Clock Management Unit
	cmu_open();

	// Configure and open the gpio pins used for LEDs, I2C, LETIMER, and LEUART
	gpio_open();

	// Configure and open the scheduler
	scheduler_open();

	// Configure and open the sleep routines
	sleep_open();

	// Configure and open the i2c for the si7021
	si7021_i2c_open();

	// Configure and open the i2c for the veml6030
	veml6030_i2c_open();

	// Configure and open the pwm from LETIMER0
	app_letimer_pwm_open(PWM_PER, PWM_ACT_PER, PWM_ROUTE_0, PWM_ROUTE_1);

	// Configure and open the LEUART for BLE
	ble_open(BLE_RX_DONE_CB, BLE_TX_DONE_CB);

	// Block the system EM level
	sleep_block_mode(SYSTEM_BLOCK_EM);

	// Schedule the boot up event
	add_scheduled_event(BOOT_UP_CB);
}

/***************************************************************************//**
 * @brief
 *	Initializes LETIEMER0 for PWM operation
 *
 * @details
 *	Creates a struct with all elements we will need for LETimer PWM operation
 *
 * @note
 *	Called once from the above app_peripheral_setup function
 *
 * @param[in] period
 *	Float for the LETimer period
 *
 * @param[in] act_period
 * 	Float for the LETimer active period
 *
 * @param[in] out0_route
 * 	uint32_t for pin 0 route
 *
 * @param[in] out1_route
 * 	uint32_t for pin 1 route
 *
 ******************************************************************************/
void app_letimer_pwm_open(float period, float act_period, uint32_t out0_route, uint32_t out1_route){
	// Initializing LETIMER0 for PWM operation by creating the
	// letimer_pwm_struct and initializing all of its elements
	APP_LETIMER_PWM_TypeDef letimerstruct;
	letimerstruct.debugRun = false;
	letimerstruct.enable = false;
	letimerstruct.period = period;
	letimerstruct.active_period = act_period;
	letimerstruct.out_pin_route0 = out0_route;
	letimerstruct.out_pin_route1 = out1_route;
	letimerstruct.out_pin_0_en = false;
	letimerstruct.out_pin_1_en = false;

	letimerstruct.comp0_irq_enable = false; // disable comp0 interrupt for now
	letimerstruct.comp0_cb = LETIMER0_COMP0_CB;
	letimerstruct.comp1_irq_enable = false; // disable comp1 interrupt for now
	letimerstruct.comp1_cb = LETIMER0_COMP1_CB;
	letimerstruct.uf_irq_enable = true; 	// enable underflow interrupt to start sensor reads
	letimerstruct.uf_cb = LETIMER0_UF_CB;

	letimer_pwm_open(LETIMER0, &letimerstruct);

}

/***************************************************************************//**
 * @brief
 *	Handles underflow
 *
 * @details
 *	Calls SI7021_read to call i2c_start
 *
 * @note
 *	Called once for each UF interrupt
 *
 *
 ******************************************************************************/

void scheduled_letimer0_uf_cb (void){
	EFM_ASSERT(get_scheduled_events() & LETIMER0_UF_CB);
	remove_scheduled_event(LETIMER0_UF_CB);

	if (i2c_counter == 0) {
		si7021_h_read(SI7021_H_READ_CB);
	}
	if (i2c_counter == 1) {
		si7021_t_read(SI7021_T_READ_CB);
	}
	if (i2c_counter == 2) {
		veml6030_read(VEML6030_READ_CB);
		i2c_counter = -1;
	}
	i2c_counter++;
}

/***************************************************************************//**
 * @brief
 *	Handles COMP0
 *
 * @details
 *	Then removes the scheduled COMP0 event
 *
 * @note
 *	Called once for each COMP0 interrupt
 *
 *
 ******************************************************************************/

void scheduled_letimer0_comp0_cb (void) {
	EFM_ASSERT(get_scheduled_events() & LETIMER0_COMP0_CB);
	remove_scheduled_event(LETIMER0_COMP0_CB);
	EFM_ASSERT(false);
}

/***************************************************************************//**
 * @brief
 *	Handles COMP1
 *
 * @details
 *	Then removes the scheduled COMP1 event
 *
 * @note
 *	Called once for each COMP1 interrupt
 *
 *
 ******************************************************************************/

void scheduled_letimer0_comp1_cb (void){
	EFM_ASSERT(get_scheduled_events() & LETIMER0_COMP1_CB);
	remove_scheduled_event(LETIMER0_COMP1_CB);
	EFM_ASSERT(false);
}

/***************************************************************************//**
 * @brief
 *	Handles si7021_h_read_cb
 *
 * @details
 *	Converts the data read to a humidity value
 *	Also sends it via bluetooth
 *
 * @note
 *	Called every time the I2C0 state machine finishes and there is a value to be converted
 *
 *
 ******************************************************************************/

void humidity_done_cb (void){
	EFM_ASSERT(get_scheduled_events() & SI7021_H_READ_CB);
	remove_scheduled_event(SI7021_H_READ_CB);
	float humidity = si7021_humidity_conversion();
	char str[80];
	sprintf(str, "%4.1f%% humidity\n", humidity);
	ble_write(str);

}

/***************************************************************************//**
 * @brief
 *	Handles si7021_t_read_cb
 *
 * @details
 *	Converts the data read to a temp value
 *	Also sends it via bluetooth
 *
 * @note
 *	Called every time the I2C0 state machine finishes and there is a value to be converted
 *
 *
 ******************************************************************************/

void temp_done_cb (void){
	EFM_ASSERT(get_scheduled_events() & SI7021_T_READ_CB);
	remove_scheduled_event(SI7021_T_READ_CB);
	float temp = si7021_temperature_conversion();
	char str[80];
	sprintf(str, "%4.1f F\n", temp);
	ble_write(str);

}

/***************************************************************************//**
 * @brief
 *	Handles veml6030_read_cb
 *
 * @details
 *	Converts the data read to a light value
 *	Also sends it via bluetooth
 *
 * @note
 *	Called every time the I2C1 state machine finishes and there is a value to be converted
 *
 *
 ******************************************************************************/

void light_done_cb (void){
	EFM_ASSERT(get_scheduled_events() & VEML6030_READ_CB);
	remove_scheduled_event(VEML6030_READ_CB);
	int light = veml6030_conversion();
	char str[80];
	unsigned int ulight = (unsigned int) light;
	sprintf(str, "%3u lux\n", ulight);
	ble_write(str);

}

/***************************************************************************//**
 * @brief
 *	Handles boot_up_cb
 *
 * @details
 *	if BLE TEST is enabled, names the Bluetooth Device
 *	if TDD TEST is enabled, runs the test driven development
 *	Starts VEML6030
 *	Starts LETIMER
 *
 * @note
 *	Called to boot up the machine
 *
 *
 ******************************************************************************/

void scheduled_boot_up_cb(void) {
	EFM_ASSERT(get_scheduled_events() & BOOT_UP_CB);
	remove_scheduled_event(BOOT_UP_CB);
#ifdef BLE_TEST_ENABLED
	EFM_ASSERT(ble_test("BLE_Athena"));
	timer_delay(2000);
#endif
#ifdef TDD_TEST_ENABLED
	tdd_i2c_routine(SI7021_H_READ_CB, SI7021_T_READ_CB);
#endif
	//ble_write("\nHello World\n");
	veml_start_up(VEML6030_READ_CB);
	letimer_start(LETIMER0, true);   // letimer_start will inform the LETIMER0 peripheral to begin counting.
}

/***************************************************************************//**
 * @brief
 *	Handles ble_tx_done_cb
 *
 * @details
 *	Removes BLE TX DONE CB event
 *
 * @note
 *	Called when BLE TX DONE is set
 *
 *
 ******************************************************************************/

void scheduled_ble_tx_done_cb(void) {
	EFM_ASSERT(get_scheduled_events() & BLE_TX_DONE_CB);
	remove_scheduled_event(BLE_TX_DONE_CB);
}


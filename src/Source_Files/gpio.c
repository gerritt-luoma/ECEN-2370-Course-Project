/**
 * @file gpio.c
 * @author Gerritt Luoma
 * @date February 18th, 2021
 * @brief Contains GPIO setup
 *
 */

//***********************************************************************************
// Include files
//***********************************************************************************
#include "gpio.h"


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
 *	Configures LED pins
 *
 * @details
 *	First enables GPIO clock
 *	Then sets drive strength and pin mode for LED0
 *	Does the same for LED1
 *
 * @note
 *	Called once to configure the LEDs
 *
 ******************************************************************************/

void gpio_open(void){

	CMU_ClockEnable(cmuClock_GPIO, true);

	// Configure LED pins
	GPIO_DriveStrengthSet(LED0_PORT, LED0_DRIVE_STRENGTH);
	GPIO_PinModeSet(LED0_PORT, LED0_PIN, LED0_GPIOMODE, LED0_DEFAULT);

	GPIO_DriveStrengthSet(LED1_PORT, LED1_DRIVE_STRENGTH);
	GPIO_PinModeSet(LED1_PORT, LED1_PIN, LED1_GPIOMODE, LED1_DEFAULT);

	//configure SI7021 pins
	GPIO_DriveStrengthSet(SI7021_SENSOR_EN_PORT, SI7021_SENSOR_DRIVE_STRENGTH);
	GPIO_PinModeSet(SI7021_SENSOR_EN_PORT, SI7021_SENSOR_EN_PIN, SI7021_SENSOR_GPIOMODE, true);

	GPIO_PinModeSet(SI7021_SCL_PORT, SI7021_SCL_PIN, SI7021_I2C_GPIOMODE, SENSOR_I2C_SCL);
	GPIO_PinModeSet(SI7021_SDA_PORT, SI7021_SDA_PIN, SI7021_I2C_GPIOMODE, SENSOR_I2C_SDA);

	//configure VEML pins
	GPIO_PinModeSet(VEML6030_SCL_PORT, VEML6030_SCL_PIN, VEML6030_GPIOMODE, SENSOR_I2C_SCL);
	GPIO_PinModeSet(VEML6030_SDA_PORT, VEML6030_SDA_PIN, VEML6030_GPIOMODE, SENSOR_I2C_SDA);

	//configure LEUART pins
	GPIO_DriveStrengthSet(LEUART0_TX_PORT, LEUART0_DRIVE_STRENGTH);
	GPIO_PinModeSet(LEUART0_TX_PORT, LEUART0_TX_PIN, LEUART0_TX_GPIOMODE, LEUART0_TX_DEFAULT);

	GPIO_PinModeSet(LEUART0_RX_PORT, LEUART0_RX_PIN, LEUART0_RX_GPIOMODE, LEUART0_RX_DEFAULT);


}

/**
 * @file SI7021.c
 * @author Gerritt Luoma
 * @date March 2nd, 2021
 * @brief Contains all the SI7021 functions
 *
 */

//***********************************************************************************
// Include files
//***********************************************************************************

#include "SI7021.h"
#include "em_assert.h"
#include "em_core.h"
#include "em_emu.h"
#include "em_i2c.h"


//***********************************************************************************
// Private variables
//***********************************************************************************

uint32_t data;
bool RW = true; //for now - read
uint8_t byte_count = 2; //humidity and temp
uint8_t byte_count_user1 = 1; //user1

//***********************************************************************************
// Functions
//***********************************************************************************



/***************************************************************************//**
 * @brief
 *   Initializes i2c for SI7021 operation
 *
 * @details
 * 	 Creates a struct with all the information needed for i2c SI7021 operation
 *
 * @note
 *   This function is called once in the beginning, in app_peripheral_setup
 *
 ******************************************************************************/

void si7021_i2c_open() {
	timer_delay(80);

	I2C_OPEN_STRUCT i2c_open_s;
	i2c_open_s.freq = SI7021_FREQ;
	i2c_open_s.SCLPEN = SENSOR_I2C_SCL;
	i2c_open_s.SCL_route = SI7021_SCL_ROUTE;
	i2c_open_s.SDAPEN = SENSOR_I2C_SDA;
	i2c_open_s.SDA_route = SI7021_SDA_ROUTE;
	i2c_open_s.clhr = SI7021_CLHR;
	i2c_open_s.master = true;
	i2c_open_s.refFreq = 0;
	i2c_open_s.enable = true;

	i2c_open(SI7021_I2C, &i2c_open_s);
}

/***************************************************************************//**
 * @brief
 *   Starts the i2c state machine
 *
 * @details
 * 	 Calls i2c_start with all information needed to initialize the state machine
 *
 * @note
 *   This function is called every time there is an LETIMER underflow interrupt
 *
 * @param[in] SI7021_h_read_cb
 *   Callback for when the SI7021 humidity read operation is completed
 ******************************************************************************/

void si7021_h_read(uint32_t SI7021_h_read_cb) {
	i2c_start(SI7021_I2C, SI7021_SLAVE_ADDRESS, SI7021_COMMAND, RW, &data, byte_count, SI7021_h_read_cb); //start i2c
	timer_delay(15);
}

/***************************************************************************//**
 * @brief
 *   Starts the i2c state machine
 *
 * @details
 * 	 Calls i2c_start with all information needed to initialize the state machine
 *
 * @note
 *   This function is called every time there is an LETIMER underflow interrupt
 *
 * @param[in] SI7021_t_read_cb
 *   Callback for when the SI7021 temp read operation is completed
 ******************************************************************************/

void si7021_t_read(uint32_t SI7021_t_read_cb) {
	i2c_start(SI7021_I2C, SI7021_SLAVE_ADDRESS, SI7021_TEMP_COMMAND, RW, &data, byte_count, SI7021_t_read_cb); //start i2c
	timer_delay(15);
}

/***************************************************************************//**
 * @brief
 *   Returns the humidity value
 *
 * @details
 * 	 Converts the value read from the SI7021 to a humidity value
 *
 * @note
 *   This function is called every time there is an SI7021 humidity read callback interrupt (state machine done)
 *
 ******************************************************************************/

float si7021_humidity_conversion() {
	uint32_t result = data;
	return ((125.0*result)/65536)-6;
}

/***************************************************************************//**
 * @brief
 *   Returns the temperature value
 *
 * @details
 * 	 Converts the value read from the SI7021 to a temperature value
 *
 * @note
 *   This function is called every time there is an SI7021 temperature read callback interrupt (state machine done)
 *
 ******************************************************************************/

float si7021_temperature_conversion() {
	uint32_t result = data;
	float celcius = ((175.72*result)/65536) - 46.85; //c
	return celcius * 1.8 + 32; //f
}

/***************************************************************************//**
 * @brief
 *   Test driven development function to test granular pieces of code functionality
 *
 * @details
 * 	 Tests:
 * 	 test read of user register 1 by reading from SI7021 user register 1 and checking for the default value
 * 	 test write to user register 1 by writing a data value and checking whether it was written
 * 	 test read again from user register 1 and check if the data value is what you just wrote
 * 	 test a 2 byte access: humidity reading
 * 	 test a 2 byte access: temperature reading
 * 	 No test coverage escapes
 *
 * @note
 *   This function is called from the Boot up callback
 *
 * @param[in] si7021_read_cb
 *   Callback for humidity done
 *
 * @param[in] si7021_t_read_cb
 *   Callback for temperature done
 *
 ******************************************************************************/

bool tdd_i2c_routine(uint32_t si7021_read_cb, uint32_t si7021_t_read_cb) {
	//test read of user register 1
	RW = true; //read
	i2c_start(SI7021_I2C, SI7021_SLAVE_ADDRESS, SI7021_READ_COMMAND, RW, &data, byte_count_user1, si7021_read_cb);
	while(check_busy_1(SI7021_I2C));
	EFM_ASSERT(data == RESET_VALUE || data == PREVIOUS_USER1_VALUE); //default initial setting user register 1

	//test write to user register 1
	//RESOLUTION_CONFIG = 0x01 for 8 bit RH, 12 bit temp resolution
	data = RESOLUTION_CONFIG;
	RW = false; //write
	i2c_start(SI7021_I2C, SI7021_SLAVE_ADDRESS, SI7021_WRITE_COMMAND, RW, &data, byte_count_user1, si7021_read_cb);
	while(check_busy_1(SI7021_I2C));
	timer_delay(15);
	EFM_ASSERT(data == RESOLUTION_CONFIG); //01

	//read register back to make sure write actually occurred
	RW = true; //read
	i2c_start(SI7021_I2C, SI7021_SLAVE_ADDRESS, SI7021_READ_COMMAND, RW, &data, byte_count_user1, si7021_read_cb);
	while(check_busy_1(SI7021_I2C));
	EFM_ASSERT(data == RESOLUTION_FOR_8_12); //3B

	//test a 2 byte access of the humidity
	i2c_start(SI7021_I2C, SI7021_SLAVE_ADDRESS, SI7021_COMMAND, RW, &data, byte_count, si7021_read_cb);
	while(check_busy_1(SI7021_I2C));
	int humidity = si7021_humidity_conversion();
	EFM_ASSERT((humidity > 10) && (humidity < 50));

	//test a 2 byte access to the temp
	i2c_start(SI7021_I2C, SI7021_SLAVE_ADDRESS, SI7021_TEMP_COMMAND, RW, &data, byte_count, si7021_t_read_cb);
	while(check_busy_1(SI7021_I2C));
	int temp = si7021_temperature_conversion();
	EFM_ASSERT((temp > 40) && (temp < 80));

	return true;
}


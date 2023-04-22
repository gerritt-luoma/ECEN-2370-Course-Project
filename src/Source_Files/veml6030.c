/**
 * @file veml6030.c
 * @author Gerritt Luoma
 * @date April 21st, 2021
 * @brief Contains all the veml6030 functions
 *
 */

//***********************************************************************************
// Include files
//***********************************************************************************

#include "veml6030.h"
#include "em_assert.h"
#include "em_core.h"
#include "em_emu.h"
#include "em_i2c.h"


//***********************************************************************************
// Private variables
//***********************************************************************************

uint32_t data;
bool veml_RW = true; //for now - read
uint8_t veml_byte_count = 2; //2 byte

//***********************************************************************************
// Functions
//***********************************************************************************



/***************************************************************************//**
 * @brief
 *   Initializes i2c for VEML6030 operation
 *
 * @details
 * 	 Creates a struct with all the information needed for i2c VEML6030 operation
 *
 * @note
 *   This function is called once in the beginning, in app_peripheral_setup
 *
 ******************************************************************************/

void veml6030_i2c_open() {
	timer_delay(80);

	I2C_OPEN_STRUCT i2c_open_s;
	i2c_open_s.freq = VEML6030_FREQ;
	i2c_open_s.SCLPEN = SENSOR_I2C_SCL;
	i2c_open_s.SCL_route = VEML6030_SCL_ROUTE;
	i2c_open_s.SDAPEN = SENSOR_I2C_SDA;
	i2c_open_s.SDA_route = VEML6030_SDA_ROUTE;
	i2c_open_s.clhr = VEML6030_CLHR;
	i2c_open_s.master = true;
	i2c_open_s.refFreq = 0;
	i2c_open_s.enable = true;

	i2c_open(VEML6030_I2C, &i2c_open_s);
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
 * @param[in] VEML6030_read_cb
 *   Callback for when the VEML6030 read operation is completed
 ******************************************************************************/

void veml6030_read(uint32_t VEML6030_read_cb) {
	i2c_start(VEML6030_I2C, VEML6030_ADDRESS, VEML6030_COMMAND, veml_RW, &data, veml_byte_count, VEML6030_read_cb); //start i2c
	timer_delay(15);
}

/***************************************************************************//**
 * @brief
 *   Returns the lux value
 *
 * @details
 * 	 Converts the value read from the VEML6030 to a light value
 *
 * @note
 *   This function is called every time there is a VEML6030 read callback interrupt (state machine done)
 *
 ******************************************************************************/

//Light level [lx] is (ALS OUTPUT DATA [dec.] / ALS Gain x responsivity). Please study also the application note
//for cofiguration 0x00, gain x 1, intergration time 100 ms, multiply data by 0.0576

float veml6030_conversion() {
	uint32_t result = data;
	return 0.0576*result; //lux
}

/***************************************************************************//**
 * @brief
 *   Starts the VEML6030
 *
 * @details
 * 	 Does a 2 byte write to the VEML6030 with the start up command, 0x00, to prepare for later reading
 *
 * @note
 *   This function is called from boot up cb function in app.c
 *
 * @param[in] VEML6030_read_cb
 *   Callback for when the VEML6030 read operation is completed
 ******************************************************************************/

bool veml_start_up(uint32_t veml6030_read_cb) {
	//2 byte write to the veml
	//START_UP_COMMAND = 0x0
	data = START_UP_COMMAND;
	veml_RW = false; //write
	i2c_start(VEML6030_I2C, VEML6030_ADDRESS, START_UP_COMMAND, veml_RW, &data, veml_byte_count, veml6030_read_cb);
	while(check_busy_0(VEML6030_I2C));
	timer_delay(15);
	EFM_ASSERT(data == START_UP_COMMAND); //01
	return true;
}


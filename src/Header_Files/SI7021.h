//***********************************************************************************
// Include files
//***********************************************************************************

#ifndef SI7021_HG
#define SI7021_HG

/* System include statements */
#include <stdint.h>
#include <stdbool.h>

/* Silicon Labs include statements */
#include "em_assert.h"
#include "em_int.h"
#include "em_i2c.h"


/* The developer's include statements */
#include "i2c.h"
#include "brd_config.h"
#include "HW_delay.h"

//***********************************************************************************
// defined files
//***********************************************************************************

#define SI7021_FREQ 			I2C_FREQ_FAST_MAX
#define SI7021_CLHR 			i2cClockHLRAsymetric
#define SI7021_I2C				I2C1
#define SI7021_COMMAND			0xF5 //humidity
#define SI7021_SLAVE_ADDRESS	0x40

#define SI7021_TEMP_COMMAND		0xF3
#define SI7021_READ_COMMAND		0xE7
#define SI7021_WRITE_COMMAND	0xE6

#define RESET_VALUE 			0x3A
#define RESOLUTION_CONFIG		0x01
#define RESOLUTION_FOR_8_12		0x3B
#define PREVIOUS_USER1_VALUE	0x3B


//***********************************************************************************
// global variables
//***********************************************************************************

//***********************************************************************************
// function prototypes
//***********************************************************************************

void si7021_i2c_open();
void si7021_h_read(uint32_t SI7021_H_READ_CB);
void si7021_t_read(uint32_t SI7021_T_READ_CB);
float si7021_humidity_conversion();
float si7021_temperature_conversion();
bool tdd_i2c_routine(uint32_t si7021_read_cb, uint32_t si7021_t_read_cb);

#endif

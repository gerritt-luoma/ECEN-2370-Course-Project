//***********************************************************************************
// Include files
//***********************************************************************************

#ifndef VEML6030_HG
#define VEML6030_HG

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

#define VEML6030_FREQ 			I2C_FREQ_FAST_MAX //idk
#define VEML6030_CLHR 			i2cClockHLRAsymetric //idk
#define VEML6030_I2C			I2C0
//0 first - do a 2 byte write at boot up
#define START_UP_COMMAND		0x00 //initial write
#define VEML6030_ADDRESS 		0x48
#define VEML6030_COMMAND		0x04 //read ALS

//***********************************************************************************
// global variables
//***********************************************************************************

//***********************************************************************************
// function prototypes
//***********************************************************************************

void veml6030_i2c_open();
void veml6030_read(uint32_t VEML6030_READ_CB);
float veml6030_conversion();
bool veml_start_up(uint32_t veml6030_read_cb);

#endif

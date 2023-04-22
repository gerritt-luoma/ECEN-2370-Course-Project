//***********************************************************************************
// Include files
//***********************************************************************************

#ifndef I2C_HG
#define I2C_HG

/* System include statements */
#include <stdint.h>
#include <stdbool.h>

/* Silicon Labs include statements */
#include "em_assert.h"
#include "em_int.h"
#include "em_i2c.h"

/* The developer's include statements */
#include "sleep_routines.h"
#include "scheduler.h"
#include "HW_delay.h"

//***********************************************************************************
// defined files
//***********************************************************************************

#define I2C_EM_BLOCK EM2
#define I2C_READ 	 1
#define I2C_WRITE	 0

//***********************************************************************************
// global variables
//***********************************************************************************

typedef struct {
	bool					enable;		//Enable I2C peripheral when initialization completed.
	bool					master;		//Set to master (true) or slave (false) mode.
	uint32_t				refFreq;	//I2C reference clock assumed when configuring bus frequency setup.
	uint32_t				freq;		//(Max) I2C bus frequency to use.
	I2C_ClockHLR_TypeDef	clhr;		//Clock low/high ratio control.

	uint32_t				SDA_route;	// SDA route
	uint32_t				SCL_route;	// SCL route
	bool					SDAPEN;		// SDA pin enable
	bool					SCLPEN;		// SCL pin enable
} I2C_OPEN_STRUCT ;

typedef struct {
	uint32_t				state;		// current state of state machine

	I2C_TypeDef				*i2c_def;	// i2c0 or i2c1
	uint32_t				slave_address;// slave device address
	uint32_t				slave_reg;	// slave register address being requested
	bool					w_r;		// write or read
	uint32_t				*w_r_store;	// pointer of where to store read result or where to get write data
	bool					i2c_busy;
	uint32_t				bytes_count;
	uint32_t 				callback;

} I2C_STATE_MACHINE ;

enum i2c_defined_states {
	StartCommand,	//0
	ReadCommand,	//1
	WriteCommand, 	//2
	WaitRead,		//3
	EndSensing,		//4
	Stop			//5
} ;

//***********************************************************************************
// function prototypes
//***********************************************************************************

void i2c_open(I2C_TypeDef *i2c_def, I2C_OPEN_STRUCT *i2c_setup);
void I2C0_IRQHandler(void);
void I2C1_IRQHandler(void);
void i2c_start(I2C_TypeDef *i2c, uint32_t address, uint32_t reg, bool RW, uint32_t *loc, uint8_t byte_count, uint32_t SI7021_READ_CB);
bool check_busy_0(I2C_TypeDef *i2c);
bool check_busy_1(I2C_TypeDef *i2c);

#endif

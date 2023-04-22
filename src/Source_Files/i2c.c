/**
 * @file i2c.c
 * @author Gerritt Luoma
 * @date March 2nd, 2021
 * @brief Contains all the i2c functions
 *
 */

//***********************************************************************************
// Include files
//***********************************************************************************

#include "i2c.h"
#include "em_assert.h"
#include "em_core.h"
#include "em_emu.h"
#include "em_i2c.h"
#include "em_cmu.h"

//***********************************************************************************
// Private variables
//***********************************************************************************



static I2C_STATE_MACHINE 	 i2c_state; //for light sensor = I2C0
static I2C_STATE_MACHINE	 i2c_state_1; //for SI7021 = I2C1
//static uint32_t			 SI7021_READ_CB;


//***********************************************************************************
// Functions
//***********************************************************************************

//private
static void i2c_ack(I2C_STATE_MACHINE *i2c_state);
static void i2c_nack(I2C_STATE_MACHINE *i2c_state);
static void i2c_rxdatav(I2C_STATE_MACHINE *i2c_state);
static void i2c_mstop(I2C_STATE_MACHINE *i2c_state);
void i2c_bus_reset(I2C_TypeDef *i2c_def);

/***************************************************************************//**
 * @brief
 *   Function to do an i2c bus reset
 *
 * @details
 * 	 This routine aborts i2c operation and resets it for new operation
 *
 * @note
 *   This function is called once in the beginning, from i2c_open
 *
 * @param[in] *i2c_def
 *   Pointer to the base peripheral address of the i2c peripheral being reset
 *
 ******************************************************************************/

void i2c_bus_reset(I2C_TypeDef *i2c_def) {
	i2c_def->CMD = I2C_CMD_ABORT; //abort i2c peripheral operation
	uint32_t ien = i2c_def->IEN & i2c_def->IF;
	i2c_def->IEN = 0;
	i2c_def->IFC = i2c_def->IF;
	i2c_def->CMD = I2C_CMD_CLEARTX;
	i2c_def->CMD = I2C_CMD_START | I2C_CMD_STOP;
	while(!(i2c_def->IF & I2C_IF_MSTOP)); //ensure that reset occurred
	i2c_def->IFC = i2c_def->IF;
	i2c_def->IEN |= ien;
	i2c_def->IEN |= I2C_IF_MSTOP;
	i2c_def->IEN |= I2C_IEN_MSTOP;

	i2c_def->CMD = I2C_CMD_ABORT;

}

/***************************************************************************//**
 * @brief
 *   Function to open and set an i2c peripheral
 *
 * @details
 * 	 This routine opens one of the i2c peripherals be enabling the correct clock,
 * 	 verifying correct clock operation, initializing the i2c, calling a bus reset
 * 	 and enabling interrupts
 *
 * @note
 *   This function is called once in the beginning, from SI7021_i2c_open
 *
 * @param[in] *i2c_def
 *   Pointer to the base peripheral address of the i2c peripheral being opened
 *
 * @param[in] *i2c_setup
 *   Pointer to the struct used to set up this i2c
 *
 ******************************************************************************/

void i2c_open(I2C_TypeDef *i2c_def, I2C_OPEN_STRUCT *i2c_setup) {
	//enable correct clock
	if(i2c_def == I2C0) {
		CMU_ClockEnable(cmuClock_I2C0, true);
	}
	else if(i2c_def == I2C1) {
		CMU_ClockEnable(cmuClock_I2C1, true);
	}
	// Test if clock is enabled and we can read/set/clear interrupt flags
	if ((i2c_def->IF & 0x01) == 0) {
		i2c_def->IFS = 0x01;
		EFM_ASSERT(i2c_def->IF & 0x01);
		i2c_def->IFC = 0x01;
	} else {
		i2c_def->IFC = 0x01;
	}
	EFM_ASSERT(!(i2c_def->IF & 0x01));

	I2C_Init_TypeDef i2c_init;

	i2c_init.enable = i2c_setup->enable;
	i2c_init.master = i2c_setup->master;
	i2c_init.refFreq = i2c_setup->refFreq;
	i2c_init.freq = i2c_setup->freq;
	i2c_init.clhr = i2c_setup->clhr;

	I2C_Init(i2c_def, &i2c_init);

	// Route the I2C to the correct pins and enable pins
	i2c_def->ROUTELOC0 = i2c_setup->SCL_route | i2c_setup->SDA_route;
	i2c_def->ROUTEPEN = (I2C_ROUTEPEN_SCLPEN*i2c_setup->SCLPEN | I2C_ROUTEPEN_SDAPEN*i2c_setup->SDAPEN);

	// Reset the bus
	i2c_bus_reset(i2c_def);

	// clear and enable ACK interrupts
	i2c_def->IFC = I2C_IF_ACK;
	i2c_def->IEN |= I2C_IF_ACK;

	// clear and enable NACK interrupts
	i2c_def->IFC = I2C_IF_NACK;
	i2c_def->IEN |= I2C_IF_NACK;

	// clear and enable STOP interrupts
	i2c_def->IFC = I2C_IF_MSTOP;
	i2c_def->IEN |= I2C_IF_MSTOP;

	// clear and enable rxdata interrupt
	i2c_def->IFC = I2C_IF_RXDATAV;
	i2c_def->IEN |= I2C_IF_RXDATAV;


	// enable interrupts for specific i2c
	if(i2c_def == I2C0) {
		NVIC_EnableIRQ(I2C0_IRQn);
	}
	if(i2c_def == I2C1) {
		NVIC_EnableIRQ(I2C1_IRQn);
	}

}

/***************************************************************************//**
 * @brief
 *   Function to start i2c
 *
 * @details
 * 	 This routine sets up the i2c state machine struct to initialize the state machine
 * 	 depending on whether we are using I2C0 or I2C1
 *
 * @note
 *   This function is called from SI7021_read at every underflow interrupt
 *
 * @param[in] *i2c
 *   Pointer to the base peripheral address of the i2c peripheral being opened
 *
 * @param[in] address
 *   Is the address of the SI7021
 *
 * @param[in] reg
 *   Is the register of the SI7021 being used
 *
 * @param[in] RW
 *   Is the boolean to tell whether we are reading or writing
 *
 * @param[in] *loc
 *   Location where the data to read or dara to be written is/will be stored
 *
 * @param[in] byte_count
 *   Is the number of bytes to be read/written
 *
 * @param[in] SI7021_READ_CB
 *   Is the callback once reading is completed
 *
 ******************************************************************************/

void i2c_start(I2C_TypeDef *i2c, uint32_t address, uint32_t reg, bool RW, uint32_t *loc, uint8_t byte_count, uint32_t CallBack) {
	// triggers if i2c peripheral has not finished pervious i2c operation
	EFM_ASSERT((i2c->STATE & _I2C_STATE_STATE_MASK) == I2C_STATE_STATE_IDLE); // X = the I2C peripheral #
	sleep_block_mode(I2C_EM_BLOCK);

	if(i2c == I2C1) { //si7021
		i2c_state_1.i2c_def = i2c;
		i2c_state_1.slave_address = address;
		i2c_state_1.slave_reg = reg;
		i2c_state_1.w_r_store = loc;
		i2c_state_1.w_r = RW;
		i2c_state_1.bytes_count = byte_count;

		i2c_state_1.state = StartCommand;
		i2c_state_1.i2c_def->CMD = I2C_CMD_START;
		i2c_state_1.i2c_def->TXDATA = (i2c_state_1.slave_address << 1) | I2C_WRITE;
		i2c_state_1.i2c_busy = true;
		i2c_state_1.callback = CallBack;
	}
	else if(i2c == I2C0) { //veml6030
		i2c_state.i2c_def = i2c;
		i2c_state.slave_address = address;
		i2c_state.slave_reg = reg;
		i2c_state.w_r_store = loc;
		i2c_state.w_r = RW;
		i2c_state.bytes_count = byte_count;

		i2c_state.state = StartCommand;
		i2c_state.i2c_def->CMD = I2C_CMD_START;
		i2c_state.i2c_def->TXDATA = (i2c_state.slave_address << 1) | I2C_WRITE;
		i2c_state.i2c_busy = true;
		i2c_state.callback = CallBack;
	}


	//for light sensing: if i2c = i2c1, do light sensing, if 0, ^

}

/***************************************************************************//**
 * @brief
 *   Function to handle interrupts for I2C0
 *
 * @details
 * 	 This routine checks whether there is an interrupt, and whether it is ACK, NACK, RXDATAV, MSTOP
 * 	 Then calls the function for the specific interrupt
 *
 * @note
 *   This function is called to any time there is an interrupt of any type
 *
 ******************************************************************************/

void I2C0_IRQHandler(void) {
	 uint32_t int_flag; // store source interrupts

	 //AND the interrupt source (IF), with the interrupt enable register (IEN)
	 // your interrupt source variable will only contain interrupts of interest
	 int_flag = I2C0->IF & I2C0->IEN;

	 //clear interrupt flag register
	 I2C0->IFC = int_flag;

	 if (int_flag & I2C_IF_ACK){
		 i2c_ack(&i2c_state);
	 }
	 if (int_flag & I2C_IF_NACK){
		 i2c_nack(&i2c_state);
	 }
	 if (int_flag & I2C_IF_RXDATAV){
		 i2c_rxdatav(&i2c_state);
	 }
	 if (int_flag & I2C_IF_MSTOP){
	 	 i2c_mstop(&i2c_state);
	 }
}

/***************************************************************************//**
 * @brief
 *   Function to handle interrupts for I2C1
 *
 * @details
 * 	 This routine checks whether there is an interrupt, and whether it is ACK, NACK, RXDATAV, MSTOP
 * 	 Then calls the function for the specific interrupt
 *
 * @note
 *   This function is called to any time there is an interrupt of any type
 *
 ******************************************************************************/

void I2C1_IRQHandler(void) {
	 uint32_t int_flag; // store source interrupts

	 //AND the interrupt source (IF), with the interrupt enable register (IEN)
	 // your interrupt source variable will only contain interrupts of interest
	 int_flag = I2C1->IF & I2C1->IEN;

	 //clear interrupt flag register
	 I2C1->IFC = int_flag;

	 if (int_flag & I2C_IF_ACK){
		 i2c_ack(&i2c_state_1);
	 }
	 if (int_flag & I2C_IF_NACK){
		 i2c_nack(&i2c_state_1);
	 }
	 if (int_flag & I2C_IF_RXDATAV){
		 i2c_rxdatav(&i2c_state_1);
	 }
	 if (int_flag & I2C_IF_MSTOP){
	 	 i2c_mstop(&i2c_state_1);
	 }
}

/***************************************************************************//**
 * @brief
 *   Function that handles the ACK interrupt
 *
 * @details
 * 	 This routine completes different actions depending on the state of the state
 * 	 machine when the ACK interrupt is received
 *
 * @note
 *   This function is called when an ACK interrupt is received
 *
 ******************************************************************************/

static void i2c_ack (I2C_STATE_MACHINE *i2c_state){
	switch(i2c_state->state) {
		case StartCommand: {
			if (i2c_state->w_r == I2C_READ) {
				i2c_state->state = ReadCommand;
				i2c_state->i2c_def->TXDATA = i2c_state->slave_reg;
			}
			else { //write
				i2c_state->state = WriteCommand;
				i2c_state->i2c_def->TXDATA = i2c_state->slave_reg;
			}
			break;
		}
		case ReadCommand: {
			i2c_state->state = WaitRead;
			i2c_state->i2c_def->CMD = I2C_CMD_START;
			i2c_state->i2c_def->TXDATA = (i2c_state->slave_address << 1) | I2C_READ;
			break;
		}
		case WriteCommand: {
			i2c_state->state = EndSensing;
			//i2c_state->i2c_def->CMD = I2C_CMD_START;
			i2c_state->i2c_def->TXDATA = *(i2c_state->w_r_store);
			timer_delay(15);
			break;
		}
		case WaitRead: {
			i2c_state->state = EndSensing; //if acknowledged exit loop
			break;
		}
		case EndSensing: {
			if (i2c_state->w_r == I2C_WRITE) {
				i2c_state->state = Stop;
				i2c_state->i2c_def->CMD = I2C_CMD_STOP;
			}
			else {
				EFM_ASSERT(false);
			}
			break;
		}
		case Stop: {
			EFM_ASSERT(false);
			break;
		}
		default: {
			EFM_ASSERT(false);
			break;
		}
	}
}

/***************************************************************************//**
 * @brief
 *   Function that handles the NACK interrupt
 *
 * @details
 * 	 This routine completes different actions depending on the state of the state
 * 	 machine when the NACK interrupt is received
 *
 * @note
 *   This function is called when an NACK interrupt is received
 *
 ******************************************************************************/

static void i2c_nack (I2C_STATE_MACHINE *i2c_state){
	switch(i2c_state->state) {
		case StartCommand: {
			EFM_ASSERT(false);
			break;
		}
		case ReadCommand: {
			EFM_ASSERT(false);
			break;
		}
		case WriteCommand: {
			EFM_ASSERT(false);
			break;
		}
		case WaitRead: {
			i2c_state->state = WaitRead; //loop
			i2c_state->i2c_def->CMD = I2C_CMD_START;
			i2c_state->i2c_def->TXDATA = (i2c_state->slave_address << 1) | I2C_READ;
			break;
		}
		case EndSensing: {
			EFM_ASSERT(false);
			break;
		}
		case Stop: {
			EFM_ASSERT(false);
			break;
		}
		default: {
			EFM_ASSERT(false);
			break;
		}
	}
}

/***************************************************************************//**
 * @brief
 *   Function that handles the RXDATAV interrupt
 *
 * @details
 * 	 This routine completes different actions depending on the state of the state
 * 	 machine when the RXDATAV interrupt is received
 *
 * @note
 *   This function is called when an RXDATAV interrupt is received
 *
 ******************************************************************************/

static void i2c_rxdatav (I2C_STATE_MACHINE *i2c_state){
	switch(i2c_state->state) {
		case StartCommand: {
			EFM_ASSERT(false);
			break;
		}
		case ReadCommand: {
			EFM_ASSERT(false);
			break;
		}
		case WriteCommand: {
			EFM_ASSERT(false);
			break;
		}
		case WaitRead: {
			EFM_ASSERT(false);
			break;
		}
		case EndSensing: {
			i2c_state->bytes_count -= 1;
			if(i2c_state->bytes_count > 0){
				*i2c_state->w_r_store = (i2c_state->i2c_def->RXDATA) << (8 * i2c_state->bytes_count);
				i2c_state->i2c_def->CMD = I2C_CMD_ACK;
			}
			else{
				*i2c_state->w_r_store |= i2c_state->i2c_def->RXDATA;
				i2c_state->i2c_def->CMD = I2C_CMD_NACK;
				i2c_state->i2c_def->CMD = I2C_CMD_STOP;
				i2c_state->state = Stop;
				//i2c_state->i2c_def->IFS &= I2C_IFS_MSTOP;
			}
			break;
		}
		case Stop: {
			EFM_ASSERT(false);
			break;
		}
		default: {
			EFM_ASSERT(false);
			break;
		}
	}
}

/***************************************************************************//**
 * @brief
 *   Function that handles the MSTOP interrupt
 *
 * @details
 * 	 This routine completes different actions depending on the state of the state
 * 	 machine when the MSTOP interrupt is received
 *
 * @note
 *   This function is called when an MSTOP interrupt is received
 *
 ******************************************************************************/

static void i2c_mstop (I2C_STATE_MACHINE *i2c_state){
	switch(i2c_state->state) {
		case StartCommand: {
			EFM_ASSERT(false);
			break;
		}
		case ReadCommand: {
			EFM_ASSERT(false);
			break;
		}
		case WriteCommand: {
			EFM_ASSERT(false);
			break;
		}
		case WaitRead: {
			EFM_ASSERT(false);
			break;
		}
		case EndSensing: {
			EFM_ASSERT(false);
			break;
		}
		case Stop: {
			sleep_unblock_mode(I2C_EM_BLOCK);
			add_scheduled_event(i2c_state->callback);
			i2c_state->state = StartCommand;
			i2c_state->i2c_busy = false;
			break;
		}
		default: {
			EFM_ASSERT(false);
			break;
		}
	}
}

/***************************************************************************//**
 * @brief
 *   Function that checks whether I2C0 is busy
 *
 * @details
 * 	 This routine returns false if i2c is not busy, true if busy
 *
 * @note
 *   This function is called when needing to wait for i2c0 operation to be done
 *
 ******************************************************************************/

bool check_busy_0(I2C_TypeDef *i2c) {
	if (i2c_state.i2c_def == i2c){
		return i2c_state.i2c_busy;
	}
	return true;
}

/***************************************************************************//**
 * @brief
 *   Function that checks whether I2C1 is busy
 *
 * @details
 * 	 This routine returns false if i2c is not busy, true if busy
 *
 * @note
 *   This function is called when needing to wait for i2c1 operation to be done
 *
 ******************************************************************************/

bool check_busy_1(I2C_TypeDef *i2c) {
	if (i2c_state_1.i2c_def == i2c){
		return i2c_state_1.i2c_busy;
	}
	return true;
}

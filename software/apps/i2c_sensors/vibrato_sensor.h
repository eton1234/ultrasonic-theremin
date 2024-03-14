// LSM303AGR accelerometer and magnetometer

#pragma once

#include "nrf_twi_mngr.h"

// DeviceADDR for the STHS3... presence/temperature_shock sensor
static const uint8_t PRES_ADDR = 0x5A;
static const uint8_t JOYSTICK_ADDR  = 0x20;

//presence sensor register addresses(subaddresses)
typedef enum {
  WHO_AM_I = 0x0F,
  CTRL1 = 0x20,
  CTRL2 = 0x21,
  CTRL3 = 0x22,
  STATUS = 0x23,
  FUNC_STATUS = 0x25,
} pres_reg_t; 

typedef enum {
  PRESENCE_THS = 0x20,
  MOTION_THS = 0x22,
  TAMB_SHOCK_THS = 0x24,
  HYST_MOTION = 0x26,
  HYST_PRESENCE = 0x27,
  ALGO_CONFIG = 0x28,
  RESET_ALGO = 0x2A,
} config_emb_func_reg_t; 

enum joystickRegisters {
  JOYSTICK_ID = 0x00,
  JOYSTICK_VERSION1, // 0x01
  JOYSTICK_VERSION2, // 0x02
  JOYSTICK_X_MSB,    // 0x03
  JOYSTICK_X_LSB,    // 0x04
  JOYSTICK_Y_MSB,    // 0x05
  JOYSTICK_Y_LSB,    // 0x06
  JOYSTICK_BUTTON,   // 0x07
  JOYSTICK_STATUS,   // 0x08 //1 - button clicked
  JOYSTICK_I2C_LOCK,         // 0x09
  JOYSTICK_CHANGE_ADDRESS    // 0x0A
};


// Initialize and configure the LSM303AGR accelerometer/magnetometer
void pres_init(const nrf_twi_mngr_t* i2c);
static uint8_t i2c_reg_read(uint8_t i2c_addr, uint8_t reg_addr);
static void i2c_reg_write(uint8_t i2c_addr, uint8_t reg_addr, uint8_t data);
bool read_temp_shock(void);
float get_vertical(void);
void joy_init(const nrf_twi_mngr_t* i2c);
// LSM303AGR driver for Microbit_v2
//
// Initializes sensor and communicates over I2C
// Capable of reading temperature, acceleration, and magnetic field strength

#include <stdbool.h>
#include <stdint.h>
#include <math.h>


#include "vibrato_sensor.h"
#include "nrf_delay.h"

// Pointer to an initialized I2C instance to use for transactions
static const nrf_twi_mngr_t* i2c_manager = NULL;

// Helper function to perform a 1-byte I2C read of a given register
//
// i2c_addr - address of the device to read from
// reg_addr - address of the register within the device to read
//
// returns 8-bit read value
static uint8_t i2c_reg_read(uint8_t i2c_addr, uint8_t reg_addr) {
  uint8_t rx_buf;
  printf("i2c_addr %u\n", i2c_addr);
  printf("i2c_addr adjusted %u\n", (i2c_addr << 1) | 0);
  printf("reg addr: %u\n", reg_addr); 
  nrf_twi_mngr_transfer_t write = NRF_TWI_MNGR_WRITE(i2c_addr, &reg_addr, 1, NRF_TWI_MNGR_NO_STOP); 
  nrf_twi_mngr_transfer_t read= NRF_TWI_MNGR_READ(i2c_addr, &rx_buf, 1, 0);
  nrf_twi_mngr_transfer_t const read_transfer[] = {
    write,
    read
  };
  ret_code_t result = nrf_twi_mngr_perform(i2c_manager, NULL, read_transfer, 2, NULL);
    printf("transaction successful\n");

  if (result != NRF_SUCCESS) {
    // Likely error codes:
    //  NRF_ERROR_INTERNAL            (0x0003) - something is wrong with the driver itself
    //  NRF_ERROR_INVALID_ADDR        (0x0010) - buffer passed was in Flash instead of RAM
    //  NRF_ERROR_BUSY                (0x0011) - driver was busy with another transfer still
    //  NRF_ERROR_DRV_TWI_ERR_OVERRUN (0x8200) - data was overwritten during the transaction
    //  NRF_ERROR_DRV_TWI_ERR_ANACK   (0x8201) - i2c device did not acknowledge its address
    //  NRF_ERROR_DRV_TWI_ERR_DNACK   (0x8202) - i2c device did not acknowledge a data byte
    printf("I2C transaction failed! Error: %lX\n", result);
  }
  printf("transaction successful\n");
  return rx_buf;
}
// Helper function to perform a 1-byte I2C write of a given register
//
// i2c_addr - address of the device to write to
// reg_addr - address of the register within the device to write
static void i2c_reg_write(uint8_t i2c_addr, uint8_t reg_addr, uint8_t data) {
  //TODO: implement me
  //Note: there should only be a single two-byte transfer to be performed
  uint8_t arr [2] = {reg_addr, data}; 
  nrf_twi_mngr_transfer_t first = NRF_TWI_MNGR_WRITE(i2c_addr, arr, 2, 0); 
  nrf_twi_mngr_transfer_t const write_transfer[] = {first}; 
  nrf_twi_mngr_perform(i2c_manager, NULL, write_transfer, 1, NULL);
}

// Initialize and configure the LSM303AGR accelerometer/magnetometer
//
// i2c - pointer to already initialized and enabled twim instance
void pres_init(const nrf_twi_mngr_t* i2c) {
  printf("initializing the presence sensor\n");
  i2c_manager = i2c;
  uint8_t who_am_i = i2c_reg_read(PRES_ADDR,WHO_AM_I);
  //checking to see if properly connected to the presence sensor
  if (who_am_i != 0xD3) {
    printf("Error: WHO_AM_I returned 0x%02X\n", who_am_i);
  } else {
    printf("connected to the presence sensor\n");
  }
  printf("ctrl1 %u\n", i2c_reg_read(PRES_ADDR, CTRL1));
  printf("ctrl2 %u\n", i2c_reg_read(PRES_ADDR, CTRL2));
  i2c_reg_write(PRES_ADDR, CTRL1, 0b10011);
  //   //regbooting, config access to 1
  i2c_reg_write(PRES_ADDR, CTRL2, 0b00010000);
  //refer to data sheet. setting the BDU(data update feature) to 1 and ODR set to 2hz
  //ctrl has presence flag, motion flag, temperature flag
}
void joy_init(const nrf_twi_mngr_t* i2c) {
  printf("initializing the joystick sensor\n");
  i2c_manager = i2c;
  // uint8_t who_am_i = i2c_reg_read(JOYSTICK_ADDR,WHO_AM_I);
  // //checking to see if properly connected to the presence sensor
}

//read temp_shock
bool read_temp_shock(void) {
  uint8_t status = i2c_reg_read(PRES_ADDR, STATUS);
  printf("status  %u\n", status);
  if (status != 0) {
    uint8_t values = i2c_reg_read(PRES_ADDR, FUNC_STATUS) & 0x01;
    printf("tamb_shock %u\n", values);
    return values;
  } else {
    return false;
  }
}

float get_vertical()
{
  uint16_t Y_MSB = i2c_reg_read(JOYSTICK_ADDR, JOYSTICK_Y_MSB);
  uint16_t Y_LSB = i2c_reg_read(JOYSTICK_ADDR, JOYSTICK_Y_LSB);
  uint16_t val = ((Y_MSB<<8) | Y_LSB)>>6;
  printf("val: %d\n", val);
  //maybe divide from 0 value and represent in float might cause overflow!
  float divided = ((float)val) / 60.0f; 
  float converted_num = ((float) divided) - 8.0;
  printf("converted_num: %f\n", converted_num);
  return converted_num;
}
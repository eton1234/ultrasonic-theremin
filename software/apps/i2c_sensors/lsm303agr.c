// LSM303AGR driver for Microbit_v2
//
// Initializes sensor and communicates over I2C
// Capable of reading temperature, acceleration, and magnetic field strength

#include <stdbool.h>
#include <stdint.h>
#include <math.h>

#include "lsm303agr.h"
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
  static uint8_t rx_buf;
  printf("i2c_addr %u\n", i2c_addr);
  printf("i2c_addr adjusted %u\n", (i2c_addr << 1) | 0);
  printf("reg addr: %u\n", reg_addr); 
  nrf_twi_mngr_transfer_t write = NRF_TWI_MNGR_WRITE(i2c_addr, &reg_addr, 1, NRF_TWI_MNGR_NO_STOP); 
  printf("hi trying to read");
  nrf_twi_mngr_transfer_t read= NRF_TWI_MNGR_READ(i2c_addr, &rx_buf, 1, 0);
  nrf_twi_mngr_transfer_t const read_transfer[] = {
    write,
    read
  };
  printf("before perform...\n"); 
  ret_code_t result = nrf_twi_mngr_perform(i2c_manager, NULL, read_transfer, 2, NULL);
  printf("after perform...\n"); 
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
  printf("hi \n");
  i2c_manager = i2c;
  uint8_t who_am_i = i2c_reg_read(PRES_ADDR,0x0F);
}

void lsm303agr_init(const nrf_twi_mngr_t* i2c) {
  i2c_manager = i2c;
  // ---Initialize Accelerometer---
  // Reboot acclerometer
  i2c_reg_write(LSM303AGR_ACC_ADDRESS, CTRL_REG5_A, 0x80);
  nrf_delay_ms(100); // needs delay to wait for reboot

  // Enable Block Data Update
  // Only updates sensor data when both halves of the data has been read
  i2c_reg_write(LSM303AGR_ACC_ADDRESS, CTRL_REG4_A, 0x80);

  // Configure accelerometer at 100Hz, normal mode (10-bit)
  // Enable x, y and z axes
  i2c_reg_write(LSM303AGR_ACC_ADDRESS, CTRL_REG1_A, 0x57);

  // Read WHO AM I register
  // Always returns the same value if working
  //TODO: read the Accelerometer WHO AM I register and check the result
  uint8_t whoamIVal = i2c_reg_read(LSM303AGR_ACC_ADDRESS, WHO_AM_I_A);
  printf("from accel: %x\n", whoamIVal); 

  // ---Initialize Magnetometer---

  // Reboot magnetometer
  i2c_reg_write(LSM303AGR_MAG_ADDRESS, CFG_REG_A_M, 0x40);
  nrf_delay_ms(100); // needs delay to wait for reboot

  // Enable Block Data Update
  // Only updates sensor data when both halves of the data has been read
  i2c_reg_write(LSM303AGR_MAG_ADDRESS, CFG_REG_C_M, 0x10);

  // Configure magnetometer at 100Hz, continuous mode
  i2c_reg_write(LSM303AGR_MAG_ADDRESS, CFG_REG_A_M, 0x0C);

  // Read WHO AM I register
  //TODO: read the Magnetometer WHO AM I register and check the result
  uint8_t whoamIMag = i2c_reg_read(LSM303AGR_MAG_ADDRESS, WHO_AM_I_M);
  printf("from mag: %x\n", whoamIMag); 
  // ---Initialize Temperature---

  // Enable temperature sensor
  i2c_reg_write(LSM303AGR_ACC_ADDRESS, TEMP_CFG_REG_A, 0xC0);
}

// Read the internal temperature sensor
//
// Return measurement as floating point value in degrees C
float lsm303agr_read_temperature(void) {
  //TODO: implement me
  uint16_t low = (uint16_t) i2c_reg_read(LSM303AGR_ACC_ADDRESS, OUT_TEMP_L_A);
  uint16_t high = (uint16_t) i2c_reg_read(LSM303AGR_ACC_ADDRESS, OUT_TEMP_H_A); 
  float overall = (float) ((high << 8) | low); 
  overall = ((float)overall / 256) + 25;
  printf("temperature reading: %f\n", overall); 
  return overall; 
}

lsm303agr_measurement_t lsm303agr_read_accelerometer(void) {
  //TODO: implement me
  int8_t low_x = i2c_reg_read(LSM303AGR_ACC_ADDRESS, OUT_X_L_A);
  int8_t high_x = i2c_reg_read(LSM303AGR_ACC_ADDRESS, OUT_X_H_A); 
  float x_overall = (3.9/1000) * (((high_x << 8) | low_x) >> 6); 
  int8_t low_y =  i2c_reg_read(LSM303AGR_ACC_ADDRESS, OUT_Y_L_A);
  int8_t high_y = i2c_reg_read(LSM303AGR_ACC_ADDRESS, OUT_Y_H_A); 
  float y_overall = (3.9/1000) * (((high_y << 8) | low_y) >> 6);
  int8_t low_z = i2c_reg_read(LSM303AGR_ACC_ADDRESS, OUT_Z_L_A);
  int8_t high_z = i2c_reg_read(LSM303AGR_ACC_ADDRESS, OUT_Z_H_A); 
  float z_overall = (3.9/1000) * (((high_z << 8) | low_z) >> 6); 
  lsm303agr_measurement_t measurement = {x_overall, y_overall, z_overall};
  printf("accelerometer reading: %f, %f, %f\n", measurement.x_axis,measurement.y_axis, measurement.z_axis); 
  return measurement;
}

lsm303agr_measurement_t lsm303agr_read_magnetometer(void) {
  //TODO: implement me

  int8_t low_x = i2c_reg_read(LSM303AGR_MAG_ADDRESS, OUTX_L_REG_M);
  int8_t high_x = i2c_reg_read(LSM303AGR_MAG_ADDRESS, OUTX_H_REG_M); 
  float overall_x = (1.5/10) * ((high_x << 8) | low_x); 
  int8_t low_y = i2c_reg_read(LSM303AGR_MAG_ADDRESS, OUTY_L_REG_M);
  int8_t high_y = i2c_reg_read(LSM303AGR_MAG_ADDRESS, OUTY_H_REG_M);
  float overall_y = (1.5/10) * ((high_y << 8) | low_y); 
  int8_t low_z = i2c_reg_read(LSM303AGR_MAG_ADDRESS, OUTZ_L_REG_M);
  int8_t high_z = i2c_reg_read(LSM303AGR_MAG_ADDRESS, OUTZ_H_REG_M);
  float overall_z = (1.5/10) * ((high_z << 8) | low_z); 
  lsm303agr_measurement_t result;
  result.x_axis = overall_x;
  result.y_axis = overall_y;
  result.z_axis = overall_z; 
  printf("magnetometer reading: %f, %f, %f\n", result.x_axis, result.y_axis, result.z_axis); 
  return result;
}

double computeTiltAngle(){
  lsm303agr_measurement_t measurement = lsm303agr_read_accelerometer(); 
  float a_xout = measurement.x_axis;
  float a_yout = measurement.y_axis; 
  float a_zout = measurement.z_axis; 
  double result = atan(sqrt((a_xout * a_xout) + (a_yout * a_yout)) / a_zout) * 57.2957795;
  printf("tilt angle: %f\n", result); 
  return result; 
}

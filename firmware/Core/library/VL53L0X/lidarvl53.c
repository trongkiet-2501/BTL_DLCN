/*
 * lidarvl53.c
 *
 *  Created on: Jul 20, 2023
 *      Author: alcid
 */


#include "lidarvl53.h"
#include <stdio.h>

extern void UART_Send_Text(const char *text);

VL53L0X_RangingMeasurementData_t RangingData;
VL53L0X_Dev_t  vl53l0x_c; // center module
VL53L0X_DEV    Dev = &vl53l0x_c;
uint32_t refSpadCount;
  uint8_t isApertureSpads;
  uint8_t VhvSettings;
  uint8_t PhaseCal;

extern I2C_HandleTypeDef hi2c1;


void lidar_init(uint8_t dir)
{
	  Dev->I2cHandle = &hi2c1;//el puerto i2c a usar
	  Dev->I2cDevAddr = dir;
      Dev->comms_type=1;
      Dev->comms_speed_khz=100;//  i2c a 100khz

      VL53L0X_Error s;
      char buf[64];

      s = VL53L0X_WaitDeviceBooted( Dev );
      snprintf(buf, sizeof(buf), "Boot:%d\r\n", s); UART_Send_Text(buf);

	  s = VL53L0X_DataInit( Dev );
      snprintf(buf, sizeof(buf), "DataInit:%d\r\n", s); UART_Send_Text(buf);

	  s = VL53L0X_StaticInit( Dev );
      snprintf(buf, sizeof(buf), "StaticInit:%d\r\n", s); UART_Send_Text(buf);

	  s = VL53L0X_PerformRefCalibration(Dev, &VhvSettings, &PhaseCal);
      snprintf(buf, sizeof(buf), "RefCal:%d\r\n", s); UART_Send_Text(buf);

	  s = VL53L0X_PerformRefSpadManagement(Dev, &refSpadCount, &isApertureSpads);
      snprintf(buf, sizeof(buf), "Spad:%d\r\n", s); UART_Send_Text(buf);

	  VL53L0X_SetDeviceMode(Dev, VL53L0X_DEVICEMODE_SINGLE_RANGING);
	  // Enable/Disable Sigma and Signal check
	  VL53L0X_SetLimitCheckEnable(Dev, VL53L0X_CHECKENABLE_SIGMA_FINAL_RANGE, 1);
	  VL53L0X_SetLimitCheckEnable(Dev, VL53L0X_CHECKENABLE_SIGNAL_RATE_FINAL_RANGE, 1);
	  VL53L0X_SetLimitCheckValue(Dev, VL53L0X_CHECKENABLE_SIGNAL_RATE_FINAL_RANGE, (FixPoint1616_t)(0.1*65536));
	  VL53L0X_SetLimitCheckValue(Dev, VL53L0X_CHECKENABLE_SIGMA_FINAL_RANGE, (FixPoint1616_t)(60*65536));
	  VL53L0X_SetMeasurementTimingBudgetMicroSeconds(Dev, 33000);
	  VL53L0X_SetVcselPulsePeriod(Dev, VL53L0X_VCSEL_PERIOD_PRE_RANGE, 18);
	  VL53L0X_SetVcselPulsePeriod(Dev, VL53L0X_VCSEL_PERIOD_FINAL_RANGE, 14);

}

uint16_t lidar_lee_mm(uint8_t dir)
{
	Dev->I2cDevAddr = dir;
 	VL53L0X_Error status = VL53L0X_PerformSingleRangingMeasurement(Dev, &RangingData);
    if (status != VL53L0X_ERROR_NONE) {
        return (uint16_t)(1000 - status);
    }
 	return RangingData.RangeMilliMeter;
}


float lidar_lee_cm(uint8_t dir)
{
	return((float)lidar_lee_mm(dir)/10.0);
}

uint8_t lidar_set_dir(uint8_t dir)
{
	uint8_t status = VL53L0X_SetDeviceAddress(Dev, dir << 1);
	return(status);

}


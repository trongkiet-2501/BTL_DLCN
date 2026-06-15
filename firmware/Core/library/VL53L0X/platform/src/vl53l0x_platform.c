//?????#include "hal.h"
#include "vl53l0x_platform.h"
#include "vl53l0x_api.h"

#include "stm32f1xx_hal.h"
#include <string.h>

#define I2C_TIME_OUT_BASE   100
#define I2C_TIME_OUT_BYTE   2
#define VL53L0X_OsDelay(...) HAL_Delay(2)


#ifndef HAL_I2C_MODULE_ENABLED
#warning "HAL I2C module must be enable "
#endif

/* when not customized by application define dummy one */
#ifndef VL53L0X_GetI2cBus
#   define VL53L0X_GetI2cBus(...) (void)0
#endif

#ifndef VL53L0X_PutI2cBus
#   define VL53L0X_PutI2cBus(...) (void)0
#endif

#ifndef VL53L0X_OsDelay
#   define  VL53L0X_OsDelay(...) (void)0
#endif


VL53L0X_Error VL53L0X_WriteMulti(VL53L0X_DEV Dev, uint8_t index, uint8_t *pdata, uint32_t count) {
    VL53L0X_Error Status = VL53L0X_ERROR_NONE;
    int i2c_time_out = I2C_TIME_OUT_BASE + count * I2C_TIME_OUT_BYTE;
    
    VL53L0X_GetI2cBus();
    if (HAL_I2C_Mem_Write(Dev->I2cHandle, Dev->I2cDevAddr, index, I2C_MEMADD_SIZE_8BIT, pdata, count, i2c_time_out) != HAL_OK) {
        Status = VL53L0X_ERROR_CONTROL_INTERFACE;
    }
    VL53L0X_PutI2cBus();
    return Status;
}

VL53L0X_Error VL53L0X_ReadMulti(VL53L0X_DEV Dev, uint8_t index, uint8_t *pdata, uint32_t count) {
    VL53L0X_Error Status = VL53L0X_ERROR_NONE;
    int i2c_time_out = I2C_TIME_OUT_BASE + count * I2C_TIME_OUT_BYTE;
    
    VL53L0X_GetI2cBus();
    // Using HAL_I2C_Mem_Read ensures a Repeated Start is sent, which is required by VL53L0X
    if (HAL_I2C_Mem_Read(Dev->I2cHandle, Dev->I2cDevAddr, index, I2C_MEMADD_SIZE_8BIT, pdata, count, i2c_time_out) != HAL_OK) {
        Status = VL53L0X_ERROR_CONTROL_INTERFACE;
    }
    VL53L0X_PutI2cBus();
    return Status;
}

VL53L0X_Error VL53L0X_WrByte(VL53L0X_DEV Dev, uint8_t index, uint8_t data) {
    return VL53L0X_WriteMulti(Dev, index, &data, 1);
}

VL53L0X_Error VL53L0X_WrWord(VL53L0X_DEV Dev, uint8_t index, uint16_t data) {
    uint8_t buffer[2];
    buffer[0] = data >> 8;
    buffer[1] = data & 0xFF;
    return VL53L0X_WriteMulti(Dev, index, buffer, 2);
}

VL53L0X_Error VL53L0X_WrDWord(VL53L0X_DEV Dev, uint8_t index, uint32_t data) {
    uint8_t buffer[4];
    buffer[0] = (data >> 24) & 0xFF;
    buffer[1] = (data >> 16) & 0xFF;
    buffer[2] = (data >> 8) & 0xFF;
    buffer[3] = (data >> 0) & 0xFF;
    return VL53L0X_WriteMulti(Dev, index, buffer, 4);
}

VL53L0X_Error VL53L0X_UpdateByte(VL53L0X_DEV Dev, uint8_t index, uint8_t AndData, uint8_t OrData) {
    VL53L0X_Error Status = VL53L0X_ERROR_NONE;
    uint8_t data;

    Status = VL53L0X_RdByte(Dev, index, &data);
    if (Status) {
        return Status;
    }
    data = (data & AndData) | OrData;
    Status = VL53L0X_WrByte(Dev, index, data);
    return Status;
}

VL53L0X_Error VL53L0X_RdByte(VL53L0X_DEV Dev, uint8_t index, uint8_t *data) {
    return VL53L0X_ReadMulti(Dev, index, data, 1);
}

VL53L0X_Error VL53L0X_RdWord(VL53L0X_DEV Dev, uint8_t index, uint16_t *data) {
    uint8_t buffer[2];
    VL53L0X_Error status = VL53L0X_ReadMulti(Dev, index, buffer, 2);
    if (status == VL53L0X_ERROR_NONE) {
        *data = ((uint16_t)buffer[0] << 8) | buffer[1];
    }
    return status;
}

VL53L0X_Error VL53L0X_RdDWord(VL53L0X_DEV Dev, uint8_t index, uint32_t *data) {
    uint8_t buffer[4];
    VL53L0X_Error status = VL53L0X_ReadMulti(Dev, index, buffer, 4);
    if (status == VL53L0X_ERROR_NONE) {
        *data = ((uint32_t)buffer[0] << 24) | ((uint32_t)buffer[1] << 16) | ((uint32_t)buffer[2] << 8) | buffer[3];
    }
    return status;
}

VL53L0X_Error VL53L0X_PollingDelay(VL53L0X_DEV Dev) {
    VL53L0X_Error status = VL53L0X_ERROR_NONE;

    // do nothing
    VL53L0X_OsDelay();
    return status;
}

//end of file

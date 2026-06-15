################################################################################
# Automatically-generated file. Do not edit!
# Toolchain: GNU Tools for STM32 (13.3.rel1)
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../Core/library/VL53L0X/core/src/vl53l0x_api.c \
../Core/library/VL53L0X/core/src/vl53l0x_api_calibration.c \
../Core/library/VL53L0X/core/src/vl53l0x_api_core.c \
../Core/library/VL53L0X/core/src/vl53l0x_api_ranging.c \
../Core/library/VL53L0X/core/src/vl53l0x_api_strings.c 

OBJS += \
./Core/library/VL53L0X/core/src/vl53l0x_api.o \
./Core/library/VL53L0X/core/src/vl53l0x_api_calibration.o \
./Core/library/VL53L0X/core/src/vl53l0x_api_core.o \
./Core/library/VL53L0X/core/src/vl53l0x_api_ranging.o \
./Core/library/VL53L0X/core/src/vl53l0x_api_strings.o 

C_DEPS += \
./Core/library/VL53L0X/core/src/vl53l0x_api.d \
./Core/library/VL53L0X/core/src/vl53l0x_api_calibration.d \
./Core/library/VL53L0X/core/src/vl53l0x_api_core.d \
./Core/library/VL53L0X/core/src/vl53l0x_api_ranging.d \
./Core/library/VL53L0X/core/src/vl53l0x_api_strings.d 


# Each subdirectory must supply rules for building sources it contributes
Core/library/VL53L0X/core/src/%.o Core/library/VL53L0X/core/src/%.su Core/library/VL53L0X/core/src/%.cyclo: ../Core/library/VL53L0X/core/src/%.c Core/library/VL53L0X/core/src/subdir.mk
	arm-none-eabi-gcc "$<" -mcpu=cortex-m3 -std=gnu11 -g3 -DDEBUG -DUSE_HAL_DRIVER -DSTM32F103xB -c -I../Core/library/VL53L0X -I../Core/library/VL53L0X/core/inc -I../Core/library/VL53L0X/platform/inc -I../Core/Inc -I../Drivers/STM32F1xx_HAL_Driver/Inc/Legacy -I../Drivers/STM32F1xx_HAL_Driver/Inc -I../Drivers/CMSIS/Device/ST/STM32F1xx/Include -I../Drivers/CMSIS/Include -I../USB_DEVICE/App -I../USB_DEVICE/Target -I../Middlewares/ST/STM32_USB_Device_Library/Core/Inc -I../Middlewares/ST/STM32_USB_Device_Library/Class/CDC/Inc -Os -ffunction-sections -fdata-sections -Wall -fstack-usage -fcyclomatic-complexity -MMD -MP -MF"$(@:%.o=%.d)" -MT"$@" --specs=nano.specs -mfloat-abi=soft -mthumb -o "$@"

clean: clean-Core-2f-library-2f-VL53L0X-2f-core-2f-src

clean-Core-2f-library-2f-VL53L0X-2f-core-2f-src:
	-$(RM) ./Core/library/VL53L0X/core/src/vl53l0x_api.cyclo ./Core/library/VL53L0X/core/src/vl53l0x_api.d ./Core/library/VL53L0X/core/src/vl53l0x_api.o ./Core/library/VL53L0X/core/src/vl53l0x_api.su ./Core/library/VL53L0X/core/src/vl53l0x_api_calibration.cyclo ./Core/library/VL53L0X/core/src/vl53l0x_api_calibration.d ./Core/library/VL53L0X/core/src/vl53l0x_api_calibration.o ./Core/library/VL53L0X/core/src/vl53l0x_api_calibration.su ./Core/library/VL53L0X/core/src/vl53l0x_api_core.cyclo ./Core/library/VL53L0X/core/src/vl53l0x_api_core.d ./Core/library/VL53L0X/core/src/vl53l0x_api_core.o ./Core/library/VL53L0X/core/src/vl53l0x_api_core.su ./Core/library/VL53L0X/core/src/vl53l0x_api_ranging.cyclo ./Core/library/VL53L0X/core/src/vl53l0x_api_ranging.d ./Core/library/VL53L0X/core/src/vl53l0x_api_ranging.o ./Core/library/VL53L0X/core/src/vl53l0x_api_ranging.su ./Core/library/VL53L0X/core/src/vl53l0x_api_strings.cyclo ./Core/library/VL53L0X/core/src/vl53l0x_api_strings.d ./Core/library/VL53L0X/core/src/vl53l0x_api_strings.o ./Core/library/VL53L0X/core/src/vl53l0x_api_strings.su

.PHONY: clean-Core-2f-library-2f-VL53L0X-2f-core-2f-src


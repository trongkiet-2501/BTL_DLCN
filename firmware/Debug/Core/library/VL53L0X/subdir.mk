################################################################################
# Automatically-generated file. Do not edit!
# Toolchain: GNU Tools for STM32 (13.3.rel1)
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../Core/library/VL53L0X/lidarvl53.c 

OBJS += \
./Core/library/VL53L0X/lidarvl53.o 

C_DEPS += \
./Core/library/VL53L0X/lidarvl53.d 


# Each subdirectory must supply rules for building sources it contributes
Core/library/VL53L0X/%.o Core/library/VL53L0X/%.su Core/library/VL53L0X/%.cyclo: ../Core/library/VL53L0X/%.c Core/library/VL53L0X/subdir.mk
	arm-none-eabi-gcc "$<" -mcpu=cortex-m3 -std=gnu11 -g3 -DDEBUG -DUSE_HAL_DRIVER -DSTM32F103xB -c -I../Core/library/VL53L0X -I../Core/library/VL53L0X/core/inc -I../Core/library/VL53L0X/platform/inc -I../Core/Inc -I../Drivers/STM32F1xx_HAL_Driver/Inc/Legacy -I../Drivers/STM32F1xx_HAL_Driver/Inc -I../Drivers/CMSIS/Device/ST/STM32F1xx/Include -I../Drivers/CMSIS/Include -I../USB_DEVICE/App -I../USB_DEVICE/Target -I../Middlewares/ST/STM32_USB_Device_Library/Core/Inc -I../Middlewares/ST/STM32_USB_Device_Library/Class/CDC/Inc -Os -ffunction-sections -fdata-sections -Wall -fstack-usage -fcyclomatic-complexity -MMD -MP -MF"$(@:%.o=%.d)" -MT"$@" --specs=nano.specs -mfloat-abi=soft -mthumb -o "$@"

clean: clean-Core-2f-library-2f-VL53L0X

clean-Core-2f-library-2f-VL53L0X:
	-$(RM) ./Core/library/VL53L0X/lidarvl53.cyclo ./Core/library/VL53L0X/lidarvl53.d ./Core/library/VL53L0X/lidarvl53.o ./Core/library/VL53L0X/lidarvl53.su

.PHONY: clean-Core-2f-library-2f-VL53L0X


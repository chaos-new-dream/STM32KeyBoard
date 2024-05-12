################################################################################
# Automatically-generated file. Do not edit!
# Toolchain: GNU Tools for STM32 (10.3-2021.10)
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../Drivers/LCD/SRC/lcd.c \
../Drivers/LCD/SRC/lcd_init.c 

C_DEPS += \
./Drivers/LCD/SRC/lcd.d \
./Drivers/LCD/SRC/lcd_init.d 

OBJS += \
./Drivers/LCD/SRC/lcd.o \
./Drivers/LCD/SRC/lcd_init.o 


# Each subdirectory must supply rules for building sources it contributes
Drivers/LCD/SRC/%.o Drivers/LCD/SRC/%.su Drivers/LCD/SRC/%.cyclo: ../Drivers/LCD/SRC/%.c Drivers/LCD/SRC/subdir.mk
	arm-none-eabi-gcc "$<" -mcpu=cortex-m3 -std=gnu11 -g3 -DDEBUG -DUSE_HAL_DRIVER -DSTM32F103xE -c -I../Core/Inc -I../Drivers/STM32F1xx_HAL_Driver/Inc/Legacy -I../Drivers/STM32F1xx_HAL_Driver/Inc -I../Drivers/CMSIS/Device/ST/STM32F1xx/Include -I../Drivers/CMSIS/Include -I../USB_DEVICE/App -I../USB_DEVICE/Target -I../Middlewares/ST/STM32_USB_Device_Library/Core/Inc -I../Middlewares/ST/STM32_USB_Device_Library/Class/HID/Inc -O0 -ffunction-sections -fdata-sections -Wall -fstack-usage -fcyclomatic-complexity -MMD -MP -MF"$(@:%.o=%.d)" -MT"$@" --specs=nano.specs -mfloat-abi=soft -mthumb -o "$@"

clean: clean-Drivers-2f-LCD-2f-SRC

clean-Drivers-2f-LCD-2f-SRC:
	-$(RM) ./Drivers/LCD/SRC/lcd.cyclo ./Drivers/LCD/SRC/lcd.d ./Drivers/LCD/SRC/lcd.o ./Drivers/LCD/SRC/lcd.su ./Drivers/LCD/SRC/lcd_init.cyclo ./Drivers/LCD/SRC/lcd_init.d ./Drivers/LCD/SRC/lcd_init.o ./Drivers/LCD/SRC/lcd_init.su

.PHONY: clean-Drivers-2f-LCD-2f-SRC


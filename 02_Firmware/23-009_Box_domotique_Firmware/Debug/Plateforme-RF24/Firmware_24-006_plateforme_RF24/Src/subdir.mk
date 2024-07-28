################################################################################
# Automatically-generated file. Do not edit!
# Toolchain: GNU Tools for STM32 (12.3.rel1)
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../Plateforme-RF24/Firmware_24-006_plateforme_RF24/Src/NRF24L01.c \
../Plateforme-RF24/Firmware_24-006_plateforme_RF24/Src/debug.c \
../Plateforme-RF24/Firmware_24-006_plateforme_RF24/Src/frames.c \
../Plateforme-RF24/Firmware_24-006_plateforme_RF24/Src/log.c \
../Plateforme-RF24/Firmware_24-006_plateforme_RF24/Src/radio.c \
../Plateforme-RF24/Firmware_24-006_plateforme_RF24/Src/tools.c 

OBJS += \
./Plateforme-RF24/Firmware_24-006_plateforme_RF24/Src/NRF24L01.o \
./Plateforme-RF24/Firmware_24-006_plateforme_RF24/Src/debug.o \
./Plateforme-RF24/Firmware_24-006_plateforme_RF24/Src/frames.o \
./Plateforme-RF24/Firmware_24-006_plateforme_RF24/Src/log.o \
./Plateforme-RF24/Firmware_24-006_plateforme_RF24/Src/radio.o \
./Plateforme-RF24/Firmware_24-006_plateforme_RF24/Src/tools.o 

C_DEPS += \
./Plateforme-RF24/Firmware_24-006_plateforme_RF24/Src/NRF24L01.d \
./Plateforme-RF24/Firmware_24-006_plateforme_RF24/Src/debug.d \
./Plateforme-RF24/Firmware_24-006_plateforme_RF24/Src/frames.d \
./Plateforme-RF24/Firmware_24-006_plateforme_RF24/Src/log.d \
./Plateforme-RF24/Firmware_24-006_plateforme_RF24/Src/radio.d \
./Plateforme-RF24/Firmware_24-006_plateforme_RF24/Src/tools.d 


# Each subdirectory must supply rules for building sources it contributes
Plateforme-RF24/Firmware_24-006_plateforme_RF24/Src/%.o Plateforme-RF24/Firmware_24-006_plateforme_RF24/Src/%.su Plateforme-RF24/Firmware_24-006_plateforme_RF24/Src/%.cyclo: ../Plateforme-RF24/Firmware_24-006_plateforme_RF24/Src/%.c Plateforme-RF24/Firmware_24-006_plateforme_RF24/Src/subdir.mk
	arm-none-eabi-gcc "$<" -mcpu=cortex-m0plus -std=gnu11 -g3 -DDEBUG -DUSE_HAL_DRIVER -DSTM32L010x6 -c -I../../Core/Inc -I../../Drivers/STM32L0xx_HAL_Driver/Inc -I../../Drivers/STM32L0xx_HAL_Driver/Inc/Legacy -I../../Drivers/CMSIS/Device/ST/STM32L0xx/Include -I../../Drivers/CMSIS/Include -I../Core/Inc -I../Drivers/STM32L0xx_HAL_Driver/Inc -I../Drivers/STM32L0xx_HAL_Driver/Inc/Legacy -I../Drivers/CMSIS/Device/ST/STM32L0xx/Include -I../Drivers/CMSIS/Include -I"C:/Projets/23-011_Box_domotique/02_Firmware/23-009_Box_domotique_Firmware/Plateforme-RF24/Firmware_24-006_plateforme_RF24/Inc" -O0 -ffunction-sections -fdata-sections -Wall -fstack-usage -fcyclomatic-complexity -MMD -MP -MF"$(@:%.o=%.d)" -MT"$@" --specs=nano.specs -mfloat-abi=soft -mthumb -o "$@"

clean: clean-Plateforme-2d-RF24-2f-Firmware_24-2d-006_plateforme_RF24-2f-Src

clean-Plateforme-2d-RF24-2f-Firmware_24-2d-006_plateforme_RF24-2f-Src:
	-$(RM) ./Plateforme-RF24/Firmware_24-006_plateforme_RF24/Src/NRF24L01.cyclo ./Plateforme-RF24/Firmware_24-006_plateforme_RF24/Src/NRF24L01.d ./Plateforme-RF24/Firmware_24-006_plateforme_RF24/Src/NRF24L01.o ./Plateforme-RF24/Firmware_24-006_plateforme_RF24/Src/NRF24L01.su ./Plateforme-RF24/Firmware_24-006_plateforme_RF24/Src/debug.cyclo ./Plateforme-RF24/Firmware_24-006_plateforme_RF24/Src/debug.d ./Plateforme-RF24/Firmware_24-006_plateforme_RF24/Src/debug.o ./Plateforme-RF24/Firmware_24-006_plateforme_RF24/Src/debug.su ./Plateforme-RF24/Firmware_24-006_plateforme_RF24/Src/frames.cyclo ./Plateforme-RF24/Firmware_24-006_plateforme_RF24/Src/frames.d ./Plateforme-RF24/Firmware_24-006_plateforme_RF24/Src/frames.o ./Plateforme-RF24/Firmware_24-006_plateforme_RF24/Src/frames.su ./Plateforme-RF24/Firmware_24-006_plateforme_RF24/Src/log.cyclo ./Plateforme-RF24/Firmware_24-006_plateforme_RF24/Src/log.d ./Plateforme-RF24/Firmware_24-006_plateforme_RF24/Src/log.o ./Plateforme-RF24/Firmware_24-006_plateforme_RF24/Src/log.su ./Plateforme-RF24/Firmware_24-006_plateforme_RF24/Src/radio.cyclo ./Plateforme-RF24/Firmware_24-006_plateforme_RF24/Src/radio.d ./Plateforme-RF24/Firmware_24-006_plateforme_RF24/Src/radio.o ./Plateforme-RF24/Firmware_24-006_plateforme_RF24/Src/radio.su ./Plateforme-RF24/Firmware_24-006_plateforme_RF24/Src/tools.cyclo ./Plateforme-RF24/Firmware_24-006_plateforme_RF24/Src/tools.d ./Plateforme-RF24/Firmware_24-006_plateforme_RF24/Src/tools.o ./Plateforme-RF24/Firmware_24-006_plateforme_RF24/Src/tools.su

.PHONY: clean-Plateforme-2d-RF24-2f-Firmware_24-2d-006_plateforme_RF24-2f-Src


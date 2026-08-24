################################################################################
# Automatically-generated file. Do not edit!
# Toolchain: GNU Tools for STM32 (13.3.rel1)
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../Drivers/MMC5983/MMC5983.c 

OBJS += \
./Drivers/MMC5983/MMC5983.o 

C_DEPS += \
./Drivers/MMC5983/MMC5983.d 


# Each subdirectory must supply rules for building sources it contributes
Drivers/MMC5983/%.o Drivers/MMC5983/%.su Drivers/MMC5983/%.cyclo: ../Drivers/MMC5983/%.c Drivers/MMC5983/subdir.mk
	arm-none-eabi-gcc "$<" -mcpu=cortex-m4 -std=gnu11 -g3 -DDEBUG -DUSE_HAL_DRIVER -DSTM32L471xx -c -I../Core/Inc -I../Drivers/STM32L4xx_HAL_Driver/Inc -I../Drivers/STM32L4xx_HAL_Driver/Inc/Legacy -I../Drivers/CMSIS/Device/ST/STM32L4xx/Include -I../Drivers/CMSIS/Include -I"C:/Users/smpet/Desktop/SpacecraftProject/Drivers/MMC5983" -I"C:/Users/smpet/Desktop/SpacecraftProject/Drivers" -I"C:/Users/smpet/Desktop/SpacecraftProject/Drivers/STM32L4xx_HAL_Driver" -O0 -ffunction-sections -fdata-sections -Wall -fstack-usage -fcyclomatic-complexity -MMD -MP -MF"$(@:%.o=%.d)" -MT"$@" --specs=nano.specs -mfpu=fpv4-sp-d16 -mfloat-abi=hard -mthumb -o "$@"

clean: clean-Drivers-2f-MMC5983

clean-Drivers-2f-MMC5983:
	-$(RM) ./Drivers/MMC5983/MMC5983.cyclo ./Drivers/MMC5983/MMC5983.d ./Drivers/MMC5983/MMC5983.o ./Drivers/MMC5983/MMC5983.su

.PHONY: clean-Drivers-2f-MMC5983


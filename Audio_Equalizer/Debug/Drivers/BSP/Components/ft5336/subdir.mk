################################################################################
# Automatically-generated file. Do not edit!
# Toolchain: GNU Tools for STM32 (10.3-2021.10)
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../Drivers/BSP/Components/ft5336/ft5336.c \
../Drivers/BSP/Components/ft5336/ft5336_reg.c 

OBJS += \
./Drivers/BSP/Components/ft5336/ft5336.o \
./Drivers/BSP/Components/ft5336/ft5336_reg.o 

C_DEPS += \
./Drivers/BSP/Components/ft5336/ft5336.d \
./Drivers/BSP/Components/ft5336/ft5336_reg.d 


# Each subdirectory must supply rules for building sources it contributes
Drivers/BSP/Components/ft5336/%.o Drivers/BSP/Components/ft5336/%.su: ../Drivers/BSP/Components/ft5336/%.c Drivers/BSP/Components/ft5336/subdir.mk
	arm-none-eabi-gcc "$<" -mcpu=cortex-m7 -std=gnu11 -g3 -DUSE_HAL_DRIVER -DUSE_STM32H735G_DISCO -DDEBUG -DSTM32H735xx -D_FPU_PRESENT=1 -DARM_MATH_CM7 -c -I../Inc -I../Drivers/STM32H7xx_HAL_Driver/Inc -I"../Drivers/CMSIS/Device/ST/STM32H7xx/Include" -I"../Drivers/CMSIS/Core/Include" -I"../Drivers/BSP/STM32H735G-DK" -I"../Drivers/BSP/Components/Common" -I../Middlewares/ST/STM32_Audio/Addons/PDM/Inc -I../Utilities/lcd -I../Drivers/CMSIS/DSP/Include -O0 -ffunction-sections -fdata-sections -Wall -fstack-usage -MMD -MP -MF"$(@:%.o=%.d)" -MT"$@" --specs=nano.specs -mfpu=fpv5-d16 -mfloat-abi=hard -mthumb -o "$@"

clean: clean-Drivers-2f-BSP-2f-Components-2f-ft5336

clean-Drivers-2f-BSP-2f-Components-2f-ft5336:
	-$(RM) ./Drivers/BSP/Components/ft5336/ft5336.d ./Drivers/BSP/Components/ft5336/ft5336.o ./Drivers/BSP/Components/ft5336/ft5336.su ./Drivers/BSP/Components/ft5336/ft5336_reg.d ./Drivers/BSP/Components/ft5336/ft5336_reg.o ./Drivers/BSP/Components/ft5336/ft5336_reg.su

.PHONY: clean-Drivers-2f-BSP-2f-Components-2f-ft5336


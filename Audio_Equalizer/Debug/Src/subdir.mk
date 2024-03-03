################################################################################
# Automatically-generated file. Do not edit!
# Toolchain: GNU Tools for STM32 (10.3-2021.10)
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../Src/display.c \
../Src/events.c \
../Src/lab.c \
../Src/main.c \
../Src/stm32h7xx_it.c \
../Src/system_stm32h7xx.c \
../Src/touchscreen.c 

OBJS += \
./Src/display.o \
./Src/events.o \
./Src/lab.o \
./Src/main.o \
./Src/stm32h7xx_it.o \
./Src/system_stm32h7xx.o \
./Src/touchscreen.o 

C_DEPS += \
./Src/display.d \
./Src/events.d \
./Src/lab.d \
./Src/main.d \
./Src/stm32h7xx_it.d \
./Src/system_stm32h7xx.d \
./Src/touchscreen.d 


# Each subdirectory must supply rules for building sources it contributes
Src/%.o Src/%.su: ../Src/%.c Src/subdir.mk
	arm-none-eabi-gcc "$<" -mcpu=cortex-m7 -std=gnu11 -g3 -DUSE_HAL_DRIVER -DUSE_STM32H735G_DISCO -DDEBUG -DSTM32H735xx -D_FPU_PRESENT=1 -DARM_MATH_CM7 -c -I../Inc -I../Drivers/STM32H7xx_HAL_Driver/Inc -I"../Drivers/CMSIS/Device/ST/STM32H7xx/Include" -I"../Drivers/CMSIS/Core/Include" -I"../Drivers/BSP/STM32H735G-DK" -I"../Drivers/BSP/Components/Common" -I../Middlewares/ST/STM32_Audio/Addons/PDM/Inc -I../Utilities/lcd -I../Drivers/CMSIS/DSP/Include -O0 -ffunction-sections -fdata-sections -Wall -fstack-usage -MMD -MP -MF"$(@:%.o=%.d)" -MT"$@" --specs=nano.specs -mfpu=fpv5-d16 -mfloat-abi=hard -mthumb -o "$@"

clean: clean-Src

clean-Src:
	-$(RM) ./Src/display.d ./Src/display.o ./Src/display.su ./Src/events.d ./Src/events.o ./Src/events.su ./Src/lab.d ./Src/lab.o ./Src/lab.su ./Src/main.d ./Src/main.o ./Src/main.su ./Src/stm32h7xx_it.d ./Src/stm32h7xx_it.o ./Src/stm32h7xx_it.su ./Src/system_stm32h7xx.d ./Src/system_stm32h7xx.o ./Src/system_stm32h7xx.su ./Src/touchscreen.d ./Src/touchscreen.o ./Src/touchscreen.su

.PHONY: clean-Src


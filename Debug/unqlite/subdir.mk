################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../unqlite/unqlite.c 

OBJS += \
./unqlite/unqlite.o 

C_DEPS += \
./unqlite/unqlite.d 


# Each subdirectory must supply rules for building sources it contributes
unqlite/%.o: ../unqlite/%.c
	@echo 'Building file: $<'
	@echo 'Invoking: GCC C Compiler'
	gcc -O0 -g3 -Wall -c -fmessage-length=0 -std=c++11 -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@:%.o=%.d)" -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '



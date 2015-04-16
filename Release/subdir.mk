################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
CPP_SRCS += \
../CFrame.cpp \
../CMain.cpp \
../CMcast.cpp \
../GzipProcess.cpp \
../JsonMsg.cpp \
../reciver.cpp \
../sender.cpp 

OBJS += \
./CFrame.o \
./CMain.o \
./CMcast.o \
./GzipProcess.o \
./JsonMsg.o \
./reciver.o \
./sender.o 

CPP_DEPS += \
./CFrame.d \
./CMain.d \
./CMcast.d \
./GzipProcess.d \
./JsonMsg.d \
./reciver.d \
./sender.d 


# Each subdirectory must supply rules for building sources it contributes
%.o: ../%.cpp
	@echo 'Building file: $<'
	@echo 'Invoking: GCC C++ Compiler'
	g++ -I/home/zlx/boost_1_55_0 -O3 -Wall -c -fmessage-length=0  -std=c++11 -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@:%.o=%.d)" -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '



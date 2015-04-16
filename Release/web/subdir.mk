################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
CPP_SRCS += \
../web/CRegex.cpp \
../web/ParserProcess.cpp \
../web/SeessionID.cpp \
../web/connection.cpp \
../web/connection_manager.cpp \
../web/io_service_pool.cpp \
../web/main.cpp \
../web/md5.cpp \
../web/mime_types.cpp \
../web/reply.cpp \
../web/request_handler.cpp \
../web/request_parser.cpp \
../web/server.cpp 

C_SRCS += \
../web/md5coll.c 

OBJS += \
./web/CRegex.o \
./web/ParserProcess.o \
./web/SeessionID.o \
./web/connection.o \
./web/connection_manager.o \
./web/io_service_pool.o \
./web/main.o \
./web/md5.o \
./web/md5coll.o \
./web/mime_types.o \
./web/reply.o \
./web/request_handler.o \
./web/request_parser.o \
./web/server.o 

C_DEPS += \
./web/md5coll.d 

CPP_DEPS += \
./web/CRegex.d \
./web/ParserProcess.d \
./web/SeessionID.d \
./web/connection.d \
./web/connection_manager.d \
./web/io_service_pool.d \
./web/main.d \
./web/md5.d \
./web/mime_types.d \
./web/reply.d \
./web/request_handler.d \
./web/request_parser.d \
./web/server.d 


# Each subdirectory must supply rules for building sources it contributes
web/%.o: ../web/%.cpp
	@echo 'Building file: $<'
	@echo 'Invoking: GCC C++ Compiler'
	g++ -I/home/zlx/boost_1_55_0 -O3 -Wall -c -fmessage-length=0  -std=c++11 -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@:%.o=%.d)" -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '

web/%.o: ../web/%.c
	@echo 'Building file: $<'
	@echo 'Invoking: GCC C Compiler'
	gcc -O3 -Wall -c -fmessage-length=0 -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@:%.o=%.d)" -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '



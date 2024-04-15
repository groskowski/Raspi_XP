#include "Reel.h"
#include <pigpio.h>
#include <stdexcept>
#include <chrono>
#include <thread>


Reel::Reel(int bus) : m_bus(bus), m_handle(-1) {
	
	m_address = DEV_ADD;
	
    // Initialize pigpio library
    if (gpioInitialise() < 0) {
        throw std::runtime_error("Failed to initialize pigpio.");
    }
	
	// Initialize pins:
	// Set pins as outputs and set to trigger B
    gpioSetMode(PIN_EN_REEL, PI_OUTPUT);
    gpioSetMode(PIN_EN_33V, PI_OUTPUT);
	gpioSetMode(PIN_EN_LATCH, PI_OUTPUT);
	gpioSetMode(PIN_LATCH_A, PI_OUTPUT);
	gpioSetMode(PIN_LATCH_B, PI_OUTPUT);
	gpioSetMode(PIN_LATCH_C, PI_OUTPUT);
	
	gpioWrite(PIN_EN_LATCH, 0);
	gpioWrite(PIN_LATCH_A, 0);
	gpioWrite(PIN_LATCH_B, 0);
	gpioWrite(PIN_LATCH_C, 0);
	
	enable();
	std::this_thread::sleep_for(std::chrono::milliseconds(500));
	
    // Open I2C device
    m_handle = i2cOpen(m_bus, m_address, 0);
    if (m_handle < 0) {
        gpioTerminate();
        throw std::runtime_error("Failed to open I2C device.");
    }
	
	
	// Update position from stored memory
	readFilePosition();
}

Reel::~Reel() {
    if (m_handle >= 0) {
        i2cClose(m_handle);
    }
	//disable();
    gpioTerminate();
	
	storeFilePosition();
}

uint16_t Reel::readRegister(uint8_t reg) {
	
	// Write a command and a value to the device
	int result = i2cWriteByte(m_handle, reg);
	if (result != 0) {
		fprintf(stderr, "Failed to write to the I2C device\n");
		throw std::runtime_error("Failed to write to I2C device.");
	}
	
	uint16_t data_o = 0;
    result = i2cReadByte(m_handle);
    if (result < 0) {
        throw std::runtime_error("Failed to read from I2C device.");
    }
	data_o = result;
	
	result = i2cReadByte(m_handle);
    if (result < 0) {
        throw std::runtime_error("Failed to read from I2C device.");
    }
	data_o |= result << 8;
	
	
	return data_o;
}

void Reel::writeRegister(uint8_t reg, uint16_t value) {
	char data[3];
	data[0] = reg;
	data[1] = (value>>8) & 0xFF;
	data[2] = value & 0xFF;

	// Write a command and a value to the device
	int result = i2cWriteDevice(m_handle, data, 3);
	if (result != 0) {
		fprintf(stderr, "Failed to write to the I2C device\n");
		throw std::runtime_error("Failed to write to I2C device.");
	}
}


// Turns power on to control, this will also turn the 3.3 VP supply on to enable i2c
void Reel::enable(){
	gpioWrite(PIN_EN_REEL, 1);
    gpioWrite(PIN_EN_33V, 1);
}

// Turns power off to the reel only
void Reel::disable(){
	gpioWrite(PIN_EN_REEL, 0);
}

// Control functions

void Reel::setSpeed(int16_t value){	//
	writeRegister(I2C_SET_PWR, value);
}


void Reel::setAcceleration(int16_t value){
	writeRegister(I2C_SET_ACC, value);
}


void Reel::forceSpeed(int16_t value){
	writeRegister(I2C_FORCE_PWR, value);
}


void Reel::setPosition(int32_t value){
	value -= stored_position;
	value += offset_position;
	
	setRawPosition(value);
}


int32_t Reel::getPosition(){
	int32_t return_data = getRawPosition();
	return_data -= offset_position;
	return_data += stored_position;
	return return_data;
}

void Reel::definePosition(int32_t value){
	stored_position = value;
	storeFilePosition();
	offset_position = getRawPosition();
}


void Reel::cutRope(){
	writeRegister(I2C_CUT, 0x00);
}


Status Reel::getStatus(){
	return Status::OK;
}

void Reel::saveStatus(){
	std::this_thread::sleep_for(std::chrono::milliseconds(100));
}


int32_t Reel::getRawPosition(){
	int32_t return_data = 0;
	return_data = static_cast<int16_t>(readRegister(I2C_READ_POS_L));
	//return_data |= readRegister(I2C_READ_POS_H) << 16;
	return return_data;
}

void Reel::setRawPosition(int32_t value){
	writeRegister(I2C_SET_POS_H, (uint16_t) (value>>16) & 0xFFFF);
	std::this_thread::sleep_for(std::chrono::milliseconds(10));
	writeRegister(I2C_SET_POS_L, (uint16_t) value & 0xFFFF);
}

int Reel::readFilePosition(){
	FILE *file;
	file = fopen("/home/pi/Sensors/ReelLib/position.dat", "r");
    if (file == NULL) {
		fclose(file);
        fprintf(stderr, "Failed to open file for reading\n");
        return 1;
    }
	if (fscanf(file, "%d", &stored_position) != 1) {
        fclose(file); // Important to close the file before returning
        fprintf(stderr, "Failed to read value from file\n");
        return 1;
    }

    // Step 3: Close the file
    fclose(file);
	
	fprintf(stderr, "Read position %d\n", stored_position);
	
	// Calculate offset
	offset_position = getRawPosition();
}


int Reel::storeFilePosition(){
	FILE *file;
	// Step 5: Open the file for writing
    file = fopen("/home/pi/Sensors/ReelLib/position.dat", "w");
    if (file == NULL) {
		fclose(file);
        fprintf(stderr, "Failed to open file for writing\n");
        return 1;
    }
	// Step 6: Write the updated value
    fprintf(file, "%d\n", stored_position);

    // Step 7: Close the file
    fclose(file);
}

void Reel::releaseLatches(){
	gpioWrite(PIN_EN_LATCH, 1);
	std::this_thread::sleep_for(std::chrono::milliseconds(100));
	
	gpioWrite(PIN_LATCH_A, 1);
	std::this_thread::sleep_for(std::chrono::milliseconds(250));
	
	gpioWrite(PIN_LATCH_B, 1);
	std::this_thread::sleep_for(std::chrono::milliseconds(250));
	
	gpioWrite(PIN_LATCH_C, 1);
	std::this_thread::sleep_for(std::chrono::milliseconds(250));
}

void Reel::releaseLatch(Latch latch){
	gpioWrite(PIN_EN_LATCH, 1);
	std::this_thread::sleep_for(std::chrono::milliseconds(100));
	
	switch(latch){
		case Latch::Latch_A:
			gpioWrite(PIN_LATCH_A, 1);
			break;
		case Latch::Latch_B:
			gpioWrite(PIN_LATCH_B, 1);
			break;
		case Latch::Latch_C:
			gpioWrite(PIN_LATCH_C, 1);
			break;
	}
	
	std::this_thread::sleep_for(std::chrono::milliseconds(500));
	
	gpioWrite(PIN_LATCH_A, 0);
	gpioWrite(PIN_LATCH_B, 0);
	gpioWrite(PIN_LATCH_C, 0);
}
#ifndef REEL_H
#define REEL_H

#include <cstdint> // for uint8_t
#include <stdio.h>
#include <stdlib.h>

#define DEV_ADD     0x02

#define I2C_STATUS      0x80 + 0x01
#define I2C_DEF_POS_H   0x08        // 2 bytes
#define I2C_DEF_POS_L   0x0A        // 2 bytes
#define I2C_READ_POS_H  0x80 + 0x10        // 2 bytes
#define I2C_SET_POS_H   0x10        // 2 bytes
#define I2C_READ_POS_L  0x80 + 0x12        // 2 bytes
#define I2C_SET_POS_L   0x12        // 2 bytes
#define I2C_Read_ACC    0x80 + 0x14        // 2 bytes
#define I2C_SET_ACC     0x14        // 2 bytes
#define I2C_SET_PWR     0x16        // 2 bytes
#define I2C_FORCE_PWR   0x18        // 2 bytes
#define I2C_CUT         0x1A        // 1 byte

#define PIN_EN_LATCH	22

#define PIN_LATCH_A		26
#define PIN_LATCH_B		5
#define PIN_LATCH_C		25

#define PIN_EN_REEL		23
#define PIN_EN_33V		13



#define I2C_SET_PWR     	0x16

// Define an enumeration for device status
enum class Status {
	OK,           // No error, operation successful
	InitFailed,   // Initialization of the device or library failed
	OpenFailed,   // Failed to open device
	ReadFailed,   // Failed to read from device
	WriteFailed,  // Failed to write to device
	UnknownError  // An unspecified error occurred
};

// Define an enumeration for device status
enum class Latch {
	Latch_A,           // No error, operation successful
	Latch_B,   // Initialization of the device or library failed
	Latch_C   // Failed to open device
};

class Reel {
public:
    Reel(int bus);
    ~Reel();
	
	// Turns power on to control, this will also turn the 3.3 VP supply on to enable i2c
	void enable();
	
	// Turns power off to the reel only
	void disable();
	
	// Control functions
	
	void setSpeed(int16_t value); //-2000 - +2000
	void setAcceleration(int16_t value);
	void forceSpeed(int16_t value);
	
	void setPosition(int32_t value);
	int32_t getPosition();
	void definePosition(int32_t value);
	
	void cutRope();
	
	void saveStatus();
	
	Status getStatus();
	
	void releaseLatches();
	
	void releaseLatch(Latch latch);

private:
    int m_bus;
    int m_address;
    int m_handle;
	int stored_position;
	int offset_position;
	
	int readFilePosition();
	int storeFilePosition();
	
	void setRawPosition(int32_t value);
	
	int32_t getRawPosition();
	
	// Reads a word of data from register "reg"
    uint16_t readRegister(uint8_t reg);
	
	// Writes a word of data to register "reg"
    void writeRegister(uint8_t reg, uint16_t value);
};

#endif // MYI2CDEVICE_H

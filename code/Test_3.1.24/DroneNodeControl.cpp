#include <pigpio.h>
#include <stdio.h>
#include <signal.h>
#include <unistd.h>
#include <fstream>
#include <cstdint>
#include <vector>

// Radio Drivers
#include <RHGenericDriver.h>
#include <RH_RF95.h>

// Managers
#include <RHDatagram.h>
#include <RHReliableDatagram.h>
#include <RHRouter.h>

// TCP/IP Port Socket
#include <cstring>
#include <cstdlib>
#include <iostream>
#undef htonl // Conflict between macros
#undef htons // defined in the RadioHead and NetiNet
#undef ntohl
#undef ntohs
#include <netinet/in.h>
#include <sys/socket.h>

// David's Reel Library
#include <Reel.h>
#include <chrono>
#include <thread>

// Function Definitions
void sig_handler(int sig);

// Pin definitions
// LoRa Module Pins
#define RFM95_CS_PIN 7   // GPIO07 for Chip Select
#define RFM95_IRQ_PIN 21 // GPIO21 for Interrupt Request
#define RFM95_RST_PIN 6  // GPIO06 for Reset Pin

// Power Pins on Board v4.2
#define BOARD_PWM_PIN 13
#define I2C_PWM_PIN 22

// Latch Pins
#define LA_PIN 26 // Latch A
#define LB_PIN 5  // Latch B
#define LC_PIN 25 // Latch C

// Address Change Depending on Node #
#define RF_ID 20

// RFM95 Configuration
#define RFM95_FREQUENCY 434.00
#define RFM95_TXPOWER 20

// Special Flags
uint8_t CHUNK_FLAG = 118;

// Radio Driver
RH_RF95 rf95(RFM95_CS_PIN, RFM95_IRQ_PIN);

// Radio Manager
RHRouter manager(rf95, RF_ID);

void error(const char *msg) // Error Value for TCP/IP Method
{
    perror(msg);
    exit(1);
}

// Flag for Ctrl-C
int flag = 0;

void sig_handler(int sig) // Program Termination
{
    flag = 1;
}

// Main Function
int main(int argc, const char *argv[])
{
    if (gpioInitialise() < 0)
    {
        printf("\npiGPIO Started Properly\n");
        return 1;
    }

    // Powers the Board v4.2
    gpioSetMode(BOARD_PWM_PIN, PI_OUTPUT);
    gpioWrite(BOARD_PWM_PIN, 1);
    printf("Board v4.2 is Powered\n");

    // Needs to be called after piGPIO Inits
    Reel reel_dev(1);

    // Latch Pins Start Low
    gpioSetMode(I2C_PWM_PIN, PI_OUTPUT);
    gpioWrite(I2C_PWM_PIN, 0);

    gpioSetMode(LA_PIN, PI_OUTPUT);
    gpioWrite(LA_PIN, 0);

    gpioSetMode(LB_PIN, PI_OUTPUT);
    gpioWrite(LB_PIN, 0);

    gpioSetMode(LC_PIN, PI_OUTPUT);
    gpioWrite(LC_PIN, 0);

    if (!rf95.init())
    {
        fprintf(stderr, "RF95 module init failed, Please verify wiring/module\n");
        return 1;
    }

    if (!manager.init())
    {
        fprintf(stderr, "Manager init failed, Please verify wiring/module\n");
        return 1;
    }

    printf("CS on GPIO %d\n", (uint8_t)RFM95_CS_PIN);
    printf("IRQ on GPIO %d\n", (uint8_t)RFM95_IRQ_PIN);
    printf("My ID: %d\n", RF_ID);

    gpioSetSignalFunc(2, sig_handler); // 2 is SIGINT. Ctrl+C will cause signal.

    // Configure RF95 Module
    rf95.setTxPower(RFM95_TXPOWER, false);
    rf95.setFrequency(RFM95_FREQUENCY);
    rf95.setModemConfig(RH_RF95::Bw125Cr48Sf4096);
    printf("%d\n", rf95.maxMessageLength());
    // rf95.setPromiscuous(true);
    rf95.setThisAddress(RF_ID);
    rf95.setHeaderFrom(RF_ID);
    rf95.setModeRx();

    // Static Routes
    // Node 20 Routes
    manager.addRouteTo(1, 1); // To get to 1, send to 1

    printf("RF95 Started on Frequency:%3.2fMHz\n", RFM95_FREQUENCY);

    while (!flag)
    {

        uint8_t buf[RH_RF95_MAX_MESSAGE_LEN];
        uint8_t len = sizeof(buf);
        uint8_t from, to, id, flags;

        if (manager.recvfromAckTimeout(buf, &len, 10000, &from, &to, &id, &flags))
        {
            printf("Received message from Node#%d to Node#%d: %s\n", from, to, (char *)buf);     // debugging
            printf("Received message with RSSI: %d SNR: %d\n", rf95.lastRssi(), rf95.lastSNR()); // debugging
            printf("Flag: %u ID:%u \n", flags, id);                                              // debugging

            if (strncmp((char *)buf, "A", 1) == 0)
            {

            }
            if (strncmp((char *)buf, "B", 1) == 0)
            {

            }
            if (strncmp((char *)buf, "reboot", 6) == 0)
            {

            }
            if (strncmp((char *)buf, "zpos", 4) == 0)
            {

            }
            if (strncmp((char *)buf, "zpower", 6) == 0)
            {

            }
            if (strncmp((char *)buf, "cut", 3) == 0)
            {

            }
        }
    }
}

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
// #define I2C_PWM_PIN 22

// Latch Pins
// #define LA_PIN 26 // Latch A
// #define LB_PIN 5  // Latch B
// #define LC_PIN 25 // Latch C

// Address Change Depending on Node #
#define RF_ID 6

// RFM95 Configuration
#define RFM95_FREQUENCY 434.00
#define RFM95_TXPOWER 20

// Special Flags
uint8_t CHUNK_FLAG = 118;
uint8_t ZPOS_FLAG = 119;
uint8_t DEF_ZPOS_FLAG = 121;
uint8_t ZPOW_FLAG = 120;

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

// Function to read a file into a byte array (vector)
std::vector<uint8_t> readFileToByteArray(const std::string &filePath)
{
    std::ifstream file(filePath, std::ios::binary | std::ios::ate);
    if (!file.is_open())
    {
        throw std::runtime_error("Failed to open file");
    }

    std::streamsize size = file.tellg();
    file.seekg(0, std::ios::beg);

    std::vector<uint8_t> buffer(size);
    if (!file.read(reinterpret_cast<char *>(buffer.data()), size))
    {
        throw std::runtime_error("Failed to read the file");
    }

    return buffer;
}

// Function to send the file in chunks
size_t SplitFileInChunks(const std::string &filePath, size_t maxPayloadSize)
{
    size_t totalChunks = 0; // Initialize totalChunks to ensure it's accessible
    try
    {
        std::vector<uint8_t> fileData = readFileToByteArray(filePath);
        size_t fileSize = fileData.size();

        // Assuming each packet can have a 1-byte header for the sequence number
        size_t maxDataSize = maxPayloadSize - 1;
        totalChunks = (fileSize + maxDataSize - 1) / maxDataSize;

        std::cout << "Total chunks needed: " << totalChunks << std::endl;

        for (size_t chunkIndex = 0; chunkIndex < totalChunks; ++chunkIndex)
        {
            size_t chunkStart = chunkIndex * maxDataSize;
            size_t chunkEnd = std::min(chunkStart + maxDataSize, fileSize);

            // Generate chunk file name
            std::string chunkFileName = filePath + "_chunk_" + std::to_string(chunkIndex + 1) + ".bin";
            std::ofstream chunkFile(chunkFileName, std::ios::binary);

            if (!chunkFile.is_open())
            {
                throw std::runtime_error("Failed to open chunk file for writing");
            }

            std::cout << "Writing Chunk " << (chunkIndex + 1) << " to " << chunkFileName << std::endl;
            // Write the chunk data to the file
            chunkFile.write(reinterpret_cast<const char *>(&fileData[chunkStart]), chunkEnd - chunkStart);
        }

        std::cout << "File has been parsed into " << totalChunks << " chunks." << std::endl;
    }
    catch (const std::exception &e)
    {
        std::cerr << "Error: " << e.what() << std::endl;
    }
    return totalChunks; // Return the total number of chunks
}

std::vector<uint8_t> readChunkData(const std::string &baseFilePath, int chunkNumber)
{
    // Construct the filename for the chunk
    std::string chunkFileName = baseFilePath + "_chunk_" + std::to_string(chunkNumber) + ".bin";

    try
    {
        // Use readFileToByteArray to read the chunk file into a byte array
        std::vector<uint8_t> chunkData = readFileToByteArray(chunkFileName);
        return chunkData; // Return the chunkData variable
    }
    catch (const std::exception &e)
    {
        std::cerr << "Error: " << e.what() << std::endl;
        throw;
    }
}

// Main Function
int main(int argc, const char *argv[])
{

    int sockfd, newsockfd, portno;
    socklen_t clilen;
    char buffer[256]; // Buffer for socket communication
    struct sockaddr_in serv_addr, cli_addr;
    int n;

    // Create a socket
    sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd < 0)
        error("ERROR opening socket");

    // Set socket to allow reuse of local addresses
    int optval = 1;
    if (setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, &optval, sizeof(optval)) < 0)
        error("ERROR setting socket option");

    // Address structure
    bzero((char *)&serv_addr, sizeof(serv_addr));
    portno = 18200; // Example port

    // Setup the host_addr structure for use in bind call
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_addr.s_addr = INADDR_ANY;
    serv_addr.sin_port = htons(portno);

    // Bind the socket to the current IP address on port, portno
    if (bind(sockfd, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0)
        error("ERROR on binding");

    // Listen on the socket for connections
    listen(sockfd, 5);
    clilen = sizeof(cli_addr);

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
    // gpioSetMode(I2C_PWM_PIN, PI_OUTPUT);
    // gpioWrite(I2C_PWM_PIN, 0);

    // gpioSetMode(LA_PIN, PI_OUTPUT);
    // gpioWrite(LA_PIN, 0);

    // gpioSetMode(LB_PIN, PI_OUTPUT);
    // gpioWrite(LB_PIN, 0);

    // gpioSetMode(LC_PIN, PI_OUTPUT);
    // gpioWrite(LC_PIN, 0);

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
    // Node 6 Routes
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

            // Adding in the different command responses
            // when the message is 'GPS'
            if (strncmp((char *)buf, "data", 4) == 0)
            {

                printf("Sensor Data detected\n"); // debugging
                uint8_t *sdata = NULL;

                int returnCode = system("/usr/bin/python3 /home/pi/Sensors/Sensor-Client.py > /dev/null 2>&1 &");

                // Accept a connection from a client
                newsockfd = accept(sockfd, (struct sockaddr *)&cli_addr, &clilen);
                if (newsockfd < 0)
                    error("ERROR on accept");

                bzero(buffer, 256);

                // Read the message from the client
                n = read(newsockfd, buffer, 255);
                if (n < 0)
                    error("ERROR reading from socket");
                printf("Here is the message: %s\n", buffer); // debugging

                // Dynamically allocate memory for sdata based on the size of the data read
                sdata = (uint8_t *)malloc(n); // This is an assignment, not a declaration
                if (sdata == NULL)
                {
                    error("ERROR allocating memory for sdata");
                }

                // Copy the data from buffer to sdata
                memcpy(sdata, buffer, n);

                close(newsockfd);

                unsigned long ts = millis();
                uint16_t wait = 300;
                while ((millis() - ts) <= wait)
                    ;
                YIELD;

                switch (manager.sendtoWait(sdata, n, from))
                {
                case RH_ROUTER_ERROR_NONE:
                    printf("Success: No error. Dest: %d\n", from);
                    printf("My message was: %s\n", sdata);
                    break;
                case RH_ROUTER_ERROR_INVALID_LENGTH:
                    printf("Error: Invalid length. Dest: %d\n", from);
                    break;
                case RH_ROUTER_ERROR_NO_ROUTE:
                    printf("Error: No route to destination. Dest: %d\n", from);
                    break;
                case RH_ROUTER_ERROR_TIMEOUT:
                    printf("Error: Operation timed out. Dest: %d\n", from);
                    break;
                case RH_ROUTER_ERROR_NO_REPLY:
                    printf("Error: No reply received. Dest: %d\n", from);
                    break;
                case RH_ROUTER_ERROR_UNABLE_TO_DELIVER:
                    printf("Error: Unable to deliver the message. Dest: %d\n", from);
                    break;
                default:
                    printf("Error: Unknown error code %d\n", manager.sendtoWait(sdata, sizeof(sdata), from));
                }

                free(sdata);
                sdata = NULL;
            }
            else if (strncmp((char *)buf, "latch", 5) == 0)
            {
                reel_dev.releaseLatches();
            }
            else if (strncmp((char *)buf, "photo", 5) == 0)
            {
                printf("Photo detected\n"); // debugging
                const std::string filePath = "/home/pi/pictures/compressedphoto/test.jpg";
                // size_t maxPayloadSize = rf95.maxMessageLength(); //for some reason this is too large of a data amount, eve though it shouldnt be
                size_t maxPayloadSize = 200;

                size_t totalChunks = SplitFileInChunks(filePath, maxPayloadSize);

                // Convert totalChunks to a byte array
                uint8_t totalChunksArray[sizeof(totalChunks)];
                memcpy(totalChunksArray, &totalChunks, sizeof(totalChunks));

                unsigned long ts = millis();
                uint16_t wait = 300;
                while ((millis() - ts) <= wait)
                    ;
                YIELD;

                switch (manager.sendtoWait(totalChunksArray, sizeof(totalChunksArray), from))
                {
                case RH_ROUTER_ERROR_NONE:
                    printf("Success: No error. Dest: %d\n", from);
                    printf("My message was: %lu\n", (unsigned long)totalChunks);
                    break;
                case RH_ROUTER_ERROR_INVALID_LENGTH:
                    printf("Error: Invalid length. Dest: %d\n", from);
                    break;
                case RH_ROUTER_ERROR_NO_ROUTE:
                    printf("Error: No route to destination. Dest: %d\n", from);
                    break;
                case RH_ROUTER_ERROR_TIMEOUT:
                    printf("Error: Operation timed out. Dest: %d\n", from);
                    break;
                case RH_ROUTER_ERROR_NO_REPLY:
                    printf("Error: No reply received. Dest: %d\n", from);
                    break;
                case RH_ROUTER_ERROR_UNABLE_TO_DELIVER:
                    printf("Error: Unable to deliver the message. Dest: %d\n", from);
                    break;
                default:
                    printf("Error: Unknown error code %d\n", manager.sendtoWait(totalChunksArray, sizeof(totalChunksArray), from));
                }
            }
            else if (strncmp((char *)buf, "chunk ", 6) == 0) // Check if the message starts with "chunk "
            {
                int chunkNumber;
                const std::string filePath = "/home/pi/pictures/compressedphoto/test.jpg";
                if (sscanf((char *)buf + 6, "%d", &chunkNumber) == 1) // Read the number following "chunk"
                {
                    printf("Message received for chunk %d\n", chunkNumber);                // debugging
                    std::vector<uint8_t> chunkData = readChunkData(filePath, chunkNumber); // save the chunk data from a given .bin file as a byte array
                    // printf("readChunkData finished\n"); //debugging
                    // uint8_t test[] = "test";//debugging
                    const uint8_t *dataPtr = chunkData.data(); // Convert std::vector<uint8_t> to raw pointer for sending
                    size_t dataSize = chunkData.size();

                    unsigned long ts = millis();
                    uint16_t wait = 300;
                    while ((millis() - ts) <= wait)
                        ;
                    YIELD;

                    switch (manager.sendtoWait(const_cast<uint8_t *>(dataPtr), dataSize, from, CHUNK_FLAG))
                    // switch (manager.sendtoWait(test, sizeof(test), from, CHUNK_FLAG)) //debugging
                    {
                    case RH_ROUTER_ERROR_NONE:
                        printf("Success: No error. Dest: %d\n", from);
                        break;
                    case RH_ROUTER_ERROR_INVALID_LENGTH:
                        printf("Error: Invalid length. Dest: %d\n", from);
                        // printf("%d\n", dataSize);
                        break;
                    case RH_ROUTER_ERROR_NO_ROUTE:
                        printf("Error: No route to destination. Dest: %d\n", from);
                        break;
                    case RH_ROUTER_ERROR_TIMEOUT:
                        printf("Error: Operation timed out. Dest: %d\n", from);
                        break;
                    case RH_ROUTER_ERROR_NO_REPLY:
                        printf("Error: No reply received. Dest: %d\n", from);
                        break;
                    case RH_ROUTER_ERROR_UNABLE_TO_DELIVER:
                        printf("Error: Unable to deliver the message. Dest: %d\n", from);
                        break;
                    default:
                        printf("Error: Unknown error. Dest: %d\n", from);
                        // It might be helpful to log the actual return value for debugging
                        int errorCode = manager.sendtoWait(const_cast<uint8_t *>(dataPtr), dataSize, from);
                        // int errorCode = manager.sendtoWait(test, sizeof(test), from, CHUNK_FLAG);
                        printf("Error code: %d\n", errorCode);
                    }
                }
                else
                {
                    printf("Received a 'chunk' message, but the format is incorrect.\n");
                }
            }
            else if (strncmp((char *)buf, "day", 3) == 0) // Check if the message starts with "day"
            {
                printf("DAY\n");
                std::cout << "Executing shell script..." << std::endl;

                // Replace `take_photo.sh` with the actual name of your shell script
                int result = system("\"/home/pi/day.sh\"");

                if (result == 0)
                {
                    std::cout << "Shell script executed successfully." << std::endl;
                }
                else
                {
                    std::cout << "Shell script execution failed." << std::endl;
                }
            }
            else if (strncmp((char *)buf, "night", 5) == 0) // Check if the message starts with "night"
            {
                printf("NIGHT\n");
                std::cout << "Executing shell script..." << std::endl;

                // Replace `take_photo.sh` with the actual name of your shell script
                int result = system("\"/home/pi/night.sh\"");

                if (result == 0)
                {
                    std::cout << "Shell script executed successfully." << std::endl;
                }
                else
                {
                    std::cout << "Shell script execution failed." << std::endl;
                }
            }
            else if (strncmp((char *)buf, "setup", 5) == 0) // Check if the message starts with "setup"
            {
                printf("SETUP\n");
                int returnCode = system("/home/pi/takepicture");
            }
            else if (strncmp((char *)buf, "stop", 5) == 0) // Check if the message starts with "stop"
            {
                printf("STOP\n");
                int16_t speed = 0;
                reel_dev.setSpeed(speed);
                fprintf(stderr, "Speed set to: %d\n", speed);
            }
            else if (strncmp((char *)buf, "audio", 5) == 0) // Check if the message starts with "audio"
            {
                printf("AUDIO\n"); // debugging
                // Run the Python script in the background, redirecting output to /dev/null
                int returnCode = system("/usr/bin/python3 /home/pi/Sensors/MicroMoth.py > /dev/null 2>&1 &");
            }

            else if (strncmp((char *)buf, "reboot", 6) == 0) // Check if the message starts with "reboot"
            {
                printf("REBOOT\n");

                unsigned long ts = millis();
                uint16_t wait = 300;
                while ((millis() - ts) <= wait)
                    ;
                YIELD;

                int returnCode = system("sudo /sbin/reboot");
            }
            else if (flags == ZPOS_FLAG) // Check if the ZPOS flag is set
            {
                printf("ZPOS\n");

                // Correctly deserialize the integer value from the binary data in 'buf'
                int16_t zpos = (buf[0] << 8) | buf[1];
                printf("Received Position: %d\n", zpos); // Confirming the deserialized value

                // Check if the position is within the valid range
                if (zpos >= -10000 && zpos <= 10000)
                {
                    printf("Valid position: %d\n", zpos); // Debugging
                    reel_dev.setPosition(zpos);

                    // Optional: Loop to print the current position at 1-second intervals
                    for (int i = 0; i < 6; ++i)
                    {
                        std::this_thread::sleep_for(std::chrono::milliseconds(1000));
                        fprintf(stderr, "Current position: %d\n", reel_dev.getPosition());
                    }
                }
                else
                {
                    fprintf(stderr, "Error: Position %d is out of range (-10000 to 10000).\n", zpos);
                }
            }
            else if (flags == DEF_ZPOS_FLAG) // Check if the ZPOS flag is set
            {
                printf("DEF_ZPOS\n");

                // Correctly deserialize the integer value from the binary data in 'buf'
                int16_t defzpos = (buf[0] << 8) | buf[1];
                printf("Received Position: %d\n", defzpos); // Confirming the deserialized value

                // Check if the position is within the valid range
                if (defzpos >= -10000 && defzpos <= 10000)
                {
                    printf("Valid position: %d\n", defzpos); // Debugging
                    reel_dev.definePosition(defzpos);

                    // Optional: Loop to print the current position at 1-second intervals
                    for (int i = 0; i < 6; ++i)
                    {
                        std::this_thread::sleep_for(std::chrono::milliseconds(1000));
                        fprintf(stderr, "Current position: %d\n", reel_dev.getPosition());
                    }
                }
                else
                {
                    fprintf(stderr, "Error: Position %d is out of range (-10000 to 10000).\n", defzpos);
                }
            }
            else if (flags == ZPOW_FLAG) // Check if the ZPOW flag is set
            {
                printf("ZPOW\n");

                // Correctly deserialize the integer value from the binary data in 'buf'
                int16_t zpow = (buf[0] << 8) | buf[1];
                printf("Received Position: %d\n", zpow); // Confirming the deserialized value

                // Optionally, check if the speed is within a specific range if required
                if (zpow >= -10000 && zpow <= 10000) // Adjust range as needed
                {
                    reel_dev.setSpeed(zpow);

                    // Feedback or additional actions after setting the speed
                    fprintf(stderr, "Speed set to: %d\n", zpow);
                }
                else
                {
                    fprintf(stderr, "Error: Speed %d is out of range (-10000 to 10000).\n", zpow);
                }
            }

            else if (strncmp((char *)buf, "cut", 3) == 0) // Check if the message starts with "cut"
            {
                printf("CUT\n");
                reel_dev.cutRope();
                std::this_thread::sleep_for(std::chrono::milliseconds(2000));
            }
            gpioDelay(500);
        }
        printf("Timeout\n"); // debugging
    }
    // Cleanup and exit
    close(sockfd);
    gpioTerminate();
    return 0;
}
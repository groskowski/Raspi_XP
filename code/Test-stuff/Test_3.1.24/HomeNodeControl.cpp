#include <pigpio.h>
#include <stdio.h>
#include <signal.h>
#include <unistd.h>
#include <fstream>
#include <cstdint>
#include <vector>
#include <filesystem>

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

// Function Definitions
void sig_handler(int sig);

// Pin definitions
#define RFM95_CS_PIN 8   // GPIO08 for Chip Select
#define RFM95_IRQ_PIN 17 // GPIO17 for Interrupt Request
#define RFM95_RST_PIN 22 // GPIO22 for Reset Pin

// Address
#define RF_ID 1

// RFM95 Configuration
#define RFM95_FREQUENCY 434.00
#define RFM95_TXPOWER 20

// Special Flags
uint8_t CHUNK_FLAG = 118;
uint8_t ZPOS_FLAG = 119;
uint8_t ZPOW_FLAG = 120;
uint8_t DEF_ZPOS_FLAG = 121;

// Singleton instance of the radio driver
RH_RF95 rf95(RFM95_CS_PIN, RFM95_IRQ_PIN);

// Singleton instance of the radio driver
RHRouter manager(rf95, RF_ID);

// Flag for Ctrl-C
int flag = 0;

void sig_handler(int sig)
{
  flag = 1;
}

bool CombineFiles(const std::string &inputDirectoryPath, const std::string &outputDirectoryPath, const std::string &outputFileName)
{
  std::ofstream outputFile(outputDirectoryPath + "/" + outputFileName, std::ios::binary | std::ios::out);
  if (!outputFile.is_open())
  {
    std::cerr << "Failed to open output file for writing." << std::endl;
    return false;
  }

  size_t chunkIndex = 1;
  while (true)
  {
    std::string chunkFileName = inputDirectoryPath + "/chunk_" + std::to_string(chunkIndex) + ".bin";
    std::ifstream chunkFile(chunkFileName, std::ios::binary | std::ios::in);

    if (!chunkFile.is_open())
    {
      std::cout << "No more files to combine after " << chunkIndex - 1 << " chunks." << std::endl;
      break;
    }

    std::cout << "Combining " << chunkFileName << std::endl;

    outputFile << chunkFile.rdbuf();

    chunkFile.close();
    ++chunkIndex;
  }

  outputFile.close();
  return true;
}

void ClearDirectoryContents(const std::string &directoryPath)
{
  for (const auto &entry : std::filesystem::directory_iterator(directoryPath))
  {
    std::filesystem::remove_all(entry.path());
  }
}

// Number of chunks of a file
int receivedChunks = 0;

// Main Function
int main(int argc, const char *argv[])
{
  if (gpioInitialise() < 0)
  {
    printf("\npiGPIO Started Properly\n");
    return 1;
  }

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
  // Node 1 Routes
  manager.addRouteTo(6, 6); // To get to 6, send to 6
  manager.addRouteTo(3, 3);
  printf("RF95 Started on Frequency:%3.2fMHz\n", RFM95_FREQUENCY);

  while (!flag)
  {

    uint8_t dest;
    uint8_t buf[RH_RF95_MAX_MESSAGE_LEN];
    uint8_t data[] = "data";     // Sensor Data                                         // This is finished and works
    uint8_t latch[] = "latch";   // Latch Trigger                                       // This is finished and works
    uint8_t photo[] = "photo";   // Photo Transfer                                      // This is finished and works
    uint8_t test[] = "test";     // Test Message                                        // This is finished and works
    uint8_t day[] = "day";       // Start Day Timelapse                                 //
    uint8_t night[] = "night";   // Start Night Timelapse                               //
    uint8_t setup[] = "setup";   // Take 1 picture to setup reel positioning            //
    uint8_t stop[] = "stop";     // Stop current LibCam functions                       //
    uint8_t audio[] = "audio";   // Start AudioMoth                                     // This is finished and works
    uint8_t reboot[] = "reboot"; // Reboot Pi                                           //
    // uint8_t zpos[] = "zpos";      // Line Positioning                                //
    // uint8_t zpower[] = "zpower";  // Line Power                                      //
    uint8_t A[] = "A";     //
    uint8_t B[] = "B";     //
    uint8_t cut[] = "cut"; // Trigger Line Cut                                          // This is finished and works

    printf("D to send data\n L to trigger latches\n I for Imaging\n T for Test\n R to combine files downloaded from imaging\n 'audio' for AudioMoth Start\n 'reboot' to reboot the Pi\n 'zpos' for spool postion\n 'zpower' for spool power\n 'day' to start day timelapse function\n 'night' to start night timelapse function\n 'setup' to take a photo for setup\n 'stop' to set reel speed zero\n 'cut' to cut the line of a Node\n 'exit' to quit:\n");
    char input[10];
    fgets(input, sizeof(input), stdin);
    if (strncmp(input, "exit", 4) == 0)
    {
      gpioTerminate();
      exit(0);
    }
    else if (strncmp(input, "day", 3) == 0)
    {
      printf("Enter destination node ID:\n");
      char destInput[10];
      fgets(destInput, sizeof(destInput), stdin);
      dest = atoi(destInput);

      switch (manager.sendtoWait(day, sizeof(day), dest))
      {
      case RH_ROUTER_ERROR_NONE:
        printf("Success: No error. Dest: %d\n", dest);
        break;
      case RH_ROUTER_ERROR_INVALID_LENGTH:
        printf("Error: Invalid length. Dest: %d\n", dest);
        break;
      case RH_ROUTER_ERROR_NO_ROUTE:
        printf("Error: No route to destination. Dest: %d\n", dest);
        break;
      case RH_ROUTER_ERROR_TIMEOUT:
        printf("Error: Operation timed out. Dest: %d\n", dest);
        break;
      case RH_ROUTER_ERROR_NO_REPLY:
        printf("Error: No reply received. Dest: %d\n", dest);
        break;
      case RH_ROUTER_ERROR_UNABLE_TO_DELIVER:
        printf("Error: Unable to deliver the message. Dest: %d\n", dest);
        break;
      default:
        printf("Error: Unknown error code %d\n", manager.sendtoWait(day, sizeof(day), dest));
      }
    }
    else if (strncmp(input, "night", 5) == 0)
    {
      printf("Enter destination node ID:\n");
      char destInput[10];
      fgets(destInput, sizeof(destInput), stdin);
      dest = atoi(destInput);

      switch (manager.sendtoWait(night, sizeof(night), dest))
      {
      case RH_ROUTER_ERROR_NONE:
        printf("Success: No error. Dest: %d\n", dest);
        break;
      case RH_ROUTER_ERROR_INVALID_LENGTH:
        printf("Error: Invalid length. Dest: %d\n", dest);
        break;
      case RH_ROUTER_ERROR_NO_ROUTE:
        printf("Error: No route to destination. Dest: %d\n", dest);
        break;
      case RH_ROUTER_ERROR_TIMEOUT:
        printf("Error: Operation timed out. Dest: %d\n", dest);
        break;
      case RH_ROUTER_ERROR_NO_REPLY:
        printf("Error: No reply received. Dest: %d\n", dest);
        break;
      case RH_ROUTER_ERROR_UNABLE_TO_DELIVER:
        printf("Error: Unable to deliver the message. Dest: %d\n", dest);
        break;
      default:
        printf("Error: Unknown error code %d\n", manager.sendtoWait(night, sizeof(night), dest));
      }
    }
    else if (strncmp(input, "setup", 5) == 0)
    {
      printf("Enter destination node ID:\n");
      char destInput[10];
      fgets(destInput, sizeof(destInput), stdin);
      dest = atoi(destInput);

      switch (manager.sendtoWait(setup, sizeof(setup), dest))
      {
      case RH_ROUTER_ERROR_NONE:
        printf("Success: No error. Dest: %d\n", dest);
        break;
      case RH_ROUTER_ERROR_INVALID_LENGTH:
        printf("Error: Invalid length. Dest: %d\n", dest);
        break;
      case RH_ROUTER_ERROR_NO_ROUTE:
        printf("Error: No route to destination. Dest: %d\n", dest);
        break;
      case RH_ROUTER_ERROR_TIMEOUT:
        printf("Error: Operation timed out. Dest: %d\n", dest);
        break;
      case RH_ROUTER_ERROR_NO_REPLY:
        printf("Error: No reply received. Dest: %d\n", dest);
        break;
      case RH_ROUTER_ERROR_UNABLE_TO_DELIVER:
        printf("Error: Unable to deliver the message. Dest: %d\n", dest);
        break;
      default:
        printf("Error: Unknown error code %d\n", manager.sendtoWait(setup, sizeof(setup), dest));
      }
    }
    else if (strncmp(input, "stop", 4) == 0)
    {
      printf("Enter destination node ID:\n");
      char destInput[10];
      fgets(destInput, sizeof(destInput), stdin);
      dest = atoi(destInput);

      switch (manager.sendtoWait(stop, sizeof(stop), dest))
      {
      case RH_ROUTER_ERROR_NONE:
        printf("Success: No error. Dest: %d\n", dest);
        break;
      case RH_ROUTER_ERROR_INVALID_LENGTH:
        printf("Error: Invalid length. Dest: %d\n", dest);
        break;
      case RH_ROUTER_ERROR_NO_ROUTE:
        printf("Error: No route to destination. Dest: %d\n", dest);
        break;
      case RH_ROUTER_ERROR_TIMEOUT:
        printf("Error: Operation timed out. Dest: %d\n", dest);
        break;
      case RH_ROUTER_ERROR_NO_REPLY:
        printf("Error: No reply received. Dest: %d\n", dest);
        break;
      case RH_ROUTER_ERROR_UNABLE_TO_DELIVER:
        printf("Error: Unable to deliver the message. Dest: %d\n", dest);
        break;
      default:
        printf("Error: Unknown error code %d\n", manager.sendtoWait(stop, sizeof(stop), dest));
      }
    }
    else if (strncmp(input, "audio", 5) == 0)
    {
      printf("Enter destination node ID:\n");
      char destInput[10];
      fgets(destInput, sizeof(destInput), stdin);
      dest = atoi(destInput);

      switch (manager.sendtoWait(audio, sizeof(audio), dest))
      {
      case RH_ROUTER_ERROR_NONE:
        printf("Success: No error. Dest: %d\n", dest);
        break;
      case RH_ROUTER_ERROR_INVALID_LENGTH:
        printf("Error: Invalid length. Dest: %d\n", dest);
        break;
      case RH_ROUTER_ERROR_NO_ROUTE:
        printf("Error: No route to destination. Dest: %d\n", dest);
        break;
      case RH_ROUTER_ERROR_TIMEOUT:
        printf("Error: Operation timed out. Dest: %d\n", dest);
        break;
      case RH_ROUTER_ERROR_NO_REPLY:
        printf("Error: No reply received. Dest: %d\n", dest);
        break;
      case RH_ROUTER_ERROR_UNABLE_TO_DELIVER:
        printf("Error: Unable to deliver the message. Dest: %d\n", dest);
        break;
      default:
        printf("Error: Unknown error code %d\n", manager.sendtoWait(audio, sizeof(audio), dest));
      }
    }
    else if (strncmp(input, "reboot", 6) == 0)
    {
      printf("Enter destination node ID:\n");
      char destInput[10];
      fgets(destInput, sizeof(destInput), stdin);
      dest = atoi(destInput);

      switch (manager.sendtoWait(reboot, sizeof(reboot), dest))
      {
      case RH_ROUTER_ERROR_NONE:
        printf("Success: No error. Dest: %d\n", dest);
        break;
      case RH_ROUTER_ERROR_INVALID_LENGTH:
        printf("Error: Invalid length. Dest: %d\n", dest);
        break;
      case RH_ROUTER_ERROR_NO_ROUTE:
        printf("Error: No route to destination. Dest: %d\n", dest);
        break;
      case RH_ROUTER_ERROR_TIMEOUT:
        printf("Error: Operation timed out. Dest: %d\n", dest);
        break;
      case RH_ROUTER_ERROR_NO_REPLY:
        printf("Error: No reply received. Dest: %d\n", dest);
        break;
      case RH_ROUTER_ERROR_UNABLE_TO_DELIVER:
        printf("Error: Unable to deliver the message. Dest: %d\n", dest);
        break;
      default:
        printf("Error: Unknown error code %d\n", manager.sendtoWait(reboot, sizeof(reboot), dest));
      }
    }
    else if (strncmp(input, "zpos", 4) == 0) // Set Reel Position
    {
      printf("Enter destination node ID:\n");
      char destInput[10];
      fgets(destInput, sizeof(destInput), stdin);
      int dest = atoi(destInput);

      printf("Enter Reel Position Integer (-2000 to 2000):\n");
      char zposInput[10];
      fgets(zposInput, sizeof(zposInput), stdin);
      int16_t zpos = atoi(zposInput);
      printf("Sending Position: %d\n", zpos); // Validate entered position

      // Validate the range of zpos
      if (zpos < -10000 || zpos > 10000)
      {
        printf("Error: Position %d out of range.\n", zpos);
      }
      else
      {
        uint8_t buf[2]; // Buffer to hold the serialized zpos

        // Serialize the integer zpos into the buffer
        buf[0] = (zpos >> 8) & 0xFF; // High byte
        buf[1] = zpos & 0xFF;        // Low byte

        printf("Serialized Data: [%d, %d]\n", buf[0], buf[1]); // Validate serialized data

        // Send the serialized position to the specified destination
        switch (manager.sendtoWait(buf, sizeof(buf), dest, ZPOS_FLAG))
        {
        case RH_ROUTER_ERROR_NONE:
          printf("Success: No error. Dest: %d\n", dest);
          break;
        case RH_ROUTER_ERROR_INVALID_LENGTH:
          printf("Error: Invalid length. Dest: %d\n", dest);
          break;
        case RH_ROUTER_ERROR_NO_ROUTE:
          printf("Error: No route to destination. Dest: %d\n", dest);
          break;
        case RH_ROUTER_ERROR_TIMEOUT:
          printf("Error: Operation timed out. Dest: %d\n", dest);
          break;
        case RH_ROUTER_ERROR_NO_REPLY:
          printf("Error: No reply received. Dest: %d\n", dest);
          break;
        case RH_ROUTER_ERROR_UNABLE_TO_DELIVER:
          printf("Error: Unable to deliver the message. Dest: %d\n", dest);
          break;
        default:
          printf("Error: Unknown error code %d\n", manager.sendtoWait(buf, sizeof(buf), dest, ZPOS_FLAG));
        }
      }
    }
else if (strncmp(input, "defzpos", 4) == 0) // Set Reel Position
    {
      printf("Enter destination node ID:\n");
      char destInput[10];
      fgets(destInput, sizeof(destInput), stdin);
      int dest = atoi(destInput);

      printf("Enter Reel Position Integer (-2000 to 2000):\n");
      char defzposInput[10];
      fgets(defzposInput, sizeof(defzposInput), stdin);
      int16_t defzpos = atoi(defzposInput);
      printf("Sending Define Position: %d\n", defzpos); // Validate entered position

      // Validate the range of zpos
      if (defzpos < -10000 || defzpos > 10000)
      {
        printf("Error: Position %d out of range.\n", defzpos);
      }
      else
      {
        uint8_t buf[2]; // Buffer to hold the serialized zpos

        // Serialize the integer zpos into the buffer
        buf[0] = (defzpos >> 8) & 0xFF; // High byte
        buf[1] = defzpos & 0xFF;        // Low byte

        printf("Serialized Data: [%d, %d]\n", buf[0], buf[1]); // Validate serialized data

        // Send the serialized position to the specified destination
        switch (manager.sendtoWait(buf, sizeof(buf), dest, DEF_ZPOS_FLAG))
        {
        case RH_ROUTER_ERROR_NONE:
          printf("Success: No error. Dest: %d\n", dest);
          break;
        case RH_ROUTER_ERROR_INVALID_LENGTH:
          printf("Error: Invalid length. Dest: %d\n", dest);
          break;
        case RH_ROUTER_ERROR_NO_ROUTE:
          printf("Error: No route to destination. Dest: %d\n", dest);
          break;
        case RH_ROUTER_ERROR_TIMEOUT:
          printf("Error: Operation timed out. Dest: %d\n", dest);
          break;
        case RH_ROUTER_ERROR_NO_REPLY:
          printf("Error: No reply received. Dest: %d\n", dest);
          break;
        case RH_ROUTER_ERROR_UNABLE_TO_DELIVER:
          printf("Error: Unable to deliver the message. Dest: %d\n", dest);
          break;
        default:
          printf("Error: Unknown error code %d\n", manager.sendtoWait(buf, sizeof(buf), dest, DEF_ZPOS_FLAG));
        }
      }
    }
    else if (strncmp(input, "zpower", 6) == 0) // Set Reel Speed
    {
      printf("Enter destination node ID:\n");
      char destInput[10];
      fgets(destInput, sizeof(destInput), stdin);
      int dest = atoi(destInput); // Convert destination ID to integer

      printf("Enter Reel Power Integer (-2000 to 2000):\n");
      char zpowInput[10];
      fgets(zpowInput, sizeof(zpowInput), stdin);
      int16_t zpow = atoi(zpowInput);
      printf("Sending Power: %d\n", zpow); // Validate entered position

      // Validate the range of zpow
      if (zpow < -2000 || zpow > 2000)
      {
        printf("Error: Power %d out of range.\n", zpow);
      }
      else
      {
        uint8_t buf[2]; // Buffer to hold the serialized zpow

        // Serialize the integer zpow into the buffer
        buf[0] = (zpow >> 8) & 0xFF; // High byte
        buf[1] = zpow & 0xFF;        // Low byte

        printf("Serialized Data: [%d, %d]\n", buf[0], buf[1]); // Validate serialized data

        // Send the serialized position to the specified destination
        switch (manager.sendtoWait(buf, sizeof(buf), dest, ZPOW_FLAG)) // Ensure correct passing of zpower
        {
        case RH_ROUTER_ERROR_NONE:
          printf("Success: No error. Dest: %d\n", dest);
          break;
        case RH_ROUTER_ERROR_INVALID_LENGTH:
          printf("Error: Invalid length. Dest: %d\n", dest);
          break;
        case RH_ROUTER_ERROR_NO_ROUTE:
          printf("Error: No route to destination. Dest: %d\n", dest);
          break;
        case RH_ROUTER_ERROR_TIMEOUT:
          printf("Error: Operation timed out. Dest: %d\n", dest);
          break;
        case RH_ROUTER_ERROR_NO_REPLY:
          printf("Error: No reply received. Dest: %d\n", dest);
          break;
        case RH_ROUTER_ERROR_UNABLE_TO_DELIVER:
          printf("Error: Unable to deliver the message. Dest: %d\n", dest);
          break;
        default:
          printf("Error: Unknown error code %d\n", manager.sendtoWait(buf, sizeof(buf), dest, ZPOW_FLAG));
        }
      }
    }

    else if (strncmp(input, "cut", 3) == 0) // Cut the line
    {
      printf("Enter destination node ID:\n");
      char destInput[10];
      fgets(destInput, sizeof(destInput), stdin);
      dest = atoi(destInput);

      switch (manager.sendtoWait(cut, sizeof(cut), dest))
      {
      case RH_ROUTER_ERROR_NONE:
        printf("Success: No error. Dest: %d\n", dest);
        break;
      case RH_ROUTER_ERROR_INVALID_LENGTH:
        printf("Error: Invalid length. Dest: %d\n", dest);
        break;
      case RH_ROUTER_ERROR_NO_ROUTE:
        printf("Error: No route to destination. Dest: %d\n", dest);
        break;
      case RH_ROUTER_ERROR_TIMEOUT:
        printf("Error: Operation timed out. Dest: %d\n", dest);
        break;
      case RH_ROUTER_ERROR_NO_REPLY:
        printf("Error: No reply received. Dest: %d\n", dest);
        break;
      case RH_ROUTER_ERROR_UNABLE_TO_DELIVER:
        printf("Error: Unable to deliver the message. Dest: %d\n", dest);
        break;
      default:
        printf("Error: Unknown error code %d\n", manager.sendtoWait(cut, sizeof(cut), dest));
      }
    }
    else if (input[0] == 'D' || input[0] == 'd') // Sensor Data
    {

      printf("Enter destination node ID:\n");
      char destInput[10];
      fgets(destInput, sizeof(destInput), stdin);
      dest = atoi(destInput);

      switch (manager.sendtoWait(data, sizeof(data), dest))
      {
      case RH_ROUTER_ERROR_NONE:
        printf("Success: No error. Dest: %d\n", dest);
        break;
      case RH_ROUTER_ERROR_INVALID_LENGTH:
        printf("Error: Invalid length. Dest: %d\n", dest);
        break;
      case RH_ROUTER_ERROR_NO_ROUTE:
        printf("Error: No route to destination. Dest: %d\n", dest);
        break;
      case RH_ROUTER_ERROR_TIMEOUT:
        printf("Error: Operation timed out. Dest: %d\n", dest);
        break;
      case RH_ROUTER_ERROR_NO_REPLY:
        printf("Error: No reply received. Dest: %d\n", dest);
        break;
      case RH_ROUTER_ERROR_UNABLE_TO_DELIVER:
        printf("Error: Unable to deliver the message. Dest: %d\n", dest);
        break;
      default:
        printf("Error: Unknown error code %d\n", manager.sendtoWait(data, sizeof(data), dest));
      }
      uint8_t _buf[RH_RF95_MAX_MESSAGE_LEN];
      uint8_t _len = sizeof(_buf);
      uint8_t _from;
      uint8_t _to;
      uint8_t _id;
      uint8_t _flags;

      unsigned long ts = millis();
      uint8_t wait = 100;
      while ((millis() - ts) <= wait)
        ;
      YIELD;

      printf("Listening\n");

      if (manager.recvfromAckTimeout(_buf, &_len, 20000, &_from, &_to, &_id, &_flags))
      {
        printf("Received message from Node#%d to Node#%d: %s\n", _from, _to, (char *)_buf);
        printf("Received message with RSSI: %d SNR: %d\n", rf95.lastRssi(), rf95.lastSNR());
        printf("Flag: %u ID:%u \n", _flags, _id);
      }
    }
    else if (input[0] == 'L' || input[0] == 'l') // Trigger Three Latches
    {

      printf("Enter destination node ID:\n");
      char destInput[10];
      fgets(destInput, sizeof(destInput), stdin);
      dest = atoi(destInput);

      switch (manager.sendtoWait(latch, sizeof(latch), dest))
      {
      case RH_ROUTER_ERROR_NONE:
        printf("Success: No error. Dest: %d\n", dest);
        break;
      case RH_ROUTER_ERROR_INVALID_LENGTH:
        printf("Error: Invalid length. Dest: %d\n", dest);
        break;
      case RH_ROUTER_ERROR_NO_ROUTE:
        printf("Error: No route to destination. Dest: %d\n", dest);
        break;
      case RH_ROUTER_ERROR_TIMEOUT:
        printf("Error: Operation timed out. Dest: %d\n", dest);
        break;
      case RH_ROUTER_ERROR_NO_REPLY:
        printf("Error: No reply received. Dest: %d\n", dest);
        break;
      case RH_ROUTER_ERROR_UNABLE_TO_DELIVER:
        printf("Error: Unable to deliver the message. Dest: %d\n", dest);
        break;
      default:
        printf("Error: Unknown error code %d\n", manager.sendtoWait(latch, sizeof(latch), dest));
      }
    }
    else if (input[0] == 'I' || input[0] == 'i')
    {
      printf("Enter destination node ID:\n");
      char destInput[10];
      fgets(destInput, sizeof(destInput), stdin);
      dest = atoi(destInput);

      switch (manager.sendtoWait(photo, sizeof(photo), dest))
      {
      case RH_ROUTER_ERROR_NONE:
        printf("Success: No error. Dest: %d\n", dest);
        break;
      case RH_ROUTER_ERROR_INVALID_LENGTH:
        printf("Error: Invalid length. Dest: %d\n", dest);
        break;
      case RH_ROUTER_ERROR_NO_ROUTE:
        printf("Error: No route to destination. Dest: %d\n", dest);
        break;
      case RH_ROUTER_ERROR_TIMEOUT:
        printf("Error: Operation timed out. Dest: %d\n", dest);
        break;
      case RH_ROUTER_ERROR_NO_REPLY:
        printf("Error: No reply received. Dest: %d\n", dest);
        break;
      case RH_ROUTER_ERROR_UNABLE_TO_DELIVER:
        printf("Error: Unable to deliver the message. Dest: %d\n", dest);
        break;
      default:
        printf("Error: Unknown error code %d\n", manager.sendtoWait(photo, sizeof(photo), dest));
      }

      uint8_t _buf[RH_RF95_MAX_MESSAGE_LEN];
      uint8_t _len = sizeof(_buf);
      uint8_t _from;
      uint8_t _to;
      uint8_t _id;
      uint8_t _flags;

      unsigned long ts = millis();
      uint8_t wait = 100;
      while ((millis() - ts) <= wait)
        ;
      YIELD;

      if (manager.recvfromAckTimeout(_buf, &_len, 20000, &_from, &_to, &_id, &_flags))
      {
        printf("Received message from Node#%d to Node#%d\n", _from, _to);
        printf("Received message with RSSI: %d SNR: %d\n", rf95.lastRssi(), rf95.lastSNR());
        printf("Flag: %u ID:%u \n", _flags, _id);

        // Assuming size_t is 4 bytes, adjust according to your platform
        unsigned long chunkNumber = 0; // Moved outside to be accessible throughout

        if (_len >= sizeof(size_t))
        {
          size_t receivedChunks = 0;
          memcpy(&receivedChunks, _buf, sizeof(size_t)); // Save the recievedChunk to memory
          printf("Chunk Number: %lu\n", (unsigned long)receivedChunks);

          chunkNumber = receivedChunks; // Assign the value
          // printf("%lu\n", chunkNumber); // For debugging
        }

        for (size_t i = 1; i <= chunkNumber; ++i) // repeat process until chunk number is reached
        {
          unsigned long ts = millis();
          uint16_t wait = 600;
          while ((millis() - ts) <= wait)
            ;
          YIELD;
          memset(buf, 0, RH_RF95_MAX_MESSAGE_LEN);                                                           // clear the memory buffer
          int payloadLength = snprintf((char *)buf, RH_RF95_MAX_MESSAGE_LEN, "chunk %lu", (unsigned long)i); // prepare payload with chunk and number
          if (payloadLength >= 0 && payloadLength < RH_RF95_MAX_MESSAGE_LEN)
          { // make sure payload is acceptable length
            switch (manager.sendtoWait(buf, payloadLength, _from))
            { // send the chunk message
            case RH_ROUTER_ERROR_NONE:
            {
              printf("Chunk %lu sent successfully to Node#%d\n", (unsigned long)i, _from);
              uint8_t _buf[RH_RF95_MAX_MESSAGE_LEN];
              uint8_t _len = sizeof(_buf);
              uint8_t _from, _to, _id, _flags;

              unsigned long ts = millis();
              uint8_t wait = 100;
              while ((millis() - ts) <= wait)
                ;
              YIELD;

              printf("Listening for Chunk %lu\n", (unsigned long)i);

              if (manager.recvfromAckTimeout(_buf, &_len, 20000, &_from, &_to, &_id, &_flags))
              {
                if (_flags == CHUNK_FLAG)
                {
                  std::string filePath = "/home/pi-h/recombinationfile/chunk_" + std::to_string(i) + ".bin";
                  std::ofstream outFile(filePath, std::ios::binary | std::ios::out);
                  if (!outFile.is_open())
                  {
                    std::cerr << "Failed to open file for writing: " << filePath << "\n";
                  }
                  else
                  {
                    printf("Writing to %s\n", filePath.c_str());
                    outFile.write(reinterpret_cast<char *>(_buf), _len);
                    outFile.close();
                    printf("Chunk %lu received and written to %s\n", (unsigned long)i, filePath.c_str());
                  }
                }
              }
            }
            break;
            case RH_ROUTER_ERROR_INVALID_LENGTH:
              printf("Error: Invalid length. Chunk: %lu, Dest: %d\n", (unsigned long)i, _from);
              break;
            case RH_ROUTER_ERROR_NO_ROUTE:
              printf("Error: No route to destination. Chunk: %lu, Dest: %d\n", (unsigned long)i, _from);
              break;
            case RH_ROUTER_ERROR_TIMEOUT:
              printf("Error: Operation timed out. Chunk: %lu, Dest: %d\n", (unsigned long)i, _from);
              break;
            case RH_ROUTER_ERROR_NO_REPLY:
              printf("Error: No reply received. Chunk: %lu, Dest: %d\n", (unsigned long)i, _from);
              break;
            case RH_ROUTER_ERROR_UNABLE_TO_DELIVER:
              printf("Error: Unable to deliver the message. Chunk: %lu, Dest: %d\n", (unsigned long)i, _from);
              break;
            default:
              printf("Error: Unknown error. Chunk: %lu, Dest: %d\n", (unsigned long)i, _from);
            }
          }
          else
          {
            printf("Error: Payload length exceeds buffer size. Chunk: %lu, Dest: %d\n", (unsigned long)i, _from);
          }
        }
        chunkNumber = 0;
      }
    }
    else if (input[0] == 'T' || input[0] == 't') // Send Test Message
    {
      printf("Enter destination node ID:\n");
      char destInput[10];
      fgets(destInput, sizeof(destInput), stdin);
      dest = atoi(destInput);

      switch (manager.sendtoWait(test, sizeof(test), dest))
      {
      case RH_ROUTER_ERROR_NONE:
        printf("Success: No error. Dest: %d\n", dest);
        break;
      case RH_ROUTER_ERROR_INVALID_LENGTH:
        printf("Error: Invalid length. Dest: %d\n", dest);
        break;
      case RH_ROUTER_ERROR_NO_ROUTE:
        printf("Error: No route to destination. Dest: %d\n", dest);
        break;
      case RH_ROUTER_ERROR_TIMEOUT:
        printf("Error: Operation timed out. Dest: %d\n", dest);
        break;
      case RH_ROUTER_ERROR_NO_REPLY:
        printf("Error: No reply received. Dest: %d\n", dest);
        break;
      case RH_ROUTER_ERROR_UNABLE_TO_DELIVER:
        printf("Error: Unable to deliver the message. Dest: %d\n", dest);
        break;
      default:
        printf("Error: Unknown error code %d\n", manager.sendtoWait(test, sizeof(test), dest));
      }
    }
    else if (input[0] == 'R' || input[0] == 'r') // Recombine Chunks
    {
      std::string inputDirectoryPath = "/home/pi-h/recombinationfile";
      std::string outputDirectoryPath = "/home/pi-h/finishedfile";
      std::string outputFileName = "combined_file.jpg";

      if (CombineFiles(inputDirectoryPath, outputDirectoryPath, outputFileName))
      {
        std::cout << "Files have been successfully combined into " << outputFileName << std::endl;
        // Clear the input directory contents after successful combination
        ClearDirectoryContents(inputDirectoryPath);
        std::cout << "Cleared all contents from " << inputDirectoryPath << std::endl;
      }
      else
      {
        std::cerr << "An error occurred during file combination." << std::endl;
      }
    }
    gpioDelay(1000);
  }
  gpioTerminate();
  return 0;
}

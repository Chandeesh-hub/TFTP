/* Common file for server & client*/

#ifndef TFTP_H
#define TFTP_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <stdint.h>
#include <arpa/inet.h>
#include <sys/stat.h>
#include <sys/socket.h>
#include <sys/time.h>
#include <errno.h>

#define MAX_RETRIES 5   // how many times to resend a block before giving up

#define PORT 6969
#define BUFFER_SIZE 516  // TFTP data packet size (512 bytes data + 4 bytes header)
#define TIMEOUT_SEC 5    // Timeout in seconds
#define FAILURE -1
#define SUCCESS 0

// TFTP OpCodes
typedef enum {
    RRQ = 1,  // Read Request
    WRQ = 2,  // Write Request
    DATAW = 3, // Data Packet to Write
    DATAR = 4, // Data Packet to Read
    ACK = 5,  // Acknowledgment
    ERROR = 6 // Error Packet
} tftp_opcode;

// TFTP Packet Structure
typedef struct {

    uint16_t opcode; // Operation code (RRQ/WRQ/DATA/ACK/ERROR)

    union {

        struct 
        {
            char filename[256];
            char mode[8];  // Typically "octet"
        } request;  // RRQ and WRQ
        
        struct 
        {
            uint16_t block_number;
            char data[512];
        } data_packet; // DATA

        struct 
        {
            uint16_t block_number;
        } ack_packet; // ACK

        struct 
        {
            uint16_t error_code;
            char error_msg[512];
        } error_packet; // ERROR

    } body;
    
} tftp_packet;



#endif // TFTP_H

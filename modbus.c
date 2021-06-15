/***********************************
*          modbus_v3.c
*
* -Modbus communications protocol:
*  functions to get/recieve data
*  from modbus devices.
*  
* used with modbus_v3.h
*
***********************************/

// Include header file with defines, enums, macros and funcionts
#include "modbus.h"

// Define constants
#define AttemptTimeout 2    // Attempts to recieve correct message
#define SendDelay 1000000      // Micro Seconds


// Internal functions declaration
void        serialConfig(struct termios *tty);
uint16_t    errorCheck(uint8_t bytes[], int wordSize);
float       bytesToFloat(uint8_t bytes[]);
int         modbusExceptionLogger(uint8_t bytes[]);


// Variable to be used in all function (must preserve value when out of scope)
static int serial_port;



/* MODBUS ININITILIZATION: Initialize the port for protocol communication
    - Returns
        [serial_port] if everything is initialized correctly
        [>0] if there was an error.
*/
int initializePort(char *COM){
    printf("--Initializing MODBUS communication--\n");

    // Open the port
    printf("  Opening serial port in: %s\n",COM);
    serial_port = open(COM, O_RDWR);
    if(serial_port < 0){
        printf("ERROR %i from open: %s\n", errno, strerror(errno));
        syslog(LOG_ERR, "ERROR %i from open: %s", errno, strerror(errno));
        return serial_port;
    }

    // Create a termios object
    printf("  Creating termios object...\n");
    struct termios tty;

    // Configure Termio Posix
    serialConfig(&tty);
    printf("--Initialization Complete--\n");

    return serial_port;
}



/* TERMIOS POSIX CONFIGURATION: Initial configuration to enable serial communication in IO using the linux terminal.
*/
void serialConfig(struct termios* tty){
    printf(" -Serial configuration started-\n");
    
    // Control Flags
    printf("   Serial configuration (1/8)\n");
    (*tty).c_cflag |= PARENB;          // Enable parity (Can be disabled -> "&= ~PARENB") EVEN by default
    //(*tty).c_cflag |= PARODD;        // Uncomment for ODD parity
    (*tty).c_cflag &= ~CSTOPB;         // Only one stop bit (If parity disabled, 2 stop bits -> "|= CSTOPB")
    (*tty).c_cflag &= ~CSIZE;          // Clear size, and set to 8 data bits per word
    (*tty).c_cflag |= CS8;
    (*tty).c_cflag &= ~CRTSCTS;        // Disable flow control
    (*tty).c_cflag |= CREAD | CLOCAL;  // Disable other coms settings and enable reading.
    
    // Local modes
        // Canonical mode
    printf("   Serial configuration (2/8)\n");
    (*tty).c_lflag &= ~ICANON;         // Select non-cannonical mode (signal process doesn't need the end of line character to start)

        // Echoing
    printf("   Serial configuration (3/8)\n");
    (*tty).c_lflag &= ~ECHO;           // Disable echo
    (*tty).c_lflag &= ~ECHOE;          // Disable erasure
    (*tty).c_lflag &= ~ECHONL;         // Disable new line echo

        // Signal characters
    printf("   Serial configuration (4/8)\n");
    (*tty).c_lflag &= ~ISIG;           // Disable interpretation of INTR, QUIT and SUSP characters

    // Input modes
    printf("   Serial configuration (5/8)\n");
    (*tty).c_iflag &= ~(IXON | IXOFF | IXANY);                             // Turn off software flow ctrl
    (*tty).c_iflag &= ~(IGNBRK|BRKINT|PARMRK|ISTRIP|INLCR|IGNCR|ICRNL);    // Disable any special handling of received byte

    // Output modes
    printf("   Serial configuration (6/8)\n");
    (*tty).c_oflag &= ~OPOST;      // Prevent special interpretation of output bytes
    (*tty).c_oflag &= ~ONLCR;      // Prevent conversion of newline to carriage return/line feed
    //(*tty).c_oflag &= ~OXTABS;     // Prevent conversion of tabs to spaces (Comment out if errors)
    //(*tty).c_oflag &= ~ONOEOT;     // Prevent removal of C-d chars (0x004) in output (Comment out if errors)

    // Return timings (for read() function)
        // VMIN -> number of bytes
        // VTIME -> timeout value
    printf("   Serial configuration (7/8)\n");
    (*tty).c_cc[VTIME] = 1;         // Block for up to 0.1 s, until a byte arrives.
    (*tty).c_cc[VMIN] = 0;          // Block until 0 bytes have arrived.

    // Baund rate
        // Set in/out baud rate to be 9600
    printf("   Serial configuration (8/8)\n");
    cfsetispeed(tty, B9600);
    cfsetospeed(tty, B9600);

    printf("  Saving serial settings...\n");
    if (tcsetattr(serial_port, TCSANOW, tty) != 0) {
        printf("Error %i from tcsetattr: %s\n", errno, strerror(errno));
        syslog(LOG_ERR, "ERROR %i from tcsetattr: %s", errno, strerror(errno));
        abort();
    }

    if(tcgetattr(serial_port, tty) != 0){
        printf("ERROR %i from tcgetattr: %s\n", errno, strerror(errno));
        syslog(LOG_ERR, "ERROR %i from tcgetattr: %s", errno, strerror(errno));
        abort();
    }
    printf(" -Serial configuration successful-\n");
}



/* MODBUS MASTER QUERY AND RESPONSE: Send all the necessary bytes for the salve to understand the message.

    +Slave ID::         [0...255] in HEX
    +Start Address::    Choose from the StartAddress typedef ennum [2 bytes]

    returns 0xFFFFFFFF if error
            or value of parameter in float format
*/
float modbusQuery(uint8_t slave_id, StartAddress_3X StartAddress){
    uint16_t    byte_count      = 0x0002;                       // Register Size (2 bytes)
    uint8_t     funtion_code    = R_3X;                         // Function code for message (read from 3x registers)
    uint8_t     tx_msg[8];                                      // Transfer message (8 bytes)
    uint16_t    errorWord;                                      // Error check word (2 bytes)
    int         attempts        = 0;                            // Counter that keeps track of the attempts
    int         n_bytes;                                        // Number of bytes recieved
    uint8_t     rx_buffer[64];                                  // Recieved bytes buffer (64 bytes reserved)
    uint8_t     rx_message[20];                                 // Array for the recieved message (Max size of 20 bits (device max is 9 bytes))
    uint8_t     *byte_response  = calloc(4, sizeof(uint8_t));   // Pointer of the response in bytes
    float       float_response  = 0xFFFFFFFF;                   // Number that the funtion will return                     

    printf("--Begining Query process--\n");
    //_____SEND MESSAGE_______
    // MESSAGE FORMAT (8 bytes) 
    //[Salve Adress | Function Code | Start Address (H) | Start Address (L) | Register Size (H) | Register Size (L) | Error Check (L) | Error Check (H)]
    printf("  Composing message to send...\n");

    // Initialize message without error check bytes
    printf("   Composing message (1/2)\n");
    tx_msg[0] = slave_id;
    tx_msg[1] = funtion_code;
    tx_msg[2] = GET_HIGH(StartAddress);
    tx_msg[3] = GET_LOW(StartAddress);
    tx_msg[4] = GET_HIGH(byte_count);
    tx_msg[5] = GET_LOW(byte_count);

    // Calcualte error checking bytes CRC (only for the part of the message without error check bytes (6 bytes))
    errorWord = errorCheck(tx_msg, 6);
    // Add error check bytes to transfer message (first the Low bytes then the High)
    printf("   Composing message (2/2)\n");
    tx_msg[6] = GET_LOW(errorWord);
    tx_msg[7] = GET_HIGH(errorWord);        
    

    //___SEND MESSEGE AND PROCESS RESPONSE___
    printf("  Preparing to send message...\n");
    // Initialize response to error signal by default
    byte_response[0] = 0xFF;
    byte_response[1] = 0xFF;
    byte_response[2] = 0xFF;
    byte_response[3] = 0xFF;

    // Delete any bytes already on the buffer
    tcflush(serial_port,TCIOFLUSH);

    while(attempts < AttemptTimeout){
        // Delete any bytes already on the buffer
        tcflush(serial_port,TCIOFLUSH);
        //___Send the message___
        printf("  Sending message...\n");
        if( write(serial_port, tx_msg, sizeof(tx_msg)) < 0){
            printf("ERROR %i from write: %s\n", errno, strerror(errno));
            syslog(LOG_ERR, "ERROR %i from write: %s", errno, strerror(errno));
            abort();
        }
        printf("   Sending : ");
        for(int i = 0; i < sizeof(tx_msg); i ++) printf("%#02x ", tx_msg[i]);
        printf("\n");
        printf("  Message Sent\n");

        //___Wait for response___
        // Wait 1 ms for buffered message to be transfered and reply waiting in buffer
        printf("  Waiting for response...\n");
        if(usleep(SendDelay) < 0) printf("ERROR %i from usleep: %s\n", errno, strerror(errno));
        
        
        //___Read response___
        printf("  Reading response...\n");
        n_bytes = read(serial_port+1, rx_buffer, 20);
        for(int i = 0; i < n_bytes; i ++)rx_message[i] = rx_buffer[i];
        int temp = 0;       // Variable to temporarly hold the real message size
        if(n_bytes < 5){
            // Send message again
            printf(" WARNING: Modbus message to short. Sending again...\n");
            attempts ++;
            continue;
        }
        n_bytes = n_bytes + temp;
        printf("  Message of %i bytes recieved: ", n_bytes);
        for(int i = 0; i < n_bytes; i ++) printf("%#02x ", rx_message[i]);
        printf("\n");

        //___Process message___
        // Recieved message structure (9 bytes usually)
        //    0         1         2          3           4          5           6             7                 8
        //[Slave ID, Fn Code, Byte Count, Reg1(high), Reg1(low), Reg2(high), Reg2(low), Error Check(low), Error Check(high)]
        printf("  Processing Response...\n");

        // Check if exception code was sent
        if(n_bytes == 5){
            if(modbusExceptionLogger(rx_message) == 0) break;
            else{
                attempts ++;
                continue;
            }
        }

        // Remove Error check bytes from message, compute locally, and compare if it's the same. Re-run if not.
        errorWord = errorCheck(rx_message, (n_bytes - 2));
        if((GET_HIGH(errorWord) != rx_message[n_bytes-1]) || (GET_LOW(errorWord) != rx_message[n_bytes-2])){
            printf(" WARNING: MODBUS message corrupted. Sending query again.\n");
            printf("  [%#02x, %#02x](local) doesn't match [%#02x, %#02x](RX)\n",
                    GET_HIGH(errorWord), GET_LOW(errorWord),
                    rx_message[n_bytes-1],rx_message[n_bytes-2]);
            attempts ++;
            continue;
        }else if((rx_message[0] != slave_id) || (rx_message[1] != funtion_code)){
        // Check if it's from the same slave and message we sent out
            printf(" WARNING: MODBUS unidentified message. Sending query again.\n");
            printf("  [%#02x, %#02x](local) doesn't match [%#02x, %#02x](RX)\n",
                    slave_id, funtion_code,
                    rx_message[0],rx_message[1]);
            attempts ++;
            continue;
        }else{
        // Extract float bytes
            printf("  Response recieved succesfully: ");
            for(int i = 0; i < rx_message[2]; i ++) byte_response[i] = rx_message[i+3];
            for(int i = 0; i < rx_message[2]; i ++) printf("%#02x ",byte_response[i]);
            printf("\n");
            break;
        }
    }
    if (!(attempts < AttemptTimeout)) printf("ERROR: Too many attemps, returning error signal.\n");
    
    float_response = bytesToFloat(byte_response);
    free(byte_response);
    printf("--FLOAT TO SEND: %f--\n", float_response);
    printf("--Query process finished--\n");
    return float_response;
}



/* MODBUS EXCEPTION LOGGER: Logs to the linux system the message exceptions sent by the device

    -Returns 0 if message is undefined
            -1 if exception is exception is recognized
*/
int modbusExceptionLogger(uint8_t bytes[]){
    uint16_t errorword = errorCheck(bytes, 3);
    // Check integrity of message
    if((GET_LOW(errorword) != bytes[3]) || GET_HIGH(errorword) != bytes[4]){
        printf("ERROR: MODBUS unidentified message. Sending query again.\n");
        printf("  MODBUS Exception message not recognized.\n");
        return 0;
    }
    switch(bytes[2]){
        case 0x01: 
            printf("ERROR: Modbus excpetion 0x01");
            syslog(LOG_ERR, "ERROR 0x01 from modbusExceptionLogger: MODBUS EXCEPTION Illegal Function.");
            break;
        case 0x02: 
            printf("ERROR: Modbus excpetion 0x02");
            syslog(LOG_ERR, "ERROR 0x02 from modbusExceptionLogger: MODBUS EXCEPTION Illegal Data Address.");
            break;
        case 0x03: 
            printf("ERROR: Modbus excpetion 0x03");
            syslog(LOG_ERR, "ERROR 0x03 from modbusExceptionLogger: MODBUS EXCEPTION Illegal Data Value.");
            break;
        case 0x05: 
            printf("ERROR: Modbus excpetion 0x05");
            syslog(LOG_ERR, "ERROR 0x04 from modbusExceptionLogger: MODBUS EXCEPTION Salve Devive Failure.");
            break;
        default:
            printf("ERROR: MODBUS exception not recognized. Sending query again.\n");
            return 0;
    }
    return -1;
}



/* ERROR CHECK ALGORITHM: crc error checking bytes required by the MODBUS protocol
    Parameters:
        -bytes : messages of n bytes that requires CRC
        -n     : number of bytes in message
    returns a 2 byte word
*/
uint16_t errorCheck(uint8_t bytes[], int n){
    // Calcualte error checking bytes CRC
    uint16_t errorWord = 0xFFFF;   // Set to 1's
    // For each byte in message (Code from: SDM230-ModBus Protocol V1.2)
    for(int i = 0; i < n; i ++){
        errorWord ^= bytes[i];
        // For each bit in current byte
        for(int j = 0; j < 8; j++){
            uint8_t LSB = errorWord & 0x1;
            if(LSB == 0x1){
                errorWord -= 0x1;
            }
            errorWord = errorWord/2;
            if (LSB == 0x1){
                errorWord ^= 0xA001;
            }
            
        }
    }
    return errorWord;
}



/* BYTES TO FLOATING POINT NUMBER: function that recieves an array of 4 bytes an returns a floating poiny number*/
float bytesToFloat(uint8_t bytes[]){
    float f;
    *((uint8_t*)(&f) + 3) = bytes[0];
    *((uint8_t*)(&f) + 2) = bytes[1];
    *((uint8_t*)(&f) + 1) = bytes[2];
    *((uint8_t*)(&f) + 0) = bytes[3];
    return f;
}
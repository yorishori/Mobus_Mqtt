#ifndef modbus
#define modbus

// Using standar libraries
// C headers
#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <math.h>
#include <float.h>

// Linux headers
#include <fcntl.h>      // Contains serial coms controls
#include <errno.h>      // Defines the ERROR variables
#include <termios.h>    // Contains POSIX terminal controls
#include <unistd.h>     // write(), read(), close()
#include <syslog.h>     // Log events (/var/logs)

// MQTT Server (Mosquitto)
#include <mosquitto.h>

// Macros
#define GET_HIGH(a)(a >> 8)     // Get high 8 bits of 16
#define GET_LOW(a)(a & 0xFF)    // Get low 8 bits of 16

// Enums
typedef enum{
    RW_4X   = 0x03,     // Read contents of read/write locations (4X References )
    R_3X    = 0x04      // Read contents of read-only location (3X References)
}FunctionCode;



typedef enum{
    // Read locations
    VOLTAGE                 = 0x0000,   // Line to neutral volts (V)
    CURRENT                 = 0x0006,   // Current (A)

    POWER_ACTIVE            = 0x000C,   // Active power (W)
    POWER_APPARENT          = 0x0012,   // Apparent power (VA)
    POWER_REACTIVE          = 0x0018,   // Reactive power (VAr)

    POWER_FACTOR            = 0x001E,   // Power factor
    PHASE                   = 0x0024,   // Phase angle (º)
    FREQUENCY               = 0x0046,   // Frecuency (Hz)

    ACT_ENERGY_IM           = 0x0048,   // Import active energy (kwh)
    ACT_ENERGY_EX           = 0x004A,   // Export active energy (kwh)
    REA_ENERGY_IM           = 0x004C,   // Import reactive energy (kvarh)
    REA_ENERGY_EX           = 0x004E,   // Export reactive energy (kvarh)

    T_PDEMAND               = 0x0054,   // Total system power demand (W)
    T_PDEMAND_MAX           = 0x0056,   // Maximum total system power demand (W)
    POS_PDEMAND_CURRENT     = 0x0058,   // Current system positive power demand (W)
    POS_PDEMAND_MAX         = 0x005A,   // Maximum system positive power demand (W)
    REV_PDEMAND_CURRENT     = 0X005C,   // Current system reverse power demand (W)
    REV_PDEMAND_MAX         = 0X005E,   // Maximum system reverse power demand (W)

    CDEMAND                 = 0x0102,   // Current demand (A)
    CDEMAND_MAX             = 0x0108,   // Maximum current demand (A)

    ACT_ENERGY_T            = 0x0156,   // Total active energy (kwh)
    REA_ENERGY_T            = 0x0158,   // Total reactive energy (kvarh)
    RS_ACT_ENERGY           = 0x0180,   // Current resettable total active energy (kwh)
    RS_REA_ENERGY           = 0x0182,   // Current resettable total reactive energy (kvarh) 
}StartAddress_3X;



typedef enum{
 // Write locations ('·' default)
    PULSE_OUT_W             = 0x000C,   // Pulse output 1 width: [float] => 60, ·100, 200
    NET_PARITY              = 0x0012,   // Network parity: [float](Stop bit, parity) => ·0(1,none), 1(1,even), 2(1,odd), 3(2,none)
    NET_NODE                = 0x0014,   // Network port node: [float] => ·1...247
    NET_BD                  = 0x001C,   // Network baud rate: [float](bd) => ·0(2400), 1(4800), 2(9600), 5(1200)
    PULSE_TYPE              = 0x0056,   // Pulse1 Energy Type: [float](type) => 1(import wh), 2(im/export wh), ·4(export wh), 5(import varh), 6(im/export varh), 8(export varh)
    DISP_SETTINGS           = 0xF500,   // 4 bytes [Demand interval(min), Slide time(min), Scroll interval (s), Backlight time(min)]
    PULSE_CONST             = 0xF910,   // Pulse 1 constant: [2 bytes hex](kwh/imp) => ·0000(0.001), 0001(0.01), 0002(0.1), 0003(1)
    MEASURE_MODE            = 0xF920,   // Measurment mode: [2 bytes hex](total) => 0001(imp), ·0002(imp+exp), 0003(imp-exp)
    RUN_TIME                = 0xF930    // Running time, continuos working period--hour: [float]
}StartAddress_4X;



// Functions
int         initializePort(char *COM);
float       modbusQuery(uint8_t slave_id, StartAddress_3X StartAddress);

#endif
------------------------------------
**** MODBUS QUERY START ADDRESS ****
****    READ REFISTERS (3X)     ****
------------------------------------
    VOLTAGE                 // Line to neutral volts (V)
    CURRENT                 // Current (A)

    POWER_ACTIVE            // Active power (W)
    POWER_APPARENT          // Apparent power (VA)
    POWER_REACTIVE          // Reactive power (VAr)

    POWER_FACTOR            // Power factor
    PHASE                   // Phase angle (º)
    FREQUENCY               // Frecuency (Hz)

    ACT_ENERGY_IM           // Import active energy (kwh)
    ACT_ENERGY_EX           // Export active energy (kwh)
    REA_ENERGY_IM           // Import reactive energy (kvarh)
    REA_ENERGY_EX           // Export reactive energy (kvarh)

    T_PDEMAND               // Total system power demand (W)
    T_PDEMAND_MAX           // Maximum total system power demand (W)
    POS_PDEMAND_CURRENT     // Current system positive power demand (W)
    POS_PDEMAND_MAX         // Maximum system positive power demand (W)
    REV_PDEMAND_CURRENT     // Current system reverse power demand (W)
    REV_PDEMAND_MAX         // Maximum system reverse power demand (W)

    CDEMAND                 // Current demand (A)
    CDEMAND_MAX             // Maximum current demand (A)

    ACT_ENERGY_T            // Total active energy (kwh)
    REA_ENERGY_T            // Total reactive energy (kvarh)
    RS_ACT_ENERGY           // Current resettable total active energy (kwh)
    RS_REA_ENERGY           // Current resettable total reactive energy (kvarh) 

------------------------------------
**** MODBUS QUERY START ADDRESS ****
****   WRITE REFISTERS (4X)     ****
------------------------------------
    PULSE_OUT_W             // Pulse output 1 width: [float] => 60, ·100, 200
    NET_PARITY              // Network parity: [float](Stop bit, parity) => ·0(1,none), 1(1,even), 2(1,odd), 3(2,none)
    NET_NODE                // Network port node: [float] => ·1...247
    NET_BD                  // Network baud rate: [float](bd) => ·0(2400), 1(4800), 2(9600), 5(1200)
    PULSE_TYPE              // Pulse1 Energy Type: [float](type) => 1(import wh), 2(im/export wh), ·4(export wh), 5(import varh), 6(im/export varh), 8(export varh)
    DISP_SETTINGS           // 4 bytes [Demand interval(min), Slide time(min), Scroll interval (s), Backlight time(min)]
    PULSE_CONST             // Pulse 1 constant: [2 bytes hex](kwh/imp) => ·0000(0.001), 0001(0.01), 0002(0.1), 0003(1)
    MEASURE_MODE            // Measurment mode: [2 bytes hex](total) => 0001(imp), ·0002(imp+exp), 0003(imp-exp)
    RUN_TIME                // Running time, continuos working period--hour: [float]
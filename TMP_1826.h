/***
___Precision (16-Bit) Temperature Data Format___
|TEMPERATURE |DIGITAL OUTPUT (PRECISION FORMAT)|
|____(°C)____|_______BINARY______|_HEXADECIMAL_|
|     150    |0100 1011 0000 0000|    4B00     |
|     127    |0011 1111 1000 0000|    3F80     |
|     100    |0011 0010 0000 0000|    3200     |
|      25    |0000 1100 1000 0000|    0C80     |
|      1     |0000 0000 1000 0000|    0080     |
|   0.125    |0000 0000 0001 0000|    0010     |
|   0.03125  |0000 0000 0000 0100|    0004     |
|  0.0078125 |0000 0000 0000 0001|    0001     |
|      0     |0000 0000 0000 0000|    0000     |
| –0.0078125 |1111 1111 1111 1111|    FFFF     |
|  –0.03125  |1111 1111 1111 1100|    FFFC     |
|   –0.125   |1111 1111 1111 0000|    FFF0     |
|     –1     |1111 1111 1000 0000|    FF80     |
|    –25     |1111 0011 1000 0000|    F380     |
|    –40     |1110 1100 0000 0000|    FC00     |
|    –55     |1110 0100 1000 0000|    F480     |
***/

#ifndef TMP_1826
#define TMP_1826

#include <cstdint>
#include "mbed.h"
#include "owm.h"


#define BITS_TO_TEMP_16 0.0078125   // Precision (16-Bit) Temperature Data Format (requires TMP_FMT bit to be set in the Configuration-1 Register)
#define BITS_TO_TEMP_12 0.125       // Legacy (12-Bit) Temperature Data Format (default configuration)
#define ADDRSIZE        8           // 8 bytes = 64bit address 
#define SUCCESS         0           // Function executed without error
#define ERROR_RESET     1           // No response after reset
#define ERROR_CRC       2           // Recieved CRC checksum did not match calculated CRC checksum  
#define ERROR_PROG      3           // Failed to program

// address commands
#define READADDR            0x33    // Read device address, only use it when there is one device on the bus 
#define MATCHADDR           0x55    // Require a matching address
#define SEARCHADDR          0xF0    // Enter address search mode
#define ALERTSEARCH         0xEC    // Works like SEARCHADDR but only devices with an alert flag respond
#define SKIPADDR            0xCC    // Skip addressing to interact with all devices on bus
#define OVD_MATCHADDR       0x69    // Overdrive version of MATCHADDR
#define OVD_SKIPADDR        0x3C    // Overdrive version of SKIPADDR
#define FLEXADDR            0x0F    // Access a device by the short address which in stored on the SCRATCHPAD_1\SHORT_ADDR register

// functions
#define CONVERTTEMP         0x44
#define WRITE_SCRATCHPAD_1  0x4E
#define READ_SCRATCHPAD_1   0xBE
#define COPY_SCRATCHPAD_1   0x48
#define WRITE_SCRATCHPAD_2  0x0F
#define READ_SCRATCHPAD_2   0xAA
#define COPY_SCRATCHPAD_2   0x55
#define READ_EEPROM         0xF0

// Status Register bitmask 
#define ALERT_HIGH          0x80
#define ALERT_LOW           0x40
// reserved                 0x20
// reserved                 0x10
#define DATA_VALD           0x08    //STATUS_REG bit indicating update in temperature result register
#define POWER_MODE          0x04
#define ARB_DONE            0x02
#define LOCK_STATUS         0x01

// Device Configuration-1 Register bitmask 
#define TEMP_FMT            0x80    //Selects the temperature format, 0b = 12-bit legacy format, 1b = 16-bit high precision format
//reserved                  0x40
#define CONV_TIME_SEL       0x20    //ADC conversion time, 0b = 3ms, 1b = 5.5ms
#define ALERT_MODE          0x10
#define AVG_SEL             0x08    //Conversion averaging selection
//CONV_MODE_SEL             0x04    //010b = Auto temperature conversion mode is enabled (in bus powered mode)
//CONV_MODE_SEL             0x02    //000b = Default one shot conversion mode using CONVERT
//CONV_MODE_SEL             0x01    //001b = Stacked conversion mode is enabled

// Device Configuration-2 Register bitmask 
#define OD_EN               0x80    //Overdrive mode enable
//FLEX_ADDR_MODE            0x40    //refer to datasheet
//FLEX_ADDR_MODE            0x20
//ARB_MODE                  0x10    //refer to datasheet
//ARB_MODE                  0x08  
//HYSTERESIS                0x04    //refer to datasheet
//HYSTERESIS                0x02
#define LOCK_EN             0x01    //Register protection enable bit



class TMP1826 {
public:
    std::uint8_t TMP1826_config[9];
    //bool bus_powered;   // is TMP1826 powered via bus 
    bool str_pu; // is in bus powered "strong pullup" configuration, (using a FET/transitor as a switchable low impedance current path)


    int init (bool long_line_enable, bool overdrive_enable);

    std::uint8_t CalculateCRC(const std::uint8_t* byte_block, const int num_bytes); // returns 0 upon success, returns 1 if crc failed

    /// Find the addresses using fast arbritarion search
    /// @param addresses[][8] Array of 64bit addresses
    /// @param num number of addresses in the array
    /// @note devices must be set to fast arbitration Config_2(ARB_MODE=11b)
    int SearchDevices(std::uint8_t addresses[][ADDRSIZE], int num); //


    /// @brief   Send reset pulse and commands all devices on the bus to convert ADC reading into 16-bit format and store in scratchpad-1
    /// @retval  (0) = temerature converted and device responded, (1) = no response detected
    int OneShotConversion();


    /// @brief   Send reset pulse and command all devices on the bus to convert current temperature reading
    /// @param   owm         Pointer to OWM regs.
    /// @param   address     64-bit device address, use nullptr if there is only 1 device on the bus
    /// @retval  (0) = temerature converted and device responded, (1) = no response detected
    int GetDebugTemperature(const std::uint8_t address[ADDRSIZE], std::uint8_t read_buffer[18]); // 64-bit address, use nullptr if there is only 1 device on the bus

    /// @brief    WIP
    /// @param   WIP         WIP.
    /// @param   address     64-bit device address, use nullptr if there is only 1 device on the bus
    /// @retval  NAN = Failed to get a temperature reading
    int ProgramConfig(const std::uint8_t address[ADDRSIZE], const std::uint8_t configuration[9]);


    int ProgramAll(const std::uint8_t configuration[9]);

    int SetShortAddress(const std::uint8_t long_address[8], const std::uint8_t short_address);

    int SetOffset(const std::uint8_t long_address[ADDRSIZE], const std::uint8_t offset_l, const std::uint8_t offset_h);

    /// @brief   Get the temerature in degrees Celcius from the provided address
    /// @param   address     64-bit device address, use nullptr if there is only 1 device on the bus
    /// @retval  float  Temperature reading in degrees Celcius
    /// @retval  NAN    Failed to get a temperature reading
    float Temperature(std::uint8_t address[ADDRSIZE]); // 64-bit address, use nullptr if there is only 1 device on the bus

    /// @brief   Same as Temerature function but with short address and without errorchecking
    /// @param   short_address     uses 8-bit device address on scratchpad-1
    /// @retval  NAN = Failed to get a temperature reading
    float FastTemperature(std::uint8_t short_addr); // 8-bit address

    typedef enum read_buffer_map{     // full reading sequence of SCRATCHPAD_1 returns 18 bytes
        TEMP_RESULT_L = 0,  // Device sends temperature result LSB register
        TEMP_RESULT_H = 1,  // Device sends temperature result MSB register
        STATUS_REG,         // (Optional read for host) Device sends status register
        reserved_byte,      // (Optional read for host) Device sends reserved byte
        CONFIG_REG1,        // (Optional read for host) Device sends configuration-1 register
        CONFIG_REG2,        // (Optional read for host) Device sends configuration-2 register
        SHORT_ADDR,         // (Optional read for host) Device sends short address register
        reserved_byte2,     // (Optional read for host) Device sends reserved byte
        CRC1,               // (Optional read for host) Device sends CRC (checksum) on first 8 bytes
        TEMP_ALERT_LOW_L,   // (Optional read for host) Device sends temperature alert low LSB register
        TEMP_ALERT_LOW_H,   // (Optional read for host) Device sends temperature alert low MSB register
        TEMP_ALERT_HIGH_L,  // (Optional read for host) Device sends temperature alert high LSB register
        TEMP_ALERT_HIGH_H,  // (Optional read for host) Device sends temperature alert high MSB register
        TEMP_OFFSET_L,      // (Optional read for host) Device sends temperature offset LSB register
        TEMP_OFFSET_H,      // (Optional read for host) Device sends temperature offset MSB register
        reserved_byte3,     // (Optional read for host) Device sends reserved byte
        reserved_byte4,     // (Optional read for host) Device sends reserved byte
        CRC2                // (Optional read for host) Device sends CRC on last 8 bytes
    } buffer_map; 

    typedef enum TMP1826_write_config_map{     // 9 byte write sequence to SCRATCHPAD_1 which then returns CRC
        CONFIG1=0,          // (Optional read for host) Device sends configuration-1 register
        CONFIG2=1,          // (Optional read for host) Device sends configuration-2 register
        SH_ADDR=2,          // (Optional read for host) Device sends short address register
        ALERT_LOW_LSB=3,    // (Optional read for host) Device sends temperature alert low LSB register
        ALERT_LOW_MSB=4,    // (Optional read for host) Device sends temperature alert low MSB register
        ALERT_HIGH_LSB=5,   // (Optional read for host) Device sends temperature alert high LSB register
        ALERT_HIGH_MSB=6,   // (Optional read for host) Device sends temperature alert high MSB register
        OFFSET_LSB=7,       // (Optional read for host) Device sends temperature offset LSB register
        OFFSET_MSB=8,       // (Optional read for host) Device sends temperature offset MSB register
    } write_config; 
    

};




#endif

#include <cstdint>
#include <stdint.h>
#include <stddef.h>
#include <iostream>
#include "TMP_1826.h"

/***
______Typical Communication Flow____   (Copied from the TMP1826 Datasheet)
• Start with the Reset Sequence
• If the host must determine which target devices are present on the bus, it should perform a search to detect
the 64-bit device address of the devices.
• Before performing an operation on a device, the device must be configured and/or selected using the ROM
commands. Some of the available functional ROM commands are:
    – Read Address [0x33]: Only used when there is a single device on the bus. This command reads the 64-bit
        device address of the only device present on the bus.
    – Match Address [0x55]: This command followed by a 64-bit device address selects the device with the
        matching address. All other devices wait until the next reset pulse.
    – Search Address [0xF0]: This command is required to obtain the 64-bit device address of multiple devices,
        and it informs devices that a search is going to be conducted by the host. The Search is then conducted
        by reading a bit and its complement of the ROM numbers from the devices and sending an appropriate bit
        back. For more details, see the Section 4. Target devices that have the same bit as the one sent by the
            host remain active while others wait for the next reset
    – Skip Address [0xCC]: Devices can be addressed without the host knowing the 64-bit device address. This
        command is helpful when giving a common command to all the devices.
    – Overdrive Skip Address [0x3C]: This command is used only in single drop. This command is the same as
        the Skip Address command except that only devices that can run in overdrive mode remain active and go
        into overdrive mode. Devices that cannot run in overdrive mode wait for the next reset.
    – Overdrive Match Address [0x69]: This command is the same as the Match Address command except that
        the device is only matched if it can run in overdrive mode. All other devices wait for the next reset.
• After selecting the required devices, a device-specific command can be issued to perform the required
operation.
• Typically after each operation, a reset pulse is issued.
***/



int TMP1826::init (bool long_line_enable, bool overdrive_enable){
    OWM_Shutdown(MXC_OWM);
    // setting up onewire
    owm_cfg_t m_owmCfg;
    sys_cfg_owm_t m_sysCfgOwm;
    m_sysCfgOwm.clk_scale = CLKMAN_SCALE_AUTO;
    m_owmCfg.ext_pu_mode = OWM_EXT_PU_ACT_LOW; // MAX32630fthr has built in strongpullup
    ioman_cfg_t cfg = IOMAN_OWM(1, 1);
    m_sysCfgOwm.io_cfg = cfg;
    m_owmCfg.int_pu_en = 1;
    m_owmCfg.long_line_mode = long_line_enable;
    if(overdrive_enable){
        m_owmCfg.overdrive_spec = OWM_OVERDRIVE_10US;
    }   else m_owmCfg.overdrive_spec = OWM_OVERDRIVE_UNUSED; 

    thread_sleep_for(500);
    OWM_Init(MXC_OWM, &m_owmCfg, &m_sysCfgOwm);
    str_pu = 1;

    //                                0x80 | 0x40 | 0x20 | 0x10 | 0x08 | 0x04 | 0x02 | 0x01 = 0xFF = 11111111b
    TMP1826_config[CONFIG1]         = TEMP_FMT | 0x40 | CONV_TIME_SEL | ALERT_MODE; // =0xF0
    TMP1826_config[CONFIG2]         = OD_EN | 0x18; // 0x18 = fast arbitration mode
    TMP1826_config[SH_ADDR]         = 0x00; // default
    TMP1826_config[ALERT_LOW_LSB]   = 0x00; // default
    TMP1826_config[ALERT_LOW_MSB]   = 0x00; // default
    TMP1826_config[ALERT_HIGH_LSB]  = 0x00; // 0xF0 default
    TMP1826_config[ALERT_HIGH_MSB]  = 0x32; // 0x07 default
    TMP1826_config[OFFSET_LSB]      = 0x00; // default; 
    TMP1826_config[OFFSET_MSB]      = 0x00; // default; 
    //OWM_Init(mxc_owm_regs_t *owm, const owm_cfg_t *cfg, const sys_cfg_owm_t *sys_cfg);
    //if (!OWM_Reset(MXC_OWM)) return ERROR_RESET;
    return SUCCESS;
 }


int TMP1826::ProgramConfig (const std::uint8_t address[ADDRSIZE], const std::uint8_t configuration[9]) { // saves to EEPROM
    std::uint8_t read_crc[1];

    if (!OWM_Reset(MXC_OWM)) return ERROR_RESET;
    OWM_WriteByte(MXC_OWM, MATCHADDR); // require address
    for  (int i=0; i<ADDRSIZE; i++) OWM_WriteByte(MXC_OWM, address[i]); // Host sends all 8 Bytes of Address, LSb to MSb
    OWM_WriteByte(MXC_OWM, WRITE_SCRATCHPAD_1);                         // Host sends function command to write register scratchpad-1
    for  (int i=0; i<9; i++) OWM_WriteByte(MXC_OWM, configuration[i]);  //Host sends the updated 9 registers of scratchpad-1

    OWM_Read(MXC_OWM, read_crc, 1);                     //Device sends CRC for the recieved register bytes
    std::uint8_t crc = CalculateCRC(configuration, 9);  //Calculate CRC based on what should have been sent
    if (read_crc[0] != crc) return ERROR_CRC;                   //Ensure they are equal

    if (!OWM_Reset(MXC_OWM)) return ERROR_RESET;
    OWM_WriteByte(MXC_OWM, MATCHADDR); // require address
    for  (int i=0; i<ADDRSIZE; i++) OWM_WriteByte(MXC_OWM, address[i]); // Host sends all 8 Bytes of Address, LSb to MSb
    OWM_WriteByte(MXC_OWM, COPY_SCRATCHPAD_1);  // Save current scratchpad-1 configuration to EEPROM
    thread_sleep_for(45);                       // Wait for EEPROM to finish
    return SUCCESS;
};


int TMP1826::SetShortAddress (const std::uint8_t long_address[ADDRSIZE], const std::uint8_t short_address) {
    std::uint8_t read_buff[18];
    std::uint8_t configuration[9];
    std::uint8_t read_crc[1];

    if (!OWM_Reset(MXC_OWM)) return ERROR_RESET;
    OWM_WriteByte(MXC_OWM, MATCHADDR); // require address
    for  (int i=0; i<ADDRSIZE; i++) OWM_WriteByte(MXC_OWM, long_address[i]); // Host sends all 8 Bytes of Address, LSb to MSb
    OWM_WriteByte(MXC_OWM, READ_SCRATCHPAD_1);
    OWM_Read(MXC_OWM, read_buff, 18);

    if(read_buff[CRC1] != CalculateCRC(read_buff, CRC1)) return ERROR_CRC;  // Test first checksum
    if(read_buff[CRC2] != CalculateCRC(read_buff, CRC2)) return ERROR_CRC;  // Test second checksum

    // Copy the current configuration and change only the short_address register 
    configuration[CONFIG1] = read_buff[CONFIG_REG1];
    configuration[CONFIG2] = read_buff[CONFIG_REG2];
    configuration[SH_ADDR] = short_address;
    configuration[ALERT_LOW_LSB] = read_buff[TEMP_ALERT_LOW_L];
    configuration[ALERT_LOW_MSB] = read_buff[TEMP_ALERT_LOW_H];
    configuration[ALERT_HIGH_LSB] = read_buff[TEMP_ALERT_HIGH_L];
    configuration[ALERT_HIGH_MSB] = read_buff[TEMP_ALERT_HIGH_H];
    configuration[OFFSET_LSB] = read_buff[TEMP_OFFSET_L];
    configuration[OFFSET_MSB] = read_buff[TEMP_OFFSET_H];
    
    int return_val = ERROR_PROG;
    return_val = ProgramConfig(long_address, configuration);
    //if (!OWM_Reset(MXC_OWM)) return ERROR_RESET;
    //OWM_WriteByte(MXC_OWM, MATCHADDR); // require address
    //for  (int i=0; i<ADDRSIZE; i++) OWM_WriteByte(MXC_OWM, long_address[i]); // Host sends all 8 Bytes of Address, LSb to MSb
    //OWM_WriteByte(MXC_OWM, WRITE_SCRATCHPAD_1);                         // Host sends function command to write register scratchpad-1
    //for  (int i=0; i<9; i++) OWM_WriteByte(MXC_OWM, configuration[i]);  //Host sends the updated 9 registers of scratchpad-1
//
    //OWM_Read(MXC_OWM, read_crc, 1);                     //Device sends CRC for the recieved register bytes
    //std::uint8_t crc = CalculateCRC(configuration, 9);  //Calculate CRC based on what should have been sent
    //if (read_crc[0] != crc) return ERROR_CRC;                   //Ensure they are equal
//
    //if (!OWM_Reset(MXC_OWM)) return ERROR_RESET;
    //OWM_WriteByte(MXC_OWM, MATCHADDR); // require address
    //for  (int i=0; i<ADDRSIZE; i++) OWM_WriteByte(MXC_OWM, long_address[i]); // Host sends all 8 Bytes of Address, LSb to MSb
    //OWM_WriteByte(MXC_OWM, COPY_SCRATCHPAD_1);  // Save current scratchpad-1 configuration to EEPROM
    //thread_sleep_for(45);                       // Wait for EEPROM to finish
    return return_val;
};

int TMP1826::SetOffset (const std::uint8_t long_address[ADDRSIZE], const std::uint8_t offset_l, const std::uint8_t offset_h) {
    std::uint8_t read_buff[18];
    std::uint8_t configuration[9];
    std::uint8_t read_crc[1];

    if (!OWM_Reset(MXC_OWM)) return ERROR_RESET;
    OWM_WriteByte(MXC_OWM, MATCHADDR); // require address
    for  (int i=0; i<ADDRSIZE; i++) OWM_WriteByte(MXC_OWM, long_address[i]); // Host sends all 8 Bytes of Address, LSb to MSb
    OWM_WriteByte(MXC_OWM, READ_SCRATCHPAD_1);
    OWM_Read(MXC_OWM, read_buff, 18);

    if(read_buff[CRC1] != CalculateCRC(read_buff, CRC1)) return ERROR_CRC;  // Test first checksum
    if(read_buff[CRC2] != CalculateCRC(read_buff, CRC2)) return ERROR_CRC;  // Test second checksum

    // Copy the current configuration and change only the short_address register 
    configuration[CONFIG1] = read_buff[CONFIG_REG1];
    configuration[CONFIG2] = read_buff[CONFIG_REG2];
    configuration[SH_ADDR] = read_buff[SHORT_ADDR];
    configuration[ALERT_LOW_LSB] = read_buff[TEMP_ALERT_LOW_L];
    configuration[ALERT_LOW_MSB] = read_buff[TEMP_ALERT_LOW_H];
    configuration[ALERT_HIGH_LSB] = read_buff[TEMP_ALERT_HIGH_L];
    configuration[ALERT_HIGH_MSB] = read_buff[TEMP_ALERT_HIGH_H];
    configuration[OFFSET_LSB] = offset_l;
    configuration[OFFSET_MSB] = offset_h;
    
    int return_val = ERROR_PROG;
    return_val = ProgramConfig(long_address, configuration);
    //if (!OWM_Reset(MXC_OWM)) return ERROR_RESET;
    //OWM_WriteByte(MXC_OWM, MATCHADDR); // require address
    //for  (int i=0; i<ADDRSIZE; i++) OWM_WriteByte(MXC_OWM, long_address[i]); // Host sends all 8 Bytes of Address, LSb to MSb
    //OWM_WriteByte(MXC_OWM, WRITE_SCRATCHPAD_1);                         // Host sends function command to write register scratchpad-1
    //for  (int i=0; i<9; i++) OWM_WriteByte(MXC_OWM, configuration[i]);  //Host sends the updated 9 registers of scratchpad-1
//
    //OWM_Read(MXC_OWM, read_crc, 1);                     //Device sends CRC for the recieved register bytes
    //std::uint8_t crc = CalculateCRC(configuration, 9);  //Calculate CRC based on what should have been sent
    //if (read_crc[0] != crc) return ERROR_CRC;                   //Ensure they are equal
//
    //if (!OWM_Reset(MXC_OWM)) return ERROR_RESET;
    //OWM_WriteByte(MXC_OWM, MATCHADDR); // require address
    //for  (int i=0; i<ADDRSIZE; i++) OWM_WriteByte(MXC_OWM, long_address[i]); // Host sends all 8 Bytes of Address, LSb to MSb
    //OWM_WriteByte(MXC_OWM, COPY_SCRATCHPAD_1);  // Save current scratchpad-1 configuration to EEPROM
    //thread_sleep_for(45);                       // Wait for EEPROM to finish
    return return_val;
};


int TMP1826::ProgramAll (const std::uint8_t configuration[9]) {
    std::uint8_t read_crc[1];

    if (!OWM_Reset(MXC_OWM)) return ERROR_RESET;
    OWM_WriteByte(MXC_OWM, SKIPADDR);

    OWM_WriteByte(MXC_OWM, WRITE_SCRATCHPAD_1);                         // Host sends function command to write register scratchpad-1
    for  (int i=0; i<9; i++) OWM_WriteByte(MXC_OWM, configuration[i]);  //Host sends the updated 9 registers of scratchpad-1

    OWM_Read(MXC_OWM, read_crc, 1);                     //Device sends CRC for the recieved register bytes
    std::uint8_t crc = CalculateCRC(configuration, 9);  //Calculate CRC based on what should have been sent
    if (read_crc[0] != crc) return ERROR_CRC;                   //Ensure they are equal

    if (!OWM_Reset(MXC_OWM)) return ERROR_RESET;
    OWM_WriteByte(MXC_OWM, SKIPADDR);

    OWM_WriteByte(MXC_OWM, COPY_SCRATCHPAD_1);  // Save current scratchpad-1 configuration to EEPROM
    thread_sleep_for(45);                       // Wait for EEPROM to finish
    return SUCCESS;
};


std::uint8_t TMP1826::CalculateCRC(const std::uint8_t* data_block, const int num_bytes) {
    std::uint8_t crc = 0x00;
    for (int i=0; i<num_bytes; i++) {
        std::uint8_t byte = data_block[i];
        for (int j=0; j<8; j++) {
            std::uint8_t odd = (crc ^ byte) & 0x01;
            crc >>= 1;
            if (odd) crc ^= 0x8C;
            byte >>= 1;
        }
    }
    return crc; 
};


int TMP1826::SearchDevices(std::uint8_t addresses[][ADDRSIZE], int num){
    for (int n=0; n<num; n++){
        if(!OWM_Reset(MXC_OWM)) return ERROR_RESET;
        OWM_WriteByte(MXC_OWM, SEARCHADDR);
        OWM_Read(MXC_OWM, &addresses[n][0], ADDRSIZE);
        thread_sleep_for(45);
    }
    return SUCCESS;
};


int TMP1826::OneShotConversion() {
    if (!OWM_Reset(MXC_OWM)) return ERROR_RESET;
    OWM_WriteByte(MXC_OWM, SKIPADDR);           // Send SKIP ADDRESS command, enables write to all
    OWM_WriteByte(MXC_OWM, CONVERTTEMP);        // Send TEMP CONVERT function to convert current ADC reading
    if(str_pu)  OWM_SetExtPullup(MXC_OWM, 1);   // For multiple devices on the bus, a low impedance current path is recommended to avoid power loss during convertion
    thread_sleep_for(6);                        // The convertion takes 5.5ms
    if(str_pu)  OWM_SetExtPullup(MXC_OWM, 0);   // Set low again so communication can continue
    return SUCCESS;
}


int TMP1826::GetDebugTemperature(const std::uint8_t address[ADDRSIZE], std::uint8_t read_buffer[18]){ //returns temperature in celcius  , uint8_t* device_address
    for (int n=0; n<5; n++){
        if(OWM_Reset(MXC_OWM)) break;
    } 
    if (address != nullptr) {
        OWM_WriteByte(MXC_OWM, MATCHADDR); // require address
        for  (int i=0; i<ADDRSIZE; i++) OWM_WriteByte(MXC_OWM, address[i]); // Host sends all 8 Bytes of Address, LSb to MSb
    } else {
        OWM_WriteByte(MXC_OWM,  SKIPADDR);
    }
    
    OWM_WriteByte(MXC_OWM, READ_SCRATCHPAD_1);
    OWM_Read(MXC_OWM, read_buffer, 18);

    if(!(read_buffer[2] & 0x08)) return 1;  // Check for update flag
    if(read_buffer[CRC1] != CalculateCRC(read_buffer, CRC1)) return ERROR_CRC;  // Test first checksum
    if(read_buffer[CRC2] != CalculateCRC(read_buffer, CRC2)) return ERROR_CRC;  // Test second checksum
    std::int16_t bit_temperature = (read_buffer[TEMP_RESULT_L] + (read_buffer[TEMP_RESULT_H]<<8));
    return SUCCESS; 
}


float TMP1826::Temperature(std::uint8_t address[ADDRSIZE]) { //returns temperature in celcius  , uint8_t* device_address
    std::uint8_t read_buff[9];
    float result = NAN;

    if (!OWM_Reset(MXC_OWM)) return NAN;
    if (address != nullptr) {
        OWM_WriteByte(MXC_OWM, MATCHADDR); // require address
        for  (int i=0; i<ADDRSIZE; i++) OWM_WriteByte(MXC_OWM, address[i]); // Host sends all 8 Bytes of Address, LSb to MSb
    } else {
        OWM_WriteByte(MXC_OWM,  SKIPADDR);
    }

    OWM_WriteByte(MXC_OWM, READ_SCRATCHPAD_1);
    OWM_Read(MXC_OWM, read_buff, 9);

    if ( !(read_buff[STATUS_REG] & DATA_VALD) ) {
        return NAN;
    } 
    if (read_buff[CRC1] != CalculateCRC(read_buff, CRC1)) {
        return NAN;
    }
    std::int16_t bit_temperature = (read_buff[TEMP_RESULT_L] + (read_buff[TEMP_RESULT_H]<<8));
    return (float)(bit_temperature * BITS_TO_TEMP_16);
}


float TMP1826::FastTemperature(std::uint8_t short_address) { //returns temperature in celcius  , uint8_t* device_address
    std::uint8_t read_buff[2];
    float result = NAN;

    OWM_Reset(MXC_OWM);
    OWM_WriteByte(MXC_OWM, SHORT_ADDR); // require short address
    OWM_WriteByte(MXC_OWM, short_address); 

    OWM_WriteByte(MXC_OWM, READ_SCRATCHPAD_1);
    OWM_Read(MXC_OWM, read_buff, 2);
    return (float)((read_buff[TEMP_RESULT_L] + (read_buff[TEMP_RESULT_H]<<8)) * BITS_TO_TEMP_16);
}
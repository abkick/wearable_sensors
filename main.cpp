
/***** Includes *****/
#include "mbed.h"
#include "OneWire.h"
#include "max32630fthr.h"  // allows for setting pin voltage
#include "TMP_1826.h"
#include <cstdint>

MAX32630FTHR board(MAX32630FTHR::VIO_3V3);  // Sets the microcontroller pins to 3.3 volts

using namespace OneWire;
using namespace RomCommands;

#define NUM_TMP 3  // number of TMP1826 sensors connected to the 1-wire bus
#define WAIT_TIME_MS 300 
DigitalOut led1(LED1);
DigitalOut rLED(LED1, LED_OFF);
DigitalOut gLED(LED2, LED_OFF);
DigitalOut bLED(LED3, LED_OFF);



int main()
{   

    TMP1826 tmp_interface;
    tmp_interface.init();
    //Get 1-Wire Master (owm) instance
    //         (extWeakPullup, extStrongPullup, long_line_mode)
    MCU_OWM owm(false, true, false);
    std::uint8_t address[NUM_TMP][8];
    float temps[NUM_TMP];
    
    //Make sure owm is initialized
    OneWireMaster::CmdResult result = owm.OWInitMaster();
    if(!result) printf("\nOneWireMaster initializated\n");
    else {
        printf("OneWireMaster failed to init!!\n");
        return 1;
    }
    if(owm.m_owmCfg.long_line_mode) printf("long_line_mode is active!\n");
    if(owm.m_owmCfg.int_pu_en) printf("int_pu_en is active!\n");
    if(owm.m_owmCfg.ext_pu_mode != OWM_EXT_PU_UNUSED) printf("ext_pu_mode is active!\n");
    if(owm.m_owmCfg.overdrive_spec) printf("overdrive_spec is active!\n");

    
    rLED = !rLED;

    owm.OWReset();

    for(int i; i<10; i++){
        rLED = LED_ON;
        if(!owm.OWReset()){
            printf("\nReset response recieved!\n");
            rLED = !rLED;
            gLED = LED_ON;
            break;
        }
        else {
            printf("\nNo response after reset!\n");
        }
        thread_sleep_for(1000);
    }

    thread_sleep_for(2000);
    rLED = LED_OFF;
    gLED = LED_OFF;
    bLED = LED_OFF;

    printf("This is WIP clot detector project running on Mbed OS %d.%d.%d.\n", MBED_MAJOR_VERSION, MBED_MINOR_VERSION, MBED_PATCH_VERSION);
    rLED = LED_ON;
    if(tmp_interface.ProgramAll(tmp_interface.TMP1826_config)) {
        printf("PROGRAMING FAILED!!!!!!!\n");
        return 2;
    } else {
        printf("Programing success\n");
    }

    thread_sleep_for(1000);
    tmp_interface.SearchDevices(address, NUM_TMP);

    printf("%d addresses:", NUM_TMP);
    for(int i=0; i<NUM_TMP; i++){
        printf("\n%d: ", NUM_TMP);
        for(int j=0; j<8; j++) printf("%0x",address[i][j]);
    }
    
    printf("\n");
    
    thread_sleep_for(1000);
    rLED = LED_OFF;
    bLED = LED_ON;
    //if(tmp_interface.ProgramConfig(&address[0][1], tmp_interface.TMP1826_config)) {
    //    printf("PROGRAMING FAILED!!!!!!!\n");
    //    return 2;
    //} else {
    //    printf("Programing success\n");
    //}
    rLED = LED_OFF;
    gLED = LED_OFF;
    bLED = LED_OFF;
    std::uint8_t buffer[18];
    while (true)
    {

        bLED = LED_ON;
        //tmp_interface.OneShotConversion();
        //thread_sleep_for(5);

        //for (int i=0; i<10; i++){
        //    thread_sleep_for(50);
        //    if(!tmp_interface.OneShotConversion()) break;
        //    printf("failed to oneshot convert x%d\n", i);
        //    thread_sleep_for(50);
        //} 
    
        for (int k=0; k<NUM_TMP; k++){
            tmp_interface.GetDebugTemperature(address[k], buffer); // use nullptr if 64bit address is not used
            std::int16_t bit_temperature = (buffer[TMP1826::TEMP_RESULT_L] + (buffer[TMP1826::TEMP_RESULT_H]<<8));
            temps[k] = static_cast<float>(bit_temperature * BITS_TO_TEMP_16);;
        }
        printf("%.2f°C\t%.2f°C\t%.2f°C\n", temps[0], temps[1], temps[2]);
        tmp_interface.OneShotConversion();
        thread_sleep_for(50);
        //printf("//\t//\t//\t//\t//\t//\t//\t//\n");
        //for (int i = 0; i<25; i++) {
        //    temperature = NAN;
        //    temperature = tmp_interface.GetTemperature(address);
        //    printf("temperature with address = %.2f\n", temperature);
        //    //OWM_SetExtPullup(MXC_OWM, 1);
        //    thread_sleep_for(5);
        //    //OWM_SetExtPullup(MXC_OWM, 0);
        //}

        //thread_sleep_for(5000);
        bLED = LED_OFF;
        gLED = LED_ON;
        //printf("//\t//\t//\t//\t//\t//\t//\t//\n");

        //for (int i=0; i<10; i++){
        //    if(!tmp_interface.OneShotConversion()) break;
        //    printf("failed to oneshot convert x%d\n", i);
        //} 

        
        //for (int i = 0; i<25; i++) {
        //    temperature = NAN;
        //    temperature = tmp_interface.Temperature(address);
        //    printf("temperature = %.2f\n", temperature);
        //    //OWM_SetExtPullup(MXC_OWM, 1);
        //    thread_sleep_for(5);
        //    //OWM_SetExtPullup(MXC_OWM, 0);
        //}
        rLED = LED_OFF;
        gLED = LED_OFF;
        bLED = LED_OFF;
        //thread_sleep_for(5000);
    }
}


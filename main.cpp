
/***** Includes *****/
#include "mbed.h"
#include "max32630fthr.h"  // allows for setting pin voltage
#include "TMP_1826.h"
#include <cstdint>
#include "adc.h"
#include "tmr_utils.h"
#include "owm.h"

//// (SD card)
#include "SDBlockDevice.h"
#include "FATFileSystem.h"
#include <chrono>
#include <cstdlib>
/// IMU
#include "bmi160.h"




MAX32630FTHR board(MAX32630FTHR::VIO_3V3);  // Sets the microcontroller pins to 3.3 volts

#define NUM_TMP 16 // number of TMP1826 sensors connected to the 1-wire bus
#define WAIT_TIME_MS 300 

PwmOut buzzer(P5_0);
DigitalOut tmp_rail_reset(P3_0, 1);
DigitalOut rLED(LED1, LED_OFF);
DigitalOut gLED(LED2, LED_OFF);
DigitalOut bLED(LED3, LED_OFF);


// SD card device: pins for MAX32630FTHR
SDBlockDevice sd(P0_5, P0_6, P0_4, P0_7);
FATFileSystem fs("sd");

// Timer for timestamps
Timer sample_timer;

BMI160 imu(P5_7, P6_0);

// SD initialization + CSV header
int init_sd_and_file() {
    int err = sd.init();
    if (err) return err;

    err = fs.mount(&sd);
    if (err) {
        err = fs.reformat(&sd);
        if (err) return err;
    }

    FILE *fp = fopen("/sd/temp_demo.csv", "w");
    if (!fp) return -1;

    fprintf(fp, "time_ms,x_accl,y_accl,z_accl,tp_0,tp_1,tp_2,tp_3\n");
    fclose(fp);
    return 0;
}

/// The capacitors in the tmp1826 sensors keep the onewire bus high resulting in the sensors not resetting with the microcontroller
/// Periodicly discarging the sensors seems to make it more stable, without it the addressing phase can fail after reset
void rail_reset(){
    gLED = LED_ON;
    tmp_rail_reset = 1;
    thread_sleep_for(100);
    tmp_rail_reset = 0;
    gLED = !gLED;
    thread_sleep_for(100);
}


int main()
{   

    rLED = LED_ON;
    printf("BMI160 Init...\n");

    if (!imu.begin()) {
        printf("BMI160 init failed!\n");
        return 0;
    }
    printf("BMI160 ready.\n");
    int16_t acc[3], gyr[3];
    int16_t movcheck[3] =  {0,0,0}; //holder variable for checking value
    int sitting=0;

    tmp_rail_reset = 0;
    TMP1826 tmp_interface;
    tmp_interface.init(false, false); // will also initialize One_wire

    std::uint8_t address[NUM_TMP][8];
    float temps[NUM_TMP];




    OWM_Reset(MXC_OWM);

    for(int i; i<10; i++){
        rLED = LED_ON;
        if(OWM_Reset(MXC_OWM)){
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



    thread_sleep_for(1000);
    rLED = LED_OFF;
    gLED = LED_OFF;
    bLED = LED_OFF;

    printf("This is WIP clot detector project running on Mbed OS %d.%d.%d.\n", MBED_MAJOR_VERSION, MBED_MINOR_VERSION, MBED_PATCH_VERSION);

    thread_sleep_for(1000);
    printf("Now searching for tmp1826 sensors");
    gLED = LED_ON;
    tmp_interface.SearchDevices(address, NUM_TMP);
    thread_sleep_for(10);
    gLED = LED_OFF;
    bLED = LED_ON;
    printf("%d addresses:", NUM_TMP);
    for(int i=0; i<NUM_TMP; i++){
        printf("\n%d: ", i);
        for(int j=0; j<8; j++) printf("%0x",address[i][j]);
    }
    printf("\n");
    thread_sleep_for(500);

    //// Programing only needs to happen once with each tmp1826
    //int status = 0;
    //status = tmp_interface.ProgramConfig(&address[0][1], tmp_interface.TMP1826_config);
    //if(status) {
    //    printf("PROGRAMING FAILED!!!!!!!\nerror code %d\n", status);
    //    return 2;
    //} else {
    //    printf("Programing success\n");
    //}

    rLED = LED_OFF;
    gLED = LED_OFF;
    bLED = LED_OFF;
    std::uint8_t buffer[18];
    tmp_interface.OneShotConversion();
    thread_sleep_for(500);

    //// NEW (DATA SAVING ADDITION)
    bool sd_ok = (init_sd_and_file() == 0);
    if(sd_ok)printf("sd is working\n");
    sample_timer.start();
    //// END NEW

    bool alarm = false;
    int n_loop = 0;
    while (true)
    {
        if (n_loop % 10==9){
            rail_reset();
            tmp_interface.OneShotConversion();
        }
        n_loop++;
        if(alarm){
            for(int i=0; i<3; i++)
            {
                buzzer.period_us(1000000/2000);
                buzzer.write(0.50f); // 50% duty cycle
                thread_sleep_for(100); // 1 beat
                buzzer.period_us(0); // Sound off
                thread_sleep_for(100); // 1 beat
            }
        }
    
        int rnd_led = rand()%3;
        switch (rnd_led) { // flip state (on/off) of a random LED
            case 0: rLED = !rLED; break;
            case 1: gLED = !gLED; break;
            case 2: bLED = !bLED; break;
            default: break;
        }
        
        if (imu.read(acc, gyr)) {
            printf("ACC: %6d %6d %6d   GYR: %6d %6d %6d\n",
                   acc[0], acc[1], acc[2],
                   gyr[0], gyr[1], gyr[2]);
        }

    
        for (int k=0; k<NUM_TMP; k++){
            tmp_interface.GetDebugTemperature(address[k], buffer); // use nullptr if 64bit address is not used
            std::int16_t bit_temperature = (buffer[TMP1826::TEMP_RESULT_L] + (buffer[TMP1826::TEMP_RESULT_H]<<8));
            temps[k] = static_cast<float>(bit_temperature * BITS_TO_TEMP_16);;
            printf("%3.1f\t", temps[k]);
        }
        printf("\n");
        tmp_interface.OneShotConversion();
        thread_sleep_for(1000);

        if(acc[1] > -10000){ // checking to see if leg is horiz or vert
            sitting += 1;
        }
        else{
            sitting = 0;
        }
        float temp_max=0;
        for(int t=0; t<NUM_TMP; t++){ 
            if(temps[t]>temp_max) temp_max = temps[t];
        }

        if(sitting > 10 || temp_max > 35.5){//Alarm flag raise
            alarm = true;
        }
        else{ //Alarm Flag Lower
            alarm = false;
        }

        //// sdcard 
        uint64_t t_ms = sample_timer.read_ms();
        if (sd_ok) {
            FILE *fp = fopen("/sd/temp_demo.csv", "a");
            if (fp) {
                fprintf(fp, "%llu,", t_ms);
                fprintf(fp, "%6d,%6d,%6d",
                   acc[0], acc[1], acc[2]);
                for (int k=0; k<NUM_TMP; k++){
                    fprintf(fp,",%.3f", temps[k]);
                }
                fprintf(fp,"\n");
                fclose(fp);
            } else {
                sd_ok = false;
            }
        }
        //// END


    }
}


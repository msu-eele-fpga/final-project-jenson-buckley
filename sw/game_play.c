#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <errno.h>
#include <string.h>
#include <unistd.h>
#include <signal.h>

// stop button component
#define STOP_BUTTON_OFFSET 0x0

// adc component
#define ADC_CH_0_OFFSET 0x0

// LED driver component
// TODO: update these registers
#define OFF_COLOR_OFFSET 0x0
#define ON_COLOR_OFFSET 0x4
#define STRIP_OFFSET 0x8

// the index of the led on the strip that corrisponds to a win
#define WIN_INDEX 0

// number of addressable LEDs in the strip
#define NUM_LEDS 250

// min and max delay times between updates of the led strip in ms
#define DELAY_MIN 1
#define DELAY_MAX 1000

// loop variable that is set to zero by int_handler()
static volatile int keep_running = 1;

/**
* int_handler() - Cleanup and exit program when cntl-C is entered
*/
void int_handler(int irrelevant)
{
    printf("\nLOOP KILLED!\n");
    keep_running = 0;
}

int main () {
    // define and open sysfs files used to read from and write to registers
    FILE *file_stop_button;
    FILE *file_adc;
    FILE *file_ws2811;
    file_stop_button = fopen("/dev/stop_button" , "rb+" );
    file_adc = fopen("/dev/adc" , "rb+" );
    file_ws2811 = fopen("/dev/ws2811" , "rb+" );

    // ensure all files opened correctly
    if (file_stop_button == NULL) {
        printf("failed to open /dev/stop_button\n");
        exit(1);
    }
    if (file_adc == NULL) {
        printf("failed to open /dev/adc\n");
        exit(1);
    }
    if (file_ws2811 == NULL) {
        printf("failed to open /dev/ws2811\n");
        exit(1);
    }

    size_t ret;
    uint32_t val;

    // Test reading the registers sequentially
    printf("\n************************************\n*");
    printf("* read initial register values\n");
    printf("************************************\n\n");
    ret = fread(&val, 4, 1, file_stop_button);
    printf("stop_button = 0x%x\n", val);

    ret = fread(&val, 4, 1, file_adc);
    printf("adc_ch_0 = 0x%x\n", val);

    ret = fread(&val, 4, 1, file_ws2811);
    printf("off_color = 0x%x\n", val);
    ret = fread(&val, 4, 1, file_ws2811);
    printf("on_color = 0x%x\n", val);
    ret = fread(&val, 4, 1, file_ws2811);
    printf("strip = 0x%x\n", val);

    printf("\n************************************\n*");
    printf("* begin game!\n");
    printf("************************************\n\n");
    
    // initialize game
    uint32_t strip = 0x1;
    uint32_t delay = DELAY_MAX;

    // loop until ctl-c is entered
    signal(SIGINT, int_handler);
    while(keep_running)
    {
        // read ADC values and convert to pwm values
        // NOTE: this is designed for 3.3V supply to the pots
        // the highest value read by the ADC would be
        // max_pot_v / max_adc_v * adc_bits - 1  = 3.3/4.096 * 2^12 - 1 = 3299        ret = fseek(file_adc, ADC_CH_0_OFFSET, SEEK_SET);
        ret = fread(&val, 4, 1, file_adc);
        delay = (uint32_t) (DELAY_MIN + (DELAY_MAX - DELAY_MIN)*((float) val) / 3299.0);

        // check to see if user pressed button and won
        // if they did, pause the game for 5 seconds, then reset the button
        // otherwise, just reset the button
        ret = fseek(file_stop_button, STOP_BUTTON_OFFSET, SEEK_SET);
        ret = fread(&val, 4, 1, file_stop_button);
        if(val==1)
        {
            if(strip == WIN_INDEX)
            {
                usleep(5*1000*1000);
            }
            val = 0x0;
            ret = fseek(file_stop_button, STOP_BUTTON_OFFSET, SEEK_SET);
            ret = fwrite(&val, 4, 1, file_stop_button);
            // We need to "flush" so the OS finishes writing to the file_stop_button before our code continues.
            fflush(file_stop_button);
        }

        // update and write strip values
        strip = strip > NUM_LEDS ? 0 : strip + 2;
        ret = fseek(file_ws2811, STRIP_OFFSET, SEEK_SET);
        ret = fwrite(&strip, 4, 1, file_ws2811);
        fflush(file_ws2811);

        usleep(1000*delay);
    }

    // ON EXIT
    // set all leds to red
    val = 0xFF0000;
    ret = fseek(file_ws2811, OFF_COLOR_OFFSET, SEEK_SET);
    ret = fwrite(&val, 4, 1, file_ws2811);
    fflush(file_ws2811);
    ret = fseek(file_ws2811, ON_COLOR_OFFSET, SEEK_SET);
    ret = fwrite(&val, 4, 1, file_ws2811);
    fflush(file_ws2811);

    // close files
    fclose(file_stop_button);
    fclose(file_adc);
    fclose(file_ws2811);

    return 0;
}
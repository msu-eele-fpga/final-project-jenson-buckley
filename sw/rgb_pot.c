#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <errno.h>
#include <string.h>
#include <unistd.h>
#include <signal.h>

// rgb pwm controller component
#define DUTY_RED_OFFSET 0x0
#define DUTY_GREEN_OFFSET 0x4
#define DUTY_BLUE_OFFSET 0x8
#define BASE_PERIOD_OFFSET 0x12

// adc component
#define ADC_CH_0_OFFSET 0x0
#define ADC_CH_1_OFFSET 0x4
#define ADC_CH_2_OFFSET 0x8

// min and max PWM values
#define PWM_MIN 0x0
#define PWM_MAX 0x8000

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
    FILE *file_pwm_rgb;
    FILE *file_adc;
    file_pwm_rgb = fopen("/dev/file_pwm_rgb" , "rb+" );
    file_adc = fopen("/dev/adc" , "rb+" );

    // ensure all files opened correctly
    if (file_pwm_rgb == NULL) {
        printf("failed to open /dev/file_pwm_rgb\n");
        exit(1);
    }
    if (file_adc == NULL) {
        printf("failed to open /dev/file_adc\n");
        exit(1);
    }

    size_t ret;
    uint32_t val;

    // Test reading the registers sequentially
    printf("\n************************************\n*");
    printf("* read initial register values\n");
    printf("************************************\n\n");
    ret = fread(&val, 4, 1, file_pwm_rgb);
    printf("duty_red = 0x%x\n", val);
    ret = fread(&val, 4, 1, file_pwm_rgb);
    printf("duty_green = 0x%x\n", val);
    ret = fread(&val, 4, 1, file_pwm_rgb);
    printf("duty_blue = 0x%x\n", val);
    ret = fread(&val, 4, 1, file_pwm_rgb);
    printf("base_period = 0x%x\n", val);

    ret = fread(&val, 4, 1, file_adc);
    printf("adc_ch_0 = 0x%x\n", val);
    ret = fread(&val, 4, 1, file_adc);
    printf("adc_ch_1 = 0x%x\n", val);
    ret = fread(&val, 4, 1, file_adc);
    printf("adc_ch_2 = 0x%x\n", val);

    printf("\n************************************\n*");
    printf("* begin looping!\n");
    printf("************************************\n\n");

    // loop until ctl-c is entered
    signal(SIGINT, int_handler);

    // set base period to 1 ms
    val = 0x1000;
    ret = fseek(file_pwm_rgb, BASE_PERIOD_OFFSET, SEEK_SET);
    ret = fwrite(&val, 4, 1, file_pwm_rgb);
    fflush(file_pwm_rgb);

    uint32_t red_pwm;
    uint32_t green_pwm;
    uint32_t blue_pwm;

    while(keep_running)
    {
        // read ADC values and convert to pwm values
        // NOTE: this is designed for 3.3V supply to the pots
        // the highest value read by the ADC would be
        // max_pot_v / max_adc_v * adc_bits - 1  = 3.3/4.096 * 2^12 - 1 = 3299
        ret = fseek(file_adc, ADC_CH_0_OFFSET, SEEK_SET);
        ret = fread(&val, 4, 1, file_adc);
        red_pwm = (uint32_t) (PWM_MIN + (PWM_MAX - PWM_MIN)*((float) val) / 3299.0);
        ret = fseek(file_adc, ADC_CH_1_OFFSET, SEEK_SET);
        ret = fread(&val, 4, 1, file_adc);
        green_pwm = (uint32_t) (PWM_MIN + (PWM_MAX - PWM_MIN)*((float) val) / 3299.0);
        ret = fseek(file_adc, ADC_CH_2_OFFSET, SEEK_SET);
        ret = fread(&val, 4, 1, file_adc);
        blue_pwm = (uint32_t) (PWM_MIN + (PWM_MAX - PWM_MIN)*((float) val) / 3299.0);

        // write pwm values
        ret = fseek(file_pwm_rgb, DUTY_RED_OFFSET, SEEK_SET);
        ret = fwrite(&red_pwm, 4, 1, file_pwm_rgb);
        fflush(file_pwm_rgb);
        ret = fseek(file_pwm_rgb, DUTY_GREEN_OFFSET, SEEK_SET);
        ret = fwrite(&green_pwm, 4, 1, file_pwm_rgb);
        fflush(file_pwm_rgb);
        ret = fseek(file_pwm_rgb, DUTY_BLUE_OFFSET, SEEK_SET);
        ret = fwrite(&blue_pwm, 4, 1, file_pwm_rgb);
        fflush(file_pwm_rgb);
    }

    // ON EXIT
    // run led off
    val = 0x0;
    ret = fseek(file_pwm_rgb, DUTY_RED_OFFSET, SEEK_SET);
    ret = fwrite(&val, 4, 1, file_pwm_rgb);
    fflush(file_pwm_rgb);
    ret = fseek(file_pwm_rgb, DUTY_GREEN_OFFSET, SEEK_SET);
    ret = fwrite(&val, 4, 1, file_pwm_rgb);
    fflush(file_pwm_rgb);
    ret = fseek(file_pwm_rgb, DUTY_BLUE_OFFSET, SEEK_SET);
    ret = fwrite(&val, 4, 1, file_pwm_rgb);
    fflush(file_pwm_rgb);

    // close files
    fclose(file_pwm_rgb);
    fclose(file_adc);

    return 0;
}
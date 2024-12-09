# HDL Folder
## pwm_rgb_controller
Takes the input of three 32 bit registers for all 3 colors. Red, Green, and blue. Also takes the input of a 32 bit register with 31 fractional bits to set the pwm duty cycle. 
**Memory Mapped Registers**
duty_red 
duty_green 
duty_blue 
base_period
**IO**
GPIO0(0) = Red 
GPIO0(2) = Green
GPIO0(4) = Blue

## stop_button
Reads a gpio input connected to a switch with a pullup resistor. Also instatiates the debouncer.vhd, one_pulse.vhd, and synchronizer.vhd from previous projects.
**Memory Mapped Registers**
stop_button
**IO**
GPIO1(1) = stop_button

## ws2811_driver
Sends the required 1mhz signal to the ws2811 led strips. The signal has a 24bit rgb section per led with 8bits per color. data_array is an array with 24 * the amount of leds bits. This input is then looped through and sent to the leds. In ws2811_driver_avalon, the input is taken from two 24 bit registers to set two differnt colors. One color for the 'moving' led and one for the 'stationary' leds. There is also a 32 bit register to set the inde of the moving led.
**Memory Mapped Registers**
led_all
led_single
strip_index
**IO**
GPIO0(2) = led strip output



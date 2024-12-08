# RGB PWM LED driver for the DE10 Nano

This device driver is for an RGB LED with adjustable duty cycles for each color and an adjustable period. 

## Building

The Makefile in this directory cross-compiles the driver. Update the `KDIR` variable to point to your linux-socfpga repository directory.

Run `make` in this directory to build to kernel module.

## Device tree node

Use the following device tree node:
```devicetree
pwm_rgb: pwm_rgb@ff210000 {
compatible = "jensen,pwm_rgb";
reg = <0xff210000 16>;
};
```

## Notes / bugs :bug:
NONE

## Register map

This register map is dumb. Write-only registers are dumb. Having different read/write values at the same address is dumb. And they don't even appear to work (see the previous section).

| Offset | Name         | R/W | Purpose                    |
|--------|--------------|-----|----------------------------|
| 0x0    | duty_red     | R   | Red channel duty cycle     |
| 0x0    | duty_red     | W   | Manually update value      |
| 0x4    | duty_green   | R   | Green channel duty cycle   |
| 0x4    | duty_green   | W   | Manually update value      |
| 0x8    | duty_blue    | R   | Blue channel duty cycle    |
| 0x8    | duty_blue    | W   | Manually update value      |
| 0x12   | base_period  | R   | PWM period                 |
| 0x12   | base_period  | W   | Manually update value      |


## Documentation

- NONE

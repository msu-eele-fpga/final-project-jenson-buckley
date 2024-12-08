# Button Input driver for the DE10 Nano

This device driver is for an push button input that only pushes the register high on a button press. 

## Building

The Makefile in this directory cross-compiles the driver. Update the `KDIR` variable to point to your linux-socfpga repository directory.

Run `make` in this directory to build to kernel module.

## Device tree node

Use the following device tree node:
```devicetree
stop_button: stop_button@ff210000 {
compatible = "jensen,stop_button";
reg = <0xff220000 8>;
};
```

## Notes:
Pressing the button can ONLY set the register to a '1'. It will not set it to a zero once the button is released.

## Register map

This register map is dumb. Write-only registers are dumb. Having different read/write values at the same address is dumb. And they don't even appear to work (see the previous section).

| Offset | Name         | R/W | Purpose                    |
|--------|--------------|-----|----------------------------|
| 0x0    | stop_button  | R   | Stop button                |
| 0x0    | stop_button  | W   | Manually update value      |

## Documentation

- NONE
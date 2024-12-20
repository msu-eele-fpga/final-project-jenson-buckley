-- SPDX-License-Identifier: MIT 
-- Copyright (c) 2017 Ross K. Snider.  All rights reserved.
----------------------------------------------------------------------------
-- Description:  Top level VHDL file for the DE10-Nano
----------------------------------------------------------------------------
-- Author:       Ross K. Snider
-- Company:      Montana State University
-- Create Date:  September 1, 2017
-- Revision:     1.0
-- License: MIT  (opensource.org/licenses/MIT)
----------------------------------------------------------------------------

library IEEE;
use IEEE.STD_LOGIC_1164.all;
use IEEE.NUMERIC_STD.all;
use IEEE.std_logic_unsigned.all;

library altera;
use altera.altera_primitives_components.all;

-----------------------------------------------------------
-- Signal Names are defined in the DE10-Nano User Manual
-- http://de10-nano.terasic.com
-----------------------------------------------------------
entity DE10_Top_Level is
  port (
    ----------------------------------------
    --  CLOCK Inputs
    --  See DE10 Nano User Manual page 23
    ----------------------------------------
    FPGA_CLK1_50 : in std_logic; --! 50 MHz clock input #1
    FPGA_CLK2_50 : in std_logic; --! 50 MHz clock input #2
    FPGA_CLK3_50 : in std_logic; --! 50 MHz clock input #3

    ----------------------------------------
    --  Push Button Inputs (KEY) 
    --  See DE10 Nano User Manual page 24
    --  The KEY push button inputs produce a '0' 
    --  when pressed (asserted)
    --  and produce a '1' in the rest (non-pushed) state
    --  a better label for KEY would be Push_Button_n 
    ----------------------------------------
    KEY : in std_logic_vector(1 downto 0); --! Two Pushbuttons (active low)

    ----------------------------------------
    --  Slide Switch Inputs (SW) 
    --  See DE10 Nano User Manual page 25
    --  The slide switches produce a '0' when
    --  in the down position 
    --  (towards the edge of the board)
    ----------------------------------------
    SW : in std_logic_vector(3 downto 0); --! Four Slide Switches 

    ----------------------------------------
    --  LED Outputs 
    --  See DE10 Nano User Manual page 26
    --  Setting LED to 1 will turn it on
    ----------------------------------------
    LED : out std_logic_vector(7 downto 0); --! Eight LEDs

    ----------------------------------------
    --  GPIO Expansion Headers (40-pin)
    --  See DE10 Nano User Manual page 27
    ----------------------------------------
    Audio_Mini_GPIO_0 : inout std_logic_vector(33 downto 0); --! 34 available I/O pins on GPIO_0
    Audio_Mini_GPIO_1 : inout std_logic_vector(12 downto 0) --! 13 available I/O pins on GPIO_1 
  );
end entity DE10_Top_Level;

architecture DE10Nano_arch of DE10_Top_Level is

  -- Generic constants for WS2811 driver
  constant CLK_PERIOD : time    := 20 ns; 
  constant LED_COUNT  : integer := 250; --Number of LEDs in the WS2811 chain

  -- Signals for the ws2811 driver
  signal data_array  : std_logic_vector((24 * LED_COUNT) - 1 downto 0);
  signal rgb_all     : std_logic_vector(23 downto 0);
  signal rgb_single  : std_logic_vector(23 downto 0);
  signal strip_index : integer range 0 to LED_COUNT - 1 := 0;

  -- Define Components
  component ws2811_driver is
    generic (
      CLK_PERIOD : time;
      LED_COUNT  : integer
    );
    port (
      clk          : in std_logic;
      rst          : in std_logic;
      data_array   : in std_logic_vector((24 * LED_COUNT) - 1 downto 0);
      strip_output : out std_logic
    );
  end component;

begin
  -- Defines a single rgb value 
  rgb_all(23 downto 16) <= (others => '1'); -- Red
  rgb_all(15 downto 8)  <= (others => '0'); -- Green
  rgb_all(7 downto 0)   <= (others => '0'); -- Blue
  
  -- Defines a single rgb value 
  rgb_single(23 downto 16) <= (others => '0'); -- Red
  rgb_single(15 downto 8)  <= (others => '1'); -- Green
  rgb_single(7 downto 0)   <= (others => '0'); -- Blue
  
  -- Sets switches to strip index
  strip_index <= to_integer(unsigned(SW));
  
  process(SW, rgb_single, rgb_all)
    variable i : integer;
  begin
    -- Clear the entire data array first
    data_array <= (others => '0');

    -- Default: Set all LEDs to full red
    for i in 0 to LED_COUNT - 1 loop
      data_array(((i+1) * 24 - 1) downto (i * 24)) <= rgb_all(23 downto 0);
    end loop;

    -- Set specific LED with a different color
  if strip_index < LED_COUNT then
    data_array(((strip_index+1) * 24 - 1) downto (strip_index * 24)) <= rgb_single(23 downto 0);
  end if;
  end process;


  -- Instantiate driver
  DRIVER1 : ws2811_driver
  generic map(
    CLK_PERIOD => CLK_PERIOD,
    LED_COUNT  => LED_COUNT
  )
  port map
  (
    clk          => FPGA_CLK1_50,
    rst          => not KEY(0),
    data_array   => data_array,
    strip_output => Audio_Mini_GPIO_0(0)
  );

  -- Status LEDs
  LED(7 downto 0) <= std_logic_vector(to_unsigned(strip_index, 8));

end architecture DE10Nano_arch;


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
use IEEE.STD_LOGIC_1164.ALL;
use IEEE.NUMERIC_STD.ALL;
use IEEE.std_logic_unsigned.all;

LIBRARY altera;
USE altera.altera_primitives_components.all;

-----------------------------------------------------------
-- Signal Names are defined in the DE10-Nano User Manual
-- http://de10-nano.terasic.com
-----------------------------------------------------------
entity DE10_Top_Level is
    port(
        ----------------------------------------
        --  CLOCK Inputs
        --  See DE10 Nano User Manual page 23
        ----------------------------------------
        FPGA_CLK1_50  : in std_logic; --! 50 MHz clock input #1
        FPGA_CLK2_50  : in std_logic; --! 50 MHz clock input #2
        FPGA_CLK3_50  : in std_logic; --! 50 MHz clock input #3
        
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
        SW  : in std_logic_vector(3 downto 0); --! Four Slide Switches 
        
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
        Audio_Mini_GPIO_1 : inout std_logic_vector(12 downto 0)  --! 13 available I/O pins on GPIO_1 
    );
end entity DE10_Top_Level;

architecture DE10Nano_arch of DE10_Top_Level is
    -- Generic constants for WS2811 driver
    constant CLK_PERIOD : time := 20 ns; -- Clock period for 50 MHz clock
    constant LED_COUNT  : integer := 18; --Number of LEDs in the WS2811 chain
    
    -- Signals for the ws2811 driver
    signal data_array  : std_logic_vector((24 * LED_COUNT) - 1 downto 0);
    signal rgb_value   : std_logic_vector(23 downto 0); -- Single LED RGB value
	 
	 -- Define Components
	 component ws2811_driver is
    generic (
      CLK_PERIOD : time;
		LED_COUNT  : integer
    );
    port (
      clk        : in std_logic;
      rst        : in std_logic;
      data_array : in std_logic_vector((24 * LED_COUNT) - 1 downto 0);
      output     : out std_logic
    );
  end component;
    
begin
    -- Defines a single rgb value based of switches
    rgb_value(23 downto 16) <= (others => SW(2)); -- Red: Controlled by SW(2)
    rgb_value(15 downto 8)  <= (others => SW(1)); -- Green: Controlled by SW(1)
    rgb_value(7 downto 0)   <= (others => SW(0)); -- Blue: Controlled by SW(0)
    
    -- Fill the data array with the same RGB value for all LEDs
    generate_data_array : for i in 0 to LED_COUNT - 1 generate
        data_array((i + 1) * 24 - 1 downto i * 24) <= rgb_value;
    end generate generate_data_array;
    
    -- Instantiate driver
    DRIVER1 : ws2811_driver
    generic map (
        CLK_PERIOD => CLK_PERIOD,
        LED_COUNT    => LED_COUNT
    )
    port map (
        clk        => FPGA_CLK1_50, 
        rst        => not KEY(0),       
        data_array => data_array,   
        output     => Audio_Mini_GPIO_0(0) 
    );
    
    -- Status LEDs
    LED(2 downto 0) <= SW(2 downto 0); 
    LED(7 downto 3) <= (others => '0'); 
	 
end architecture DE10Nano_arch;

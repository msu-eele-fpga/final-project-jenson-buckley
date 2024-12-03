library ieee;
use ieee.std_logic_1164.all;
use ieee.numeric_std.all;
use ieee.math_real.all;
library IEEE;
library std;
use std.standard;

entity pwm_controller is
  generic (
    CLK_PERIOD : time := 20 ns
  );
  port (
    clk : in std_logic;
    rst : in std_logic; -- active high
    -- PWM repetition period in milliseconds;
    -- datatype (W.F) (32.24)
    period : in unsigned(31 downto 0);
    -- PWM duty cycle between [0 1]; out-of-range values are hard-limited
    -- datatype (W.F) (32.31)
    duty_cycle : in std_logic_vector(31 downto 0);
    output     : out std_logic
  );
end entity pwm_controller;

architecture arch of pwm_controller is

  -- assigned data types
  constant PERIOD_INT_BITS : natural := 8;
  constant PERIOD_FRAC_BITS : natural := 24;
  constant DUTY_INT_BITS : natural := 1;
  constant DUTY_FRAC_BITS : natural := 31;

  -- clock frequency as natural (keep in ms as period is provided in ms)
  constant FREQ_INTEGER : natural := integer(real(1 ms / CLK_PERIOD));
  -- bits needed for frequency
  constant FREQ_BITS : natural := natural(ceil(log2(real(FREQ_INTEGER))));
  -- clock frequency as unsigned 
  constant FREQ : unsigned((FREQ_BITS - 1) downto 0) := to_unsigned(FREQ_INTEGER, FREQ_BITS);

  -- counter max
  signal counter_max_fullprec : unsigned((FREQ_BITS + PERIOD_INT_BITS + PERIOD_FRAC_BITS - 1) downto 0);
  signal counter_max_int : natural;
  -- duty cycle max
  signal duty_cycle_max_fullprec : unsigned((FREQ_BITS + DUTY_INT_BITS + DUTY_FRAC_BITS - 1) downto 0);
  signal duty_cycle_max_int : natural;
  -- count
  signal count  : integer := 0;

begin
    -- calculate counter max, accounting for the fact period is provided in milliseconds
    counter_max_fullprec <= freq * period;
    counter_max_int <= to_integer(counter_max_fullprec((FREQ_BITS + PERIOD_INT_BITS + PERIOD_FRAC_BITS - 1) downto PERIOD_FRAC_BITS));
    
    -- calculate duty cycle max
    duty_cycle_max_fullprec <= freq * unsigned(duty_cycle);
    duty_cycle_max_int <= to_integer(duty_cycle_max_fullprec((FREQ_BITS + DUTY_INT_BITS + DUTY_FRAC_BITS - 1) downto DUTY_FRAC_BITS));
    

    OUTPUT_DRIVER : process(clk, rst)
    begin
        if(rst = '1') then
            output <= '0';
        elsif(rising_edge(clk)) then
            count <= count + 1;
            if(count < duty_cycle_max_int) then
                output <= '1';
            elsif(count < counter_max_int-1) then
                output <= '0';
            else
                count <= 0;
                output <= '0';
            end if;
        end if;
    end process OUTPUT_DRIVER;
end architecture arch;
library IEEE;
use IEEE.std_logic_1164.all;
use IEEE.numeric_std.all;

library std;
use std.standard;

entity debouncer is
  generic
  (
    clk_period    : time := 20 ns;
    debounce_time : time
  );
  port
  (
    clk       : in std_ulogic;
    rst       : in std_ulogic;
    input     : in std_ulogic;
    debounced : out std_ulogic
  );
end entity debouncer;

architecture arch of debouncer is

  constant COUNTER_LIMIT : natural                                           := ((debounce_time / clk_period) - 1);
  signal count           : natural range 0 to ((debounce_time / clk_period)) := 0;
  signal inter_out       : std_ulogic                                        := '0';

begin
  debounce : process (clk, rst)
  begin
    if (rst = '1') then
      debounced <= '0';
      count     <= 0;
      inter_out <= '0';
    elsif (rising_edge(clk)) then
      if (count = 0 and input /= inter_out) then
        inter_out <= input;
        debounced <= input;
        count     <= 1;
      elsif (count > 0 and count < COUNTER_LIMIT) then
        count <= count + 1;
      else
        count <= 0;
      end if;
    end if;
  end process debounce;
end architecture arch;
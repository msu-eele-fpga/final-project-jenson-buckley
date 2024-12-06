library IEEE;
use IEEE.std_logic_1164.all;
use IEEE.numeric_std.all;

library std;
use std.standard;

entity one_pulse is
  port
  (
    clk   : in std_ulogic;
    rst   : in std_ulogic;
    input : in std_ulogic;
    pulse : out std_ulogic
  );
end entity one_pulse;

architecture arch of one_pulse is

  signal risen : boolean := false;

begin

  pulser : process (clk, rst)
  begin
    if (rst = '1') then
      risen <= false;
      pulse <= '0';
    elsif (rising_edge(clk)) then
      -- on a rising edge, hit pulse
      if (not risen and input = '1') then
        risen <= true;
        pulse <= '1';
        -- riding a high, knock done pulse
      elsif (risen and input = '1') then
        pulse <= '0';
        -- when low again, prep for another pulse
      else
        risen <= false;
        pulse <= '0';
      end if;
    end if;
  end process pulser;
end architecture arch;
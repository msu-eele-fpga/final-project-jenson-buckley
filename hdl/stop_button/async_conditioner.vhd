library IEEE;
use IEEE.std_logic_1164.all;
use IEEE.numeric_std.all;

library std;
use std.standard;

entity async_conditioner is
  port
  (
    clk   : in std_ulogic;
    rst   : in std_ulogic;
    async : in std_ulogic;
    sync  : out std_ulogic
  );
end entity async_conditioner;

architecture arch of async_conditioner is
  -- componenets, types, signals, constants, procedures

  component synchronizer is
    port
    (
      clk   : in std_ulogic;
      async : in std_ulogic;
      sync  : out std_ulogic
    );
  end component synchronizer;

  component debouncer is
    generic
    (
      clk_period    : time;
      debounce_time : time
    );
    port
    (
      clk       : in std_ulogic;
      rst       : in std_ulogic;
      input     : in std_ulogic;
      debounced : out std_ulogic
    );
  end component debouncer;

  component one_pulse is
    port
    (
      clk   : in std_ulogic;
      rst   : in std_ulogic;
      input : in std_ulogic;
      pulse : out std_ulogic
    );
  end component one_pulse;

  signal post_sync     : std_ulogic := '0';
  signal post_debounce : std_ulogic := '0';

begin

  my_sync : synchronizer
  port map
  (
    clk   => clk,
    async => async,
    sync  => post_sync
  );

  my_debounce : debouncer
  generic
  map (
  clk_period    => 20 ns,
  debounce_time => 1 ms
  )
  port
  map (
  clk       => clk,
  rst       => rst,
  input     => post_sync,
  debounced => post_debounce
  );

  my_pulse : one_pulse
  port
  map (
  clk   => clk,
  rst   => rst,
  input => post_debounce,
  pulse => sync
  );

end architecture arch;
-- altera vhdl_input_version vhdl_2008

library IEEE;
use IEEE.std_logic_1164.all;
use IEEE.numeric_std.all;

library std;
use std.standard;

entity stop_button_avalon is
  port (
    clk : in std_ulogic;
    rst : in std_ulogic;
    -- avalon memory-mapped slave interface
    avs_read      : in std_logic;
    avs_write     : in std_logic;
    avs_address   : in std_logic;
    avs_readdata  : out std_logic_vector(31 downto 0);
    avs_writedata : in std_logic_vector(31 downto 0);
    -- external I/O; export to top-level
    stop_button        : in std_ulogic
  );
end entity stop_button_avalon;

architecture arch of stop_button_avalon is

  -- intermediate signal for the blip from the button to be used in a sensitivity list
  signal blip : std_ulogic;

  -- set the register that holds the stop condition to 0
  -- the LSB will hold the stop state (0 for run; 1 for stop)
  signal stop       : std_logic_vector(31 downto 0) := (others => '0');

  component async_conditioner is
    port
        (
          clk   : in std_ulogic;
          rst   : in std_ulogic;
          async : in std_ulogic;
          sync  : out std_ulogic
        );
  end component async_conditioner;

begin

  CONDITIONER : component async_conditioner
    port map
	 (
		clk => clk,
		rst => rst,
		async => stop_button,
    sync => blip
	  );

  avalon_register_read : process (clk)
  begin
    if rising_edge(clk) and avs_read = '1' then
      case avs_address is
        when '0' =>
          avs_readdata   <= stop;
        when others => avs_readdata <= (others => '0');
      end case;
    end if;
  end process;

  avalon_register_write : process (clk, rst)
  begin
    if rst = '1' then
        stop(0) <= '0';
    elsif rising_edge(clk) then
        -- NOTE: a write will not be allowed to happen when the button is pressed (during 20ms pulse)
        -- this means that the C code would think the game is reset, but the stop_button condition would stay high
        -- this corrisponds to the user hitting the button the instant the game is reset (no bad consequences)
        if blip = '1' then
            stop(0) <= '1';
        elsif avs_write = '1' then
            case avs_address is
                when '0'   => stop <= avs_writedata(31 downto 0);
                when others => null; -- ignore writes to unused registers
            end case;
        end if;
    end if;
  end process;

end architecture arch;
-- altera vhdl_input_version vhdl_2008

library IEEE;
use IEEE.std_logic_1164.all;
use IEEE.numeric_std.all;

library std;
use std.standard;

entity ws_2811_driver_avalon is
  port (
    clk : in std_ulogic;
    rst : in std_ulogic;
    -- avalon memory-mapped slave interface
    avs_read      : in std_logic;
    avs_write     : in std_logic;
    avs_address   : in std_logic_vector(1 downto 0);
    avs_readdata  : out std_logic_vector(31 downto 0);
    avs_writedata : in std_logic_vector(31 downto 0);
    -- external I/O; export to top-level
    strip_output        : out std_logic
  );
end entity ws_2811_driver_avalon;

architecture arch of ws_2811_driver_avalon is

  -- Generic constants for WS2811 driver
  constant CLK_PERIOD : time := 20 ns; -- Clock period for 50 MHz clock
  constant LED_COUNT  : integer := 18; --Number of LEDs in the WS2811 chain
    
  -- Signals for the ws2811 driver
  signal data_array  : std_logic_vector((24 * LED_COUNT) - 1 downto 0);
  
  -- Avalon bus signals
  -- Color of the majority of leds
  signal rgb_all        : std_logic_vector(31 downto 0) := (30 => '1', others => '0');
  -- Color of the single led
  signal rgb_single     : std_logic_vector(31 downto 0) := (30 => '1', others => '0');
  -- Sets which led is the single led
  signal strip_index    : std_logic_vector(31 downto 0) := (30 => '1', others => '0');
  -- Convert strip_index to integer using only the least significant LED_COUNT bits
  signal strip_index_int : integer range 0 to LED_COUNT - 1 := 0;

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
  
  -- ws2811 driver instatiation
  DRIVER1 : ws2811_driver
  generic map(
    CLK_PERIOD => CLK_PERIOD,
    LED_COUNT  => LED_COUNT
  )
  port map
  (
    clk        => clk,
    rst        => rst,
    data_array => data_array,
    strip_output     => strip_output
  );
  
  process(strip_index, rgb_single, rgb_all)
    variable i : integer;
  begin
    -- Default all LEDs to rgb_all
    for i in 0 to LED_COUNT - 1 loop
        data_array((i + 1) * 24 - 1 downto i * 24) <= rgb_all(23 downto 0);
    end loop;

    -- Assign the single LED to rgb_single
    data_array((to_integer(unsigned(strip_index(LED_COUNT - 1 downto 0))) + 1) * 24 - 1 downto 
               to_integer(unsigned(strip_index(LED_COUNT - 1 downto 0))) * 24) <= rgb_single(23 downto 0);
  end process;


  -- Process to read the register from the avalon bus
  avalon_register_read : process (clk)
  begin
    if rising_edge(clk) and avs_read = '1' then
      case avs_address is
        when "00" => avs_readdata   <= rgb_single;
        when "01" => avs_readdata   <= rgb_all;
        when "10" => avs_readdata   <= strip_index;
        when others => avs_readdata <= (others => '0');
      end case;
    end if;
  end process;
  
  -- Process to write to registers on avalon bus
  avalon_register_write : process (clk, rst)
  begin
    if rst = '1' then
      rgb_single  <= (others => '0');
      rgb_all     <= (others => '0');
      strip_index <= (others => '0');
    elsif rising_edge(clk) and avs_write = '1' then
      case avs_address is
        when "00"   => rgb_single  <= avs_writedata(31 downto 0);
        when "01"   => rgb_all     <= avs_writedata(31 downto 0);
        when "10"   => strip_index <= avs_writedata(31 downto 0);
        when others => null; -- ignore writes to unused registers
      end case;
    end if;
  end process;

end architecture arch;
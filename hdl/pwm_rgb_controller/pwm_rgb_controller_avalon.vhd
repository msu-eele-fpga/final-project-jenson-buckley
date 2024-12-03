-- altera vhdl_input_version vhdl_2008

library IEEE;
use IEEE.std_logic_1164.all;
use IEEE.numeric_std.all;

library std;
use std.standard;

entity pwm_rgb_controller_avalon is
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
    red_out        : out std_logic;
    green_out : out std_logic;
    blue_out : out std_logic
  );
end entity pwm_rgb_controller_avalon;

architecture arch of pwm_rgb_controller_avalon is

  -- duty cycles are provided in the following format: (W.F) (32.31)
  -- set duty cycles to 50% initially
  signal reg_red_duty_cycle       : std_logic_vector(31 downto 0) := (30 => '1', others => '0');
  signal reg_green_duty_cycle     : std_logic_vector(31 downto 0) := (30 => '1', others => '0');
  signal reg_blue_duty_cycle      : std_logic_vector(31 downto 0) := (30 => '1', others => '0');

  -- period in milliseconds is provided in the following format: (W.F) (32.24)
  -- set period initially to 1 ms
  signal reg_period : std_logic_vector(31 downto 0) := (24 => '1', others => '0');

  component pwm_controller is
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
  end component pwm_controller;

begin

  RED_PWM_CTL : component pwm_controller
    port map
	 (
		clk => clk,
		rst => rst,
		period => unsigned(reg_period),
		duty_cycle => reg_red_duty_cycle,
		output => red_out
	  );

    GREEN_PWM_CTL : component pwm_controller
      port map
     (
      clk => clk,
      rst => rst,
      period => unsigned(reg_period),
      duty_cycle => reg_green_duty_cycle,
      output => green_out
      );

      BLUE_PWM_CTL : component pwm_controller
        port map
       (
        clk => clk,
        rst => rst,
        period => unsigned(reg_period),
        duty_cycle => reg_blue_duty_cycle,
        output => blue_out
        );

  avalon_register_read : process (clk)
  begin
    if rising_edge(clk) and avs_read = '1' then
      case avs_address is
        when "00" => avs_readdata   <= reg_red_duty_cycle;
        when "01" => avs_readdata   <= reg_green_duty_cycle;
        when "10" => avs_readdata   <= reg_blue_duty_cycle;
        when "11" => avs_readdata <= reg_period;
        when others => avs_readdata <= (others => '0');
      end case;
    end if;
  end process;

  avalon_register_write : process (clk, rst)
  begin
    if rst = '1' then
      reg_red_duty_cycle <= (others => '0');
      reg_green_duty_cycle <= (others => '0');
      reg_blue_duty_cycle <= (others => '0');
      reg_period <= (21 => '1', others => '0');
    elsif rising_edge(clk) and avs_write = '1' then
      case avs_address is
        when "00"   => reg_red_duty_cycle <= avs_writedata(31 downto 0);
        when "01"   => reg_green_duty_cycle         <= avs_writedata(31 downto 0);
        when "10"   => reg_blue_duty_cycle     <= avs_writedata(31 downto 0);
        when "11" => reg_period <= avs_writedata(31 downto 0);
        when others => null; -- ignore writes to unused registers
      end case;
    end if;
  end process;

end architecture arch;
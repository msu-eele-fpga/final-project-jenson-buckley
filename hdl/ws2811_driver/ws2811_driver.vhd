library ieee;
use ieee.std_logic_1164.all;
use ieee.numeric_std.all;

entity ws2811_driver is
    generic (
        CLK_PERIOD : time; -- Clock period in nanoseconds
        LED_COUNT    : integer  -- Number of LEDs in the chain
    );
    port (
        clk         : in std_logic;  -- Input clock
        rst         : in std_logic;  -- Synchronous rst
        data_array  : in std_logic_vector((24 * LED_COUNT) - 1 downto 0); -- RGB values for all LEDs
        output      : out std_logic  -- WS2811 output signal
    );
end ws2811_driver;

architecture behavioral of ws2811_driver is

    -- Timing constants for high-speed mode in clock cycles
    constant T0H : integer := integer(260 ns / CLK_PERIOD);  -- High for "0"
    constant T0L : integer := integer(1000 ns / CLK_PERIOD); -- Low for "0"
    constant T1H : integer := integer(600 ns / CLK_PERIOD);  -- High for "1"
    constant T1L : integer := integer(660 ns / CLK_PERIOD);  -- Low for "1"
    constant LED_BITS     : integer := (24 * LED_COUNT);       -- Total number of bits for all LEDs
    constant rst_PERIOD : integer := integer(50 us / CLK_PERIOD); -- rst time

    -- Internal signals
    signal bit_counter       : integer range 0 to LED_BITS - 1 := 0;
    signal phase_counter     : integer := 0;
    signal rst_counter     : integer := 0;
    signal output_reg        : std_logic := '0';
    signal sending_high      : boolean := true;
    signal current_bit       : std_logic := '0'; -- Now a signal
begin
    process(clk)
    begin
        if rising_edge(clk) then
            if rst = '1' then
                -- rst all counters
                bit_counter <= 0;
                phase_counter <= 0;
                rst_counter <= 0;
                output_reg <= '0';
                sending_high <= true;
            else
                if rst_counter < rst_PERIOD then
                    -- Handle rst period
                    rst_counter <= rst_counter + 1;
                    output_reg <= '0'; -- Keep output low during rst
                elsif bit_counter < LED_BITS then
                    -- Sending bits
                    current_bit <= data_array(LED_BITS - 1 - bit_counter);
                    if sending_high then
                        -- Handle high phase of the bit
                        if (current_bit = '1' and phase_counter < T1H) or
                           (current_bit = '0' and phase_counter < T0H) then
                            phase_counter <= phase_counter + 1;
                        else
                            sending_high <= false; -- Transition to low
                            phase_counter <= 0;
                            output_reg <= '0';
                        end if;
                    else
                        -- Handle low phase of the bit
                        if (current_bit = '1' and phase_counter < T1L) or
                           (current_bit = '0' and phase_counter < T0L) then
                            phase_counter <= phase_counter + 1;
                        else
                            -- Move to the next bit
                            bit_counter <= bit_counter + 1;
                            phase_counter <= 0;
                            sending_high <= true; -- Start next bit high
                            output_reg <= '1';
                        end if;
                    end if;
                else
                    -- All bits sent, start/rst rst period
                    if rst_counter < rst_PERIOD then
                        rst_counter <= rst_counter + 1;
                        output_reg <= '0'; -- Keep low during rst
                    else
                        rst_counter <= 0; -- rst for the next frame
                        bit_counter <= 0; -- Restart sending bits
                    end if;
                end if;
            end if;
        end if;
    end process;

    -- Connect the output signal
    output <= output_reg;

end behavioral;

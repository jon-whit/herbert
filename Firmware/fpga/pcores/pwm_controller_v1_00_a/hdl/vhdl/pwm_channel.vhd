-------------------------------------------------------------------------------
-- PWM Channel
--
-- Single channel, Bi-directional PWM driver
--
-- Copyright Idaho Technonogy, 2008
--
-- Created by Brett Gilbert
--
-------------------------------------------------------------------------------
--                     Definition of Generics
-------------------------------------------------------------------------------
-- C_FAMILY          -- Default family
-- C_COUNT_WIDTH     -- Width of the counter
-------------------------------------------------------------------------------
--                  Definition of Ports
-------------------------------------------------------------------------------
-- rst               -- Reset
-- clk               -- Clock
-- start             -- Load counter reg and begin new period
-- loadReg           -- Duty cycle value - in ticks
-- pwm_pos           -- Positive PWM output
-- pwm_neg           -- Negative PWM output
-------------------------------------------------------------------------------

library ieee;
use ieee.std_logic_1164.all;
use IEEE.numeric_std.all;


library proc_common_v2_00_a;




entity pwm_channel is
  generic
  (
    C_REG_WIDTH : integer := 32
  );
  port(
    rst     : in  std_logic;
    clk     : in  std_logic;
    start   : in  std_logic;
    loadReg : in  std_logic_vector(0 to C_REG_WIDTH-1);
    pwm_pos : out std_logic;
    pwm_neg : out std_logic
    );
end pwm_channel;



architecture pwm_ch_imp of pwm_channel is

  type PwmDirType is (NegPwm, PosPwm);
  signal pwmDir : PwmDirType;

begin
  process (clk)

    variable count : unsigned(0 to C_REG_WIDTH-1);

  begin
    if clk'event and clk = '1' then
      if rst = '1' then
        count   := (others => '0');
        pwm_pos <= '0';
        pwm_neg <= '0';
      elsif start = '1' then
        pwm_pos <= '0';
        pwm_neg <= '0';
        count   := unsigned(loadReg);
        if count(0) = '0' then
          pwmDir <= PosPwm;
        else
          pwmDir <= NegPwm;
        end if;
      elsif count = 0 then
        pwm_pos <= '0';
        pwm_neg <= '0';
      else
        if pwmDir = PosPwm then
          count   := count - 1;
          pwm_pos <= '1';
          pwm_neg <= '0';
        else
          count   := count + 1;
          pwm_pos <= '0';
          pwm_neg <= '1';
        end if;
      end if;
    end if;
  end process;
end pwm_ch_imp;

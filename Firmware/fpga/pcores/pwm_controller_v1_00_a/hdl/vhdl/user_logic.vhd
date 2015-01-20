------------------------------------------------------------------------------
-- user_logic.vhd - entity/architecture pair
------------------------------------------------------------------------------
--
-- ***************************************************************************
-- ** Copyright (c) 1995-2007 Xilinx, Inc.  All rights reserved.            **
-- **                                                                       **
-- ** Xilinx, Inc.                                                          **
-- ** XILINX IS PROVIDING THIS DESIGN, CODE, OR INFORMATION "AS IS"         **
-- ** AS A COURTESY TO YOU, SOLELY FOR USE IN DEVELOPING PROGRAMS AND       **
-- ** SOLUTIONS FOR XILINX DEVICES.  BY PROVIDING THIS DESIGN, CODE,        **
-- ** OR INFORMATION AS ONE POSSIBLE IMPLEMENTATION OF THIS FEATURE,        **
-- ** APPLICATION OR STANDARD, XILINX IS MAKING NO REPRESENTATION           **
-- ** THAT THIS IMPLEMENTATION IS FREE FROM ANY CLAIMS OF INFRINGEMENT,     **
-- ** AND YOU ARE RESPONSIBLE FOR OBTAINING ANY RIGHTS YOU MAY REQUIRE      **
-- ** FOR YOUR IMPLEMENTATION.  XILINX EXPRESSLY DISCLAIMS ANY              **
-- ** WARRANTY WHATSOEVER WITH RESPECT TO THE ADEQUACY OF THE               **
-- ** IMPLEMENTATION, INCLUDING BUT NOT LIMITED TO ANY WARRANTIES OR        **
-- ** REPRESENTATIONS THAT THIS IMPLEMENTATION IS FREE FROM CLAIMS OF       **
-- ** INFRINGEMENT, IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS       **
-- ** FOR A PARTICULAR PURPOSE.                                             **
-- **                                                                       **
-- ***************************************************************************
--
------------------------------------------------------------------------------
-- Filename:          user_logic.vhd
-- Version:           1.00.a
-- Description:       User logic.
-- Date:              Wed Dec 17 11:02:47 2008 (by Create and Import Peripheral Wizard)
-- VHDL Standard:     VHDL'93
------------------------------------------------------------------------------
-- Naming Conventions:
--   active low signals:                    "*_n"
--   clock signals:                         "clk", "clk_div#", "clk_#x"
--   reset signals:                         "rst", "rst_n"
--   generics:                              "C_*"
--   user defined types:                    "*_TYPE"
--   state machine next state:              "*_ns"
--   state machine current state:           "*_cs"
--   combinatorial signals:                 "*_com"
--   pipelined or register delay signals:   "*_d#"
--   counter signals:                       "*cnt*"
--   clock enable signals:                  "*_ce"
--   internal version of output port:       "*_i"
--   device pins:                           "*_pin"
--   ports:                                 "- Names begin with Uppercase"
--   processes:                             "*_PROCESS"
--   component instantiations:              "<ENTITY_>I_<#|FUNC>"
------------------------------------------------------------------------------

-- DO NOT EDIT BELOW THIS LINE --------------------
library ieee;
use ieee.std_logic_1164.all;
use IEEE.numeric_std.all;

library proc_common_v2_00_a;
use proc_common_v2_00_a.proc_common_pkg.all;

-- DO NOT EDIT ABOVE THIS LINE --------------------

--USER libraries added here
library pwm_controller_v1_00_a;
use pwm_controller_v1_00_a.pwm_channel;


------------------------------------------------------------------------------
-- Entity section
------------------------------------------------------------------------------
-- Definition of Generics:
--   C_SLV_DWIDTH                 -- Slave interface data bus width
--   C_NUM_REG                    -- Number of software accessible registers
--   C_NUM_INTR                   -- Number of interrupt event
--
-- Definition of Ports:
--   Bus2IP_Clk                   -- Bus to IP clock
--   Bus2IP_Reset                 -- Bus to IP reset
--   Bus2IP_Data                  -- Bus to IP data bus
--   Bus2IP_BE                    -- Bus to IP byte enables
--   Bus2IP_RdCE                  -- Bus to IP read chip enable
--   Bus2IP_WrCE                  -- Bus to IP write chip enable
--   IP2Bus_Data                  -- IP to Bus data bus
--   IP2Bus_RdAck                 -- IP to Bus read transfer acknowledgement
--   IP2Bus_WrAck                 -- IP to Bus write transfer acknowledgement
--   IP2Bus_Error                 -- IP to Bus error response
--   IP2Bus_IntrEvent             -- IP to Bus interrupt event
------------------------------------------------------------------------------

entity user_logic is
  generic
  (
    -- ADD USER GENERICS BELOW THIS LINE ---------------
    C_PWM_CHANNEL_COUNT            : integer              := 24;
    -- ADD USER GENERICS ABOVE THIS LINE ---------------

    -- DO NOT EDIT BELOW THIS LINE ---------------------
    -- Bus protocol parameters, do not add to or delete
    C_SLV_DWIDTH                   : integer              := 32;
    C_NUM_REG                      : integer              := 30;
    C_NUM_INTR                     : integer              := 1;
    C_FAMILY                       : string               := "virtex5"
    -- DO NOT EDIT ABOVE THIS LINE ---------------------
  );
  port
  (
    -- ADD USER PORTS BELOW THIS LINE ------------------
    pwm_drive_pos : out std_logic_vector(0 to C_PWM_CHANNEL_COUNT-1);
    pwm_drive_neg : out std_logic_vector(0 to C_PWM_CHANNEL_COUNT-1);
    pwm_enable    : out std_logic_vector(0 to C_PWM_CHANNEL_COUNT-1);
    pwm_error     : in  std_logic_vector(0 to C_PWM_CHANNEL_COUNT-1);
    lid_pwm_drive : out std_logic_vector(0 to 1);
    -- ADD USER PORTS ABOVE THIS LINE ------------------

    -- DO NOT EDIT BELOW THIS LINE ---------------------
    -- Bus protocol ports, do not add to or delete
    Bus2IP_Clk                     : in  std_logic;
    Bus2IP_Reset                   : in  std_logic;
    Bus2IP_Data                    : in  std_logic_vector(0 to C_SLV_DWIDTH-1);
    Bus2IP_BE                      : in  std_logic_vector(0 to C_SLV_DWIDTH/8-1);
    Bus2IP_RdCE                    : in  std_logic_vector(0 to C_NUM_REG-1);
    Bus2IP_WrCE                    : in  std_logic_vector(0 to C_NUM_REG-1);
    IP2Bus_Data                    : out std_logic_vector(0 to C_SLV_DWIDTH-1);
    IP2Bus_RdAck                   : out std_logic;
    IP2Bus_WrAck                   : out std_logic;
    IP2Bus_Error                   : out std_logic;
    IP2Bus_IntrEvent               : out std_logic_vector(0 to C_NUM_INTR-1)
    -- DO NOT EDIT ABOVE THIS LINE ---------------------
  );

  attribute SIGIS : string;
  attribute SIGIS of Bus2IP_Clk    : signal is "CLK";
  attribute SIGIS of Bus2IP_Reset  : signal is "RST";

end entity user_logic;

------------------------------------------------------------------------------
-- Architecture section
------------------------------------------------------------------------------

architecture IMP of user_logic is

  --USER signal declarations added here, as needed for user logic

  ------------------------------------------
  -- Signals for user logic slave model s/w accessible register example
  ------------------------------------------
  signal period_reg                     : std_logic_vector(0 to C_SLV_DWIDTH-1);
  signal enable_reg                     : std_logic_vector(0 to C_SLV_DWIDTH-1);
  signal error_reg                      : std_logic_vector(0 to C_SLV_DWIDTH-1);
  signal dutycycle00_reg                : std_logic_vector(0 to C_SLV_DWIDTH-1);
  signal dutycycle01_reg                : std_logic_vector(0 to C_SLV_DWIDTH-1);
  signal dutycycle02_reg                : std_logic_vector(0 to C_SLV_DWIDTH-1);
  signal dutycycle03_reg                : std_logic_vector(0 to C_SLV_DWIDTH-1);
  signal dutycycle04_reg                : std_logic_vector(0 to C_SLV_DWIDTH-1);
  signal dutycycle05_reg                : std_logic_vector(0 to C_SLV_DWIDTH-1);
  signal dutycycle06_reg                : std_logic_vector(0 to C_SLV_DWIDTH-1);
  signal dutycycle07_reg                : std_logic_vector(0 to C_SLV_DWIDTH-1);
  signal dutycycle08_reg                : std_logic_vector(0 to C_SLV_DWIDTH-1);
  signal dutycycle09_reg                : std_logic_vector(0 to C_SLV_DWIDTH-1);
  signal dutycycle10_reg                : std_logic_vector(0 to C_SLV_DWIDTH-1);
  signal dutycycle11_reg                : std_logic_vector(0 to C_SLV_DWIDTH-1);
  signal dutycycle12_reg                : std_logic_vector(0 to C_SLV_DWIDTH-1);
  signal dutycycle13_reg                : std_logic_vector(0 to C_SLV_DWIDTH-1);
  signal dutycycle14_reg                : std_logic_vector(0 to C_SLV_DWIDTH-1);
  signal dutycycle15_reg                : std_logic_vector(0 to C_SLV_DWIDTH-1);
  signal dutycycle16_reg                : std_logic_vector(0 to C_SLV_DWIDTH-1);
  signal dutycycle17_reg                : std_logic_vector(0 to C_SLV_DWIDTH-1);
  signal dutycycle18_reg                : std_logic_vector(0 to C_SLV_DWIDTH-1);
  signal dutycycle19_reg                : std_logic_vector(0 to C_SLV_DWIDTH-1);
  signal dutycycle20_reg                : std_logic_vector(0 to C_SLV_DWIDTH-1);
  signal dutycycle21_reg                : std_logic_vector(0 to C_SLV_DWIDTH-1);
  signal dutycycle22_reg                : std_logic_vector(0 to C_SLV_DWIDTH-1);
  signal dutycycle23_reg                : std_logic_vector(0 to C_SLV_DWIDTH-1);
  signal lid_period_reg                 : std_logic_vector(0 to C_SLV_DWIDTH-1);
  signal lid_dutycycle0_reg             : std_logic_vector(0 to C_SLV_DWIDTH-1);
  signal lid_dutycycle1_reg             : std_logic_vector(0 to C_SLV_DWIDTH-1);
  signal slv_reg_write_sel              : std_logic_vector(0 to 29);
  signal slv_reg_read_sel               : std_logic_vector(0 to 29);
  signal slv_ip2bus_data                : std_logic_vector(0 to C_SLV_DWIDTH-1);
  signal slv_read_ack                   : std_logic;
  signal slv_write_ack                  : std_logic;

  ------------------------------------------
  -- Signals for user logic interrupt example
  ------------------------------------------
  signal intr_counter                   : std_logic_vector(0 to C_NUM_INTR-1);

  signal begin_pwm_period               : std_logic;

begin
  --USER logic implementation added here

  ------------------------------------------
  -- Example code to read/write user logic slave model s/w accessible registers
  --
  -- Note:
  -- The example code presented here is to show you one way of reading/writing
  -- software accessible registers implemented in the user logic slave model.
  -- Each bit of the Bus2IP_WrCE/Bus2IP_RdCE signals is configured to correspond
  -- to one software accessible register by the top level template. For example,
  -- if you have four 32 bit software accessible registers in the user logic,
  -- you are basically operating on the following memory mapped registers:
  --
  --    Bus2IP_WrCE/Bus2IP_RdCE   Memory Mapped Register
  --                     "1000"   C_BASEADDR + 0x0
  --                     "0100"   C_BASEADDR + 0x4
  --                     "0010"   C_BASEADDR + 0x8
  --                     "0001"   C_BASEADDR + 0xC
  --
  ------------------------------------------
  slv_reg_write_sel <= Bus2IP_WrCE(0 to 29);
  slv_reg_read_sel  <= Bus2IP_RdCE(0 to 29);

  slv_write_ack     <= Bus2IP_WrCE(0) or Bus2IP_WrCE(1) or Bus2IP_WrCE(2) or Bus2IP_WrCE(3) or Bus2IP_WrCE(4) or Bus2IP_WrCE(5) or Bus2IP_WrCE(6) or Bus2IP_WrCE(7) or Bus2IP_WrCE(8) or Bus2IP_WrCE(9) or Bus2IP_WrCE(10) or Bus2IP_WrCE(11) or Bus2IP_WrCE(12) or Bus2IP_WrCE(13) or Bus2IP_WrCE(14) or Bus2IP_WrCE(15) or Bus2IP_WrCE(16) or Bus2IP_WrCE(17) or Bus2IP_WrCE(18) or Bus2IP_WrCE(19) or Bus2IP_WrCE(20) or Bus2IP_WrCE(21) or Bus2IP_WrCE(22) or Bus2IP_WrCE(23) or Bus2IP_WrCE(24) or Bus2IP_WrCE(25) or Bus2IP_WrCE(26) or Bus2IP_WrCE(27) or Bus2IP_WrCE(28) or Bus2IP_WrCE(29);
  slv_read_ack      <= Bus2IP_RdCE(0) or Bus2IP_RdCE(1) or Bus2IP_RdCE(2) or Bus2IP_RdCE(3) or Bus2IP_RdCE(4) or Bus2IP_RdCE(5) or Bus2IP_RdCE(6) or Bus2IP_RdCE(7) or Bus2IP_RdCE(8) or Bus2IP_RdCE(9) or Bus2IP_RdCE(10) or Bus2IP_RdCE(11) or Bus2IP_RdCE(12) or Bus2IP_RdCE(13) or Bus2IP_RdCE(14) or Bus2IP_RdCE(15) or Bus2IP_RdCE(16) or Bus2IP_RdCE(17) or Bus2IP_RdCE(18) or Bus2IP_RdCE(19) or Bus2IP_RdCE(20) or Bus2IP_RdCE(21) or Bus2IP_RdCE(22) or Bus2IP_RdCE(23) or Bus2IP_RdCE(24) or Bus2IP_RdCE(25) or Bus2IP_RdCE(26) or Bus2IP_RdCE(27) or Bus2IP_RdCE(28) or Bus2IP_RdCE(29);

  -- implement slave model software accessible register(s)
  SLAVE_REG_WRITE_PROC : process( Bus2IP_Clk ) is
  begin

    if Bus2IP_Clk'event and Bus2IP_Clk = '1' then
      if Bus2IP_Reset = '1' then
        period_reg      <= (others => '0');
        enable_reg      <= (others => '0');
        --error_reg <= (others => '0');
        dutycycle00_reg <= (others => '0');
        dutycycle01_reg <= (others => '0');
        dutycycle02_reg <= (others => '0');
        dutycycle03_reg <= (others => '0');
        dutycycle04_reg <= (others => '0');
        dutycycle05_reg <= (others => '0');
        dutycycle06_reg <= (others => '0');
        dutycycle07_reg <= (others => '0');
        dutycycle08_reg <= (others => '0');
        dutycycle09_reg <= (others => '0');
        dutycycle10_reg <= (others => '0');
        dutycycle11_reg <= (others => '0');
        dutycycle12_reg <= (others => '0');
        dutycycle13_reg <= (others => '0');
        dutycycle14_reg <= (others => '0');
        dutycycle15_reg <= (others => '0');
        dutycycle16_reg <= (others => '0');
        dutycycle17_reg <= (others => '0');
        dutycycle18_reg <= (others => '0');
        dutycycle19_reg <= (others => '0');
        dutycycle20_reg <= (others => '0');
        dutycycle21_reg <= (others => '0');
        dutycycle22_reg <= (others => '0');
        dutycycle23_reg <= (others => '0');
        lid_period_reg  <= (others => '0');
        lid_dutycycle0_reg <= (others => '0');
        lid_dutycycle1_reg <= (others => '0');
      else
        case slv_reg_write_sel is
          when "100000000000000000000000000000" =>
            for byte_index in 0 to (C_SLV_DWIDTH/8)-1 loop
              if ( Bus2IP_BE(byte_index) = '1' ) then
                period_reg(byte_index*8 to byte_index*8+7) <= Bus2IP_Data(byte_index*8 to byte_index*8+7);
              end if;
            end loop;
          when "010000000000000000000000000000" =>
            for byte_index in 0 to (C_SLV_DWIDTH/8)-1 loop
              if ( Bus2IP_BE(byte_index) = '1' ) then
                enable_reg(byte_index*8 to byte_index*8+7) <= Bus2IP_Data(byte_index*8 to byte_index*8+7);
              end if;
            end loop;
          -- Error Reg is read only
          --when "001000000000000000000000000000" =>
          --  for byte_index in 0 to (C_SLV_DWIDTH/8)-1 loop
          --    if ( Bus2IP_BE(byte_index) = '1' ) then
          --      error_reg(byte_index*8 to byte_index*8+7) <= Bus2IP_Data(byte_index*8 to byte_index*8+7);
          --    end if;
          --  end loop;
          when "000100000000000000000000000000" =>
            for byte_index in 0 to (C_SLV_DWIDTH/8)-1 loop
              if ( Bus2IP_BE(byte_index) = '1' ) then
                dutycycle00_reg(byte_index*8 to byte_index*8+7) <= Bus2IP_Data(byte_index*8 to byte_index*8+7);
              end if;
            end loop;
          when "000010000000000000000000000000" =>
            for byte_index in 0 to (C_SLV_DWIDTH/8)-1 loop
              if ( Bus2IP_BE(byte_index) = '1' ) then
                dutycycle01_reg(byte_index*8 to byte_index*8+7) <= Bus2IP_Data(byte_index*8 to byte_index*8+7);
              end if;
            end loop;
          when "000001000000000000000000000000" =>
            for byte_index in 0 to (C_SLV_DWIDTH/8)-1 loop
              if ( Bus2IP_BE(byte_index) = '1' ) then
                dutycycle02_reg(byte_index*8 to byte_index*8+7) <= Bus2IP_Data(byte_index*8 to byte_index*8+7);
              end if;
            end loop;
          when "000000100000000000000000000000" =>
            for byte_index in 0 to (C_SLV_DWIDTH/8)-1 loop
              if ( Bus2IP_BE(byte_index) = '1' ) then
                dutycycle03_reg(byte_index*8 to byte_index*8+7) <= Bus2IP_Data(byte_index*8 to byte_index*8+7);
              end if;
            end loop;
          when "000000010000000000000000000000" =>
            for byte_index in 0 to (C_SLV_DWIDTH/8)-1 loop
              if ( Bus2IP_BE(byte_index) = '1' ) then
                dutycycle04_reg(byte_index*8 to byte_index*8+7) <= Bus2IP_Data(byte_index*8 to byte_index*8+7);
              end if;
            end loop;
          when "000000001000000000000000000000" =>
            for byte_index in 0 to (C_SLV_DWIDTH/8)-1 loop
              if ( Bus2IP_BE(byte_index) = '1' ) then
                dutycycle05_reg(byte_index*8 to byte_index*8+7) <= Bus2IP_Data(byte_index*8 to byte_index*8+7);
              end if;
            end loop;
          when "000000000100000000000000000000" =>
            for byte_index in 0 to (C_SLV_DWIDTH/8)-1 loop
              if ( Bus2IP_BE(byte_index) = '1' ) then
                dutycycle06_reg(byte_index*8 to byte_index*8+7) <= Bus2IP_Data(byte_index*8 to byte_index*8+7);
              end if;
            end loop;
          when "000000000010000000000000000000" =>
            for byte_index in 0 to (C_SLV_DWIDTH/8)-1 loop
              if ( Bus2IP_BE(byte_index) = '1' ) then
                dutycycle07_reg(byte_index*8 to byte_index*8+7) <= Bus2IP_Data(byte_index*8 to byte_index*8+7);
              end if;
            end loop;
          when "000000000001000000000000000000" =>
            for byte_index in 0 to (C_SLV_DWIDTH/8)-1 loop
              if ( Bus2IP_BE(byte_index) = '1' ) then
                dutycycle08_reg(byte_index*8 to byte_index*8+7) <= Bus2IP_Data(byte_index*8 to byte_index*8+7);
              end if;
            end loop;
          when "000000000000100000000000000000" =>
            for byte_index in 0 to (C_SLV_DWIDTH/8)-1 loop
              if ( Bus2IP_BE(byte_index) = '1' ) then
                dutycycle09_reg(byte_index*8 to byte_index*8+7) <= Bus2IP_Data(byte_index*8 to byte_index*8+7);
              end if;
            end loop;
          when "000000000000010000000000000000" =>
            for byte_index in 0 to (C_SLV_DWIDTH/8)-1 loop
              if ( Bus2IP_BE(byte_index) = '1' ) then
                dutycycle10_reg(byte_index*8 to byte_index*8+7) <= Bus2IP_Data(byte_index*8 to byte_index*8+7);
              end if;
            end loop;
          when "000000000000001000000000000000" =>
            for byte_index in 0 to (C_SLV_DWIDTH/8)-1 loop
              if ( Bus2IP_BE(byte_index) = '1' ) then
                dutycycle11_reg(byte_index*8 to byte_index*8+7) <= Bus2IP_Data(byte_index*8 to byte_index*8+7);
              end if;
            end loop;
          when "000000000000000100000000000000" =>
            for byte_index in 0 to (C_SLV_DWIDTH/8)-1 loop
              if ( Bus2IP_BE(byte_index) = '1' ) then
                dutycycle12_reg(byte_index*8 to byte_index*8+7) <= Bus2IP_Data(byte_index*8 to byte_index*8+7);
              end if;
            end loop;
          when "000000000000000010000000000000" =>
            for byte_index in 0 to (C_SLV_DWIDTH/8)-1 loop
              if ( Bus2IP_BE(byte_index) = '1' ) then
                dutycycle13_reg(byte_index*8 to byte_index*8+7) <= Bus2IP_Data(byte_index*8 to byte_index*8+7);
              end if;
            end loop;
          when "000000000000000001000000000000" =>
            for byte_index in 0 to (C_SLV_DWIDTH/8)-1 loop
              if ( Bus2IP_BE(byte_index) = '1' ) then
                dutycycle14_reg(byte_index*8 to byte_index*8+7) <= Bus2IP_Data(byte_index*8 to byte_index*8+7);
              end if;
            end loop;
          when "000000000000000000100000000000" =>
            for byte_index in 0 to (C_SLV_DWIDTH/8)-1 loop
              if ( Bus2IP_BE(byte_index) = '1' ) then
                dutycycle15_reg(byte_index*8 to byte_index*8+7) <= Bus2IP_Data(byte_index*8 to byte_index*8+7);
              end if;
            end loop;
          when "000000000000000000010000000000" =>
            for byte_index in 0 to (C_SLV_DWIDTH/8)-1 loop
              if ( Bus2IP_BE(byte_index) = '1' ) then
                dutycycle16_reg(byte_index*8 to byte_index*8+7) <= Bus2IP_Data(byte_index*8 to byte_index*8+7);
              end if;
            end loop;
          when "000000000000000000001000000000" =>
            for byte_index in 0 to (C_SLV_DWIDTH/8)-1 loop
              if ( Bus2IP_BE(byte_index) = '1' ) then
                dutycycle17_reg(byte_index*8 to byte_index*8+7) <= Bus2IP_Data(byte_index*8 to byte_index*8+7);
              end if;
            end loop;
          when "000000000000000000000100000000" =>
            for byte_index in 0 to (C_SLV_DWIDTH/8)-1 loop
              if ( Bus2IP_BE(byte_index) = '1' ) then
                dutycycle18_reg(byte_index*8 to byte_index*8+7) <= Bus2IP_Data(byte_index*8 to byte_index*8+7);
              end if;
            end loop;
          when "000000000000000000000010000000" =>
            for byte_index in 0 to (C_SLV_DWIDTH/8)-1 loop
              if ( Bus2IP_BE(byte_index) = '1' ) then
                dutycycle19_reg(byte_index*8 to byte_index*8+7) <= Bus2IP_Data(byte_index*8 to byte_index*8+7);
              end if;
            end loop;
          when "000000000000000000000001000000" =>
            for byte_index in 0 to (C_SLV_DWIDTH/8)-1 loop
              if ( Bus2IP_BE(byte_index) = '1' ) then
                dutycycle20_reg(byte_index*8 to byte_index*8+7) <= Bus2IP_Data(byte_index*8 to byte_index*8+7);
              end if;
            end loop;
          when "000000000000000000000000100000" =>
            for byte_index in 0 to (C_SLV_DWIDTH/8)-1 loop
              if ( Bus2IP_BE(byte_index) = '1' ) then
                dutycycle21_reg(byte_index*8 to byte_index*8+7) <= Bus2IP_Data(byte_index*8 to byte_index*8+7);
              end if;
            end loop;
          when "000000000000000000000000010000" =>
            for byte_index in 0 to (C_SLV_DWIDTH/8)-1 loop
              if ( Bus2IP_BE(byte_index) = '1' ) then
                dutycycle22_reg(byte_index*8 to byte_index*8+7) <= Bus2IP_Data(byte_index*8 to byte_index*8+7);
              end if;
            end loop;
          when "000000000000000000000000001000" =>
            for byte_index in 0 to (C_SLV_DWIDTH/8)-1 loop
              if ( Bus2IP_BE(byte_index) = '1' ) then
                dutycycle23_reg(byte_index*8 to byte_index*8+7) <= Bus2IP_Data(byte_index*8 to byte_index*8+7);
              end if;
            end loop;
          when "000000000000000000000000000100" =>
            for byte_index in 0 to (C_SLV_DWIDTH/8)-1 loop
              if ( Bus2IP_BE(byte_index) = '1' ) then
                lid_period_reg(byte_index*8 to byte_index*8+7) <= Bus2IP_Data(byte_index*8 to byte_index*8+7);
              end if;
            end loop;
          when "000000000000000000000000000010" =>
            for byte_index in 0 to (C_SLV_DWIDTH/8)-1 loop
              if ( Bus2IP_BE(byte_index) = '1' ) then
                lid_dutycycle0_reg(byte_index*8 to byte_index*8+7) <= Bus2IP_Data(byte_index*8 to byte_index*8+7);
              end if;
            end loop;
          when "000000000000000000000000000001" =>
            for byte_index in 0 to (C_SLV_DWIDTH/8)-1 loop
              if ( Bus2IP_BE(byte_index) = '1' ) then
                lid_dutycycle1_reg(byte_index*8 to byte_index*8+7) <= Bus2IP_Data(byte_index*8 to byte_index*8+7);
              end if;
            end loop;
          when others => null;
        end case;
      end if;
    end if;

  end process SLAVE_REG_WRITE_PROC;

  -- implement slave model software accessible register(s) read mux
  with slv_reg_read_sel select
    slv_ip2bus_data <= period_reg         when "100000000000000000000000000000",
                       enable_reg         when "010000000000000000000000000000",
                       error_reg          when "001000000000000000000000000000",
                       dutycycle00_reg    when "000100000000000000000000000000",
                       dutycycle01_reg    when "000010000000000000000000000000",
                       dutycycle02_reg    when "000001000000000000000000000000",
                       dutycycle03_reg    when "000000100000000000000000000000",
                       dutycycle04_reg    when "000000010000000000000000000000",
                       dutycycle05_reg    when "000000001000000000000000000000",
                       dutycycle06_reg    when "000000000100000000000000000000",
                       dutycycle07_reg    when "000000000010000000000000000000",
                       dutycycle08_reg    when "000000000001000000000000000000",
                       dutycycle09_reg    when "000000000000100000000000000000",
                       dutycycle10_reg    when "000000000000010000000000000000",
                       dutycycle11_reg    when "000000000000001000000000000000",
                       dutycycle12_reg    when "000000000000000100000000000000",
                       dutycycle13_reg    when "000000000000000010000000000000",
                       dutycycle14_reg    when "000000000000000001000000000000",
                       dutycycle15_reg    when "000000000000000000100000000000",
                       dutycycle16_reg    when "000000000000000000010000000000",
                       dutycycle17_reg    when "000000000000000000001000000000",
                       dutycycle18_reg    when "000000000000000000000100000000",
                       dutycycle19_reg    when "000000000000000000000010000000",
                       dutycycle20_reg    when "000000000000000000000001000000",
                       dutycycle21_reg    when "000000000000000000000000100000",
                       dutycycle22_reg    when "000000000000000000000000010000",
                       dutycycle23_reg    when "000000000000000000000000001000",
                       lid_period_reg     when "000000000000000000000000000100",
                       lid_dutycycle0_reg when "000000000000000000000000000010",
                       lid_dutycycle1_reg when "000000000000000000000000000001",
                       (others => '0')    when others;


  ------------------------------------------
  -- Example code to generate user logic interrupts
  --
  -- Note:
  -- The example code presented here is to show you one way of generating
  -- interrupts from the user logic. This code snippet infers a counter
  -- and generate the interrupts whenever the counter rollover (the counter
  -- will rollover ~21 sec @50Mhz).
  ------------------------------------------
  INTR_PROC : process( Bus2IP_Clk ) is
    constant COUNT_SIZE : integer := 30;
    constant ALL_ONES   : std_logic_vector(0 to COUNT_SIZE-1) := (others => '1');
    variable counter    : unsigned(0 to COUNT_SIZE-1);

  begin

    if ( Bus2IP_Clk'event and Bus2IP_Clk = '1' ) then
      if ( Bus2IP_Reset = '1' ) then
        counter := (others => '0');
        intr_counter <= (others => '0');
      else
        counter := counter + 1;
        if ( counter = unsigned(ALL_ONES) ) then
          intr_counter <= (others => '1');
        else
          intr_counter <= (others => '0');
        end if;
      end if;
    end if;

  end process INTR_PROC;

  IP2Bus_IntrEvent <= intr_counter;



  ------------------------------------------
  -- Drive IP to Bus signals
  ------------------------------------------
  IP2Bus_Data  <= slv_ip2bus_data when slv_read_ack = '1' else
                  (others => '0');

  IP2Bus_WrAck <= slv_write_ack;
  IP2Bus_RdAck <= slv_read_ack;
  IP2Bus_Error <= '0';



  ------------------------------------------
  -- PWM Enable signals
  ------------------------------------------

  pwm_enable <= enable_reg;



  ------------------------------------------
  -- PWM Error signals
  ------------------------------------------

  error_reg <= not pwm_error;



  ------------------------------------------
  -- PWM Period
  ------------------------------------------

  PWM_PERIOD_PROC : process(Bus2IP_Reset, Bus2IP_Clk, period_reg, begin_pwm_period) is
    variable count : unsigned(0 to C_SLV_DWIDTH-1);
  begin

    if ( Bus2IP_Clk'event and Bus2IP_Clk = '1' ) then
      if ( Bus2IP_Reset = '1' ) then
        count := (others => '0');
        begin_pwm_period <= '0';
      elsif (count = 0) then
        count := unsigned(period_reg);
        begin_pwm_period <= '1';
      else
        count := count - 1;
        begin_pwm_period <= '0';
      end if;
    end if;
  end process PWM_PERIOD_PROC;



  ------------------------------------------
  -- PWM Channels
  ------------------------------------------

  CH_0: entity pwm_controller_v1_00_a.pwm_channel
  generic map (
              C_REG_WIDTH   => C_SLV_DWIDTH
              )
      port map(
          rst       => Bus2IP_Reset,
          clk       => Bus2IP_CLk,
          start     => begin_pwm_period,
          loadReg   => dutycycle00_reg,
          pwm_pos   => pwm_drive_pos(23),
          pwm_neg   => pwm_drive_neg(23)
          );



  CH_1: entity pwm_controller_v1_00_a.pwm_channel
  generic map (
              C_REG_WIDTH   => C_SLV_DWIDTH
              )
      port map(
          rst       => Bus2IP_Reset,
          clk       => Bus2IP_CLk,
          start     => begin_pwm_period,
          loadReg   => dutycycle01_reg,
          pwm_pos   => pwm_drive_pos(22),
          pwm_neg   => pwm_drive_neg(22)
          );



  CH_2: entity pwm_controller_v1_00_a.pwm_channel
  generic map (
              C_REG_WIDTH   => C_SLV_DWIDTH
              )
      port map(
          rst       => Bus2IP_Reset,
          clk       => Bus2IP_CLk,
          start     => begin_pwm_period,
          loadReg   => dutycycle02_reg,
          pwm_pos   => pwm_drive_pos(21),
          pwm_neg   => pwm_drive_neg(21)
          );



  CH_3: entity pwm_controller_v1_00_a.pwm_channel
  generic map (
              C_REG_WIDTH   => C_SLV_DWIDTH
              )
      port map(
          rst       => Bus2IP_Reset,
          clk       => Bus2IP_CLk,
          start     => begin_pwm_period,
          loadReg   => dutycycle03_reg,
          pwm_pos   => pwm_drive_pos(20),
          pwm_neg   => pwm_drive_neg(20)
          );



  CH_4: entity pwm_controller_v1_00_a.pwm_channel
  generic map (
              C_REG_WIDTH   => C_SLV_DWIDTH
              )
      port map(
          rst       => Bus2IP_Reset,
          clk       => Bus2IP_CLk,
          start     => begin_pwm_period,
          loadReg   => dutycycle04_reg,
          pwm_pos   => pwm_drive_pos(19),
          pwm_neg   => pwm_drive_neg(19)
          );



  CH_5: entity pwm_controller_v1_00_a.pwm_channel
  generic map (
              C_REG_WIDTH   => C_SLV_DWIDTH
              )
      port map(
          rst       => Bus2IP_Reset,
          clk       => Bus2IP_CLk,
          start     => begin_pwm_period,
          loadReg   => dutycycle05_reg,
          pwm_pos   => pwm_drive_pos(18),
          pwm_neg   => pwm_drive_neg(18)
          );



  CH_6: entity pwm_controller_v1_00_a.pwm_channel
  generic map (
              C_REG_WIDTH   => C_SLV_DWIDTH
              )
      port map(
          rst       => Bus2IP_Reset,
          clk       => Bus2IP_CLk,
          start     => begin_pwm_period,
          loadReg   => dutycycle06_reg,
          pwm_pos   => pwm_drive_pos(17),
          pwm_neg   => pwm_drive_neg(17)
          );



  CH_7: entity pwm_controller_v1_00_a.pwm_channel
  generic map (
              C_REG_WIDTH   => C_SLV_DWIDTH
              )
      port map(
          rst       => Bus2IP_Reset,
          clk       => Bus2IP_CLk,
          start     => begin_pwm_period,
          loadReg   => dutycycle07_reg,
          pwm_pos   => pwm_drive_pos(16),
          pwm_neg   => pwm_drive_neg(16)
          );



  CH_8: entity pwm_controller_v1_00_a.pwm_channel
  generic map (
              C_REG_WIDTH   => C_SLV_DWIDTH
              )
      port map(
          rst       => Bus2IP_Reset,
          clk       => Bus2IP_CLk,
          start     => begin_pwm_period,
          loadReg   => dutycycle08_reg,
          pwm_pos   => pwm_drive_pos(15),
          pwm_neg   => pwm_drive_neg(15)
          );



  CH_9: entity pwm_controller_v1_00_a.pwm_channel
  generic map (
              C_REG_WIDTH   => C_SLV_DWIDTH
              )
      port map(
          rst       => Bus2IP_Reset,
          clk       => Bus2IP_CLk,
          start     => begin_pwm_period,
          loadReg   => dutycycle09_reg,
          pwm_pos   => pwm_drive_pos(14),
          pwm_neg   => pwm_drive_neg(14)
          );



  CH_10: entity pwm_controller_v1_00_a.pwm_channel
  generic map (
              C_REG_WIDTH   => C_SLV_DWIDTH
              )
      port map(
          rst       => Bus2IP_Reset,
          clk       => Bus2IP_CLk,
          start     => begin_pwm_period,
          loadReg   => dutycycle10_reg,
          pwm_pos   => pwm_drive_pos(13),
          pwm_neg   => pwm_drive_neg(13)
          );



  CH_11: entity pwm_controller_v1_00_a.pwm_channel
  generic map (
              C_REG_WIDTH   => C_SLV_DWIDTH
              )
      port map(
          rst       => Bus2IP_Reset,
          clk       => Bus2IP_CLk,
          start     => begin_pwm_period,
          loadReg   => dutycycle11_reg,
          pwm_pos   => pwm_drive_pos(12),
          pwm_neg   => pwm_drive_neg(12)
          );



  CH_12: entity pwm_controller_v1_00_a.pwm_channel
  generic map (
              C_REG_WIDTH   => C_SLV_DWIDTH
              )
      port map(
          rst       => Bus2IP_Reset,
          clk       => Bus2IP_CLk,
          start     => begin_pwm_period,
          loadReg   => dutycycle12_reg,
          pwm_pos   => pwm_drive_pos(11),
          pwm_neg   => pwm_drive_neg(11)
          );







  CH_13: entity pwm_controller_v1_00_a.pwm_channel
  generic map (
              C_REG_WIDTH   => C_SLV_DWIDTH
              )
      port map(
          rst       => Bus2IP_Reset,
          clk       => Bus2IP_CLk,
          start     => begin_pwm_period,
          loadReg   => dutycycle13_reg,
          pwm_pos   => pwm_drive_pos(10),
          pwm_neg   => pwm_drive_neg(10)
          );



  CH_14: entity pwm_controller_v1_00_a.pwm_channel
  generic map (
              C_REG_WIDTH   => C_SLV_DWIDTH
              )
      port map(
          rst       => Bus2IP_Reset,
          clk       => Bus2IP_CLk,
          start     => begin_pwm_period,
          loadReg   => dutycycle14_reg,
          pwm_pos   => pwm_drive_pos(9),
          pwm_neg   => pwm_drive_neg(9)
          );



  CH_15: entity pwm_controller_v1_00_a.pwm_channel
  generic map (
              C_REG_WIDTH   => C_SLV_DWIDTH
              )
      port map(
          rst       => Bus2IP_Reset,
          clk       => Bus2IP_CLk,
          start     => begin_pwm_period,
          loadReg   => dutycycle15_reg,
          pwm_pos   => pwm_drive_pos(8),
          pwm_neg   => pwm_drive_neg(8)
          );



  CH_16: entity pwm_controller_v1_00_a.pwm_channel
  generic map (
              C_REG_WIDTH   => C_SLV_DWIDTH
              )
      port map(
          rst       => Bus2IP_Reset,
          clk       => Bus2IP_CLk,
          start     => begin_pwm_period,
          loadReg   => dutycycle16_reg,
          pwm_pos   => pwm_drive_pos(7),
          pwm_neg   => pwm_drive_neg(7)
          );



  CH_17: entity pwm_controller_v1_00_a.pwm_channel
  generic map (
              C_REG_WIDTH   => C_SLV_DWIDTH
              )
      port map(
          rst       => Bus2IP_Reset,
          clk       => Bus2IP_CLk,
          start     => begin_pwm_period,
          loadReg   => dutycycle17_reg,
          pwm_pos   => pwm_drive_pos(6),
          pwm_neg   => pwm_drive_neg(6)
          );



  CH_18: entity pwm_controller_v1_00_a.pwm_channel
  generic map (
              C_REG_WIDTH   => C_SLV_DWIDTH
              )
      port map(
          rst       => Bus2IP_Reset,
          clk       => Bus2IP_CLk,
          start     => begin_pwm_period,
          loadReg   => dutycycle18_reg,
          pwm_pos   => pwm_drive_pos(5),
          pwm_neg   => pwm_drive_neg(5)
          );



  CH_19: entity pwm_controller_v1_00_a.pwm_channel
  generic map (
              C_REG_WIDTH   => C_SLV_DWIDTH
              )
      port map(
          rst       => Bus2IP_Reset,
          clk       => Bus2IP_CLk,
          start     => begin_pwm_period,
          loadReg   => dutycycle19_reg,
          pwm_pos   => pwm_drive_pos(4),
          pwm_neg   => pwm_drive_neg(4)
          );



  CH_20: entity pwm_controller_v1_00_a.pwm_channel
  generic map (
              C_REG_WIDTH   => C_SLV_DWIDTH
              )
      port map(
          rst       => Bus2IP_Reset,
          clk       => Bus2IP_CLk,
          start     => begin_pwm_period,
          loadReg   => dutycycle20_reg,
          pwm_pos   => pwm_drive_pos(3),
          pwm_neg   => pwm_drive_neg(3)
          );



  CH_21: entity pwm_controller_v1_00_a.pwm_channel
  generic map (
              C_REG_WIDTH   => C_SLV_DWIDTH
              )
      port map(
          rst       => Bus2IP_Reset,
          clk       => Bus2IP_CLk,
          start     => begin_pwm_period,
          loadReg   => dutycycle21_reg,
          pwm_pos   => pwm_drive_pos(2),
          pwm_neg   => pwm_drive_neg(2)
          );



  CH_22: entity pwm_controller_v1_00_a.pwm_channel
  generic map (
              C_REG_WIDTH   => C_SLV_DWIDTH
              )
      port map(
          rst       => Bus2IP_Reset,
          clk       => Bus2IP_CLk,
          start     => begin_pwm_period,
          loadReg   => dutycycle22_reg,
          pwm_pos   => pwm_drive_pos(1),
          pwm_neg   => pwm_drive_neg(1)
          );



  CH_23: entity pwm_controller_v1_00_a.pwm_channel
  generic map (
              C_REG_WIDTH   => C_SLV_DWIDTH
              )
      port map(
          rst       => Bus2IP_Reset,
          clk       => Bus2IP_CLk,
          start     => begin_pwm_period,
          loadReg   => dutycycle23_reg,
          pwm_pos   => pwm_drive_pos(0),
          pwm_neg   => pwm_drive_neg(0)
          );




  ------------------------------------------------------------------------------------
  -- Power Module
  ------------------------------------------------------------------------------------

  LID_PWM_PERIOD_PROC : process(Bus2IP_Reset, Bus2IP_Clk, lid_period_reg, lid_dutycycle0_reg, lid_dutycycle1_reg) is
    variable period_count, dutycycle0_count, dutycycle1_count  : unsigned(0 to C_SLV_DWIDTH-1);

  begin

    if ( Bus2IP_Clk'event and Bus2IP_Clk = '1' ) then
      if ( Bus2IP_Reset = '1' ) then
        period_count     := (others => '0');
        dutycycle0_count := (others => '0');
        dutycycle1_count := (others => '0');
        lid_pwm_drive(0) <= '0';
        lid_pwm_drive(1) <= '0';
      elsif (period_count = 0) then
        period_count     := unsigned(lid_period_reg);
        dutycycle0_count := unsigned(lid_dutycycle0_reg);
        dutycycle1_count := unsigned(lid_dutycycle1_reg);
        lid_pwm_drive(0) <= '0';
        lid_pwm_drive(1) <= '0';
      else
        period_count := period_count - 1;

        if (dutycycle0_count = 0) then
          lid_pwm_drive(0) <= '0';
        else
          dutycycle0_count := dutycycle0_count - 1;
          lid_pwm_drive(0) <= '1';
        end if;

        if (dutycycle1_count = 0) then
          lid_pwm_drive(1) <= '0';
        else
          dutycycle1_count := dutycycle1_count - 1;
          lid_pwm_drive(1) <= '1';
        end if;
      end if;
    end if;
  end process LID_PWM_PERIOD_PROC;

end IMP;

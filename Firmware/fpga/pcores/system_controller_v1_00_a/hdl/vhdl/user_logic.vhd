------------------------------------------------------------------------------
-- user_logic.vhd - entity/architecture pair
------------------------------------------------------------------------------
--
-- ***************************************************************************
-- ** Copyright (c) 1995-2008 Xilinx, Inc.  All rights reserved.            **
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
-- Date:              Fri Jan 22 11:06:40 2010 (by Create and Import Peripheral Wizard)
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
use ieee.numeric_std.all;


library proc_common_v2_00_a;
use proc_common_v2_00_a.proc_common_pkg.all;

-- DO NOT EDIT ABOVE THIS LINE --------------------

--USER libraries added here

------------------------------------------------------------------------------
-- Entity section
------------------------------------------------------------------------------
-- Definition of Generics:
--   C_SLV_DWIDTH                 -- Slave interface data bus width
--   C_NUM_REG                    -- Number of software accessible registers
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
------------------------------------------------------------------------------

entity user_logic is
  generic
  (
    -- ADD USER GENERICS BELOW THIS LINE ---------------



    -------------------------------------------------------------
    -- FPGA Version - Upper 16 bits = Major version
    --              - Lower 16 bits = Minor version
    FPGA_VERSION_NUM : std_logic_vector := X"00000001";
    -------------------------------------------------------------



    -- ADD USER GENERICS ABOVE THIS LINE ---------------

    -- DO NOT EDIT BELOW THIS LINE ---------------------
    -- Bus protocol parameters, do not add to or delete
    C_SLV_DWIDTH                   : integer              := 32;
    C_NUM_REG                      : integer              := 13
    -- DO NOT EDIT ABOVE THIS LINE ---------------------
  );
  port
  (
    -- ADD USER PORTS BELOW THIS LINE ------------------
    bootloader_led  : out std_logic;
    heartbeat_led   : out std_logic;
    activity_led    : out std_logic;
    debug_leds      : out std_logic_vector(0 to 7);
    dip_sw          : in  std_logic_vector(0 to 3);
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
    IP2Bus_Error                   : out std_logic
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
  signal bootloader_led_reg             : std_logic;
  signal heartbeat_led_reg              : std_logic;
  signal activity_led_reg               : std_logic;
  signal debug_led_reg                  : std_logic_vector(0 to 7);
  signal dip_sw_reg                     : std_logic_vector(0 to 3);
  signal slv_reg_write_sel              : std_logic_vector(0 to 12);
  signal slv_reg_read_sel               : std_logic_vector(0 to 12);
  signal slv_ip2bus_data                : std_logic_vector(0 to C_SLV_DWIDTH-1);
  signal slv_read_ack                   : std_logic;
  signal slv_write_ack                  : std_logic;

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
  slv_reg_write_sel <= Bus2IP_WrCE(0 to 12);
  slv_reg_read_sel  <= Bus2IP_RdCE(0 to 12);
  slv_write_ack     <= Bus2IP_WrCE(0) or Bus2IP_WrCE(1) or Bus2IP_WrCE(2) or Bus2IP_WrCE(3) or Bus2IP_WrCE(4) or Bus2IP_WrCE(5) or Bus2IP_WrCE(6) or Bus2IP_WrCE(7) or Bus2IP_WrCE(8) or Bus2IP_WrCE(9) or Bus2IP_WrCE(10) or Bus2IP_WrCE(11) or Bus2IP_WrCE(12);
  slv_read_ack      <= Bus2IP_RdCE(0) or Bus2IP_RdCE(1) or Bus2IP_RdCE(2) or Bus2IP_RdCE(3) or Bus2IP_RdCE(4) or Bus2IP_RdCE(5) or Bus2IP_RdCE(6) or Bus2IP_RdCE(7) or Bus2IP_RdCE(8) or Bus2IP_RdCE(9) or Bus2IP_RdCE(10) or Bus2IP_RdCE(11) or Bus2IP_RdCE(12);

  -- implement slave model software accessible register(s)
  SLAVE_REG_WRITE_PROC : process( Bus2IP_Clk ) is
  begin

    if Bus2IP_Clk'event and Bus2IP_Clk = '1' then
      if Bus2IP_Reset = '1' then
        bootloader_led_reg <= '0';
        heartbeat_led_reg  <= '0';
        activity_led_reg   <= '0';
        debug_led_reg      <= (others => '0');
      else
        case slv_reg_write_sel is
          -- FPGA Version - read only
          --when "1000000000000" =>
          --  for byte_index in 0 to (C_SLV_DWIDTH/8)-1 loop
          --    if ( Bus2IP_BE(byte_index) = '1' ) then
          --      slv_reg0(byte_index*8 to byte_index*8+7) <= Bus2IP_Data(byte_index*8 to byte_index*8+7);
          --    end if;
          --  end loop;
          when "0100000000000" =>
            if ( Bus2IP_BE(3) = '1' ) then
              bootloader_led_reg <= Bus2IP_Data(31);
            end if;
          when "0010000000000" =>
            if ( Bus2IP_BE(3) = '1' ) then
              heartbeat_led_reg <= Bus2IP_Data(31);
            end if;
          when "0001000000000" =>
            if ( Bus2IP_BE(3) = '1' ) then
              activity_led_reg <= Bus2IP_Data(31);
            end if;
          when "0000100000000" =>
            if ( Bus2IP_BE(3) = '1' ) then
              debug_led_reg(7) <= Bus2IP_Data(31);
            end if;
          when "0000010000000" =>
            if ( Bus2IP_BE(3) = '1' ) then
              debug_led_reg(6) <= Bus2IP_Data(31);
            end if;
          when "0000001000000" =>
            if ( Bus2IP_BE(3) = '1' ) then
              debug_led_reg(5) <= Bus2IP_Data(31);
            end if;
          when "0000000100000" =>
            if ( Bus2IP_BE(3) = '1' ) then
              debug_led_reg(4) <= Bus2IP_Data(31);
            end if;
          when "0000000010000" =>
            if ( Bus2IP_BE(3) = '1' ) then
              debug_led_reg(3) <= Bus2IP_Data(31);
            end if;
          when "0000000001000" =>
            if ( Bus2IP_BE(3) = '1' ) then
              debug_led_reg(2) <= Bus2IP_Data(31);
            end if;
          when "0000000000100" =>
            if ( Bus2IP_BE(3) = '1' ) then
              debug_led_reg(1) <= Bus2IP_Data(31);
            end if;
          when "0000000000010" =>
            if ( Bus2IP_BE(3) = '1' ) then
              debug_led_reg(0) <= Bus2IP_Data(31);
            end if;
          -- DIP Switches - read only
          --when "0000000000001" =>
          --  for byte_index in 0 to (C_SLV_DWIDTH/8)-1 loop
          --    if ( Bus2IP_BE(byte_index) = '1' ) then
          --      slv_reg12(byte_index*8 to byte_index*8+7) <= Bus2IP_Data(byte_index*8 to byte_index*8+7);
          --    end if;
          --  end loop;
          when others => null;
        end case;
      end if;
    end if;

  end process SLAVE_REG_WRITE_PROC;

  -- implement slave model software accessible register(s) read mux
  with slv_reg_read_sel select
    slv_ip2bus_data <=                  FPGA_VERSION_NUM   when "1000000000000",
    "0000000000000000000000000000000" & bootloader_led_reg when "0100000000000",
    "0000000000000000000000000000000" & heartbeat_led_reg  when "0010000000000",
    "0000000000000000000000000000000" & activity_led_reg   when "0001000000000",
    "0000000000000000000000000000000" & debug_led_reg(7)   when "0000100000000",
    "0000000000000000000000000000000" & debug_led_reg(6)   when "0000010000000",
    "0000000000000000000000000000000" & debug_led_reg(5)   when "0000001000000",
    "0000000000000000000000000000000" & debug_led_reg(4)   when "0000000100000",
    "0000000000000000000000000000000" & debug_led_reg(3)   when "0000000010000",
    "0000000000000000000000000000000" & debug_led_reg(2)   when "0000000001000",
    "0000000000000000000000000000000" & debug_led_reg(1)   when "0000000000100",
    "0000000000000000000000000000000" & debug_led_reg(0)   when "0000000000010",
    "0000000000000000000000000000"    & dip_sw_reg         when "0000000000001",
                                        (others => '0')    when others;

  ------------------------------------------
  -- Example code to drive IP to Bus signals
  ------------------------------------------
  IP2Bus_Data  <= slv_ip2bus_data when slv_read_ack = '1' else
                  (others => '0');

  IP2Bus_WrAck <= slv_write_ack;
  IP2Bus_RdAck <= slv_read_ack;
  IP2Bus_Error <= '0';



  ------------------------------------------
  -- External signals
  ------------------------------------------
  bootloader_led <= bootloader_led_reg;
  heartbeat_led  <= heartbeat_led_reg;
  activity_led   <= activity_led_reg;
  debug_leds     <= debug_led_reg;
  dip_sw_reg     <= dip_sw;

end IMP;

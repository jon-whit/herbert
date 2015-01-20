-------------------------------------------------------------------------------
--  $Id: watchdog_timer.vhd,v 1.1.2.1 2008/04/03 15:41:24 jennj Exp $
-------------------------------------------------------------------------------
-- watchdog_timer.vhd - entity/architecture pair
-------------------------------------------------------------------------------
--
--  ***************************************************************************
--  **  Copyright(C) 2003 by Xilinx, Inc. All rights reserved.               **
--  **                                                                       **
--  **  This text contains proprietary, confidential                         **
--  **  information of Xilinx, Inc. , is distributed by                      **
--  **  under license from Xilinx, Inc., and may be used,                    **
--  **  copied and/or disclosed only pursuant to the terms                   **
--  **  of a valid license agreement with Xilinx, Inc.                       **
--  **                                                                       **
--  **  Unmodified source code is guaranteed to place and route,             **
--  **  function and run at speed according to the datasheet                 **
--  **  specification. Source code is provided "as-is", with no              **
--  **  obligation on the part of Xilinx to provide support.                 **
--  **                                                                       **
--  **  Xilinx Hotline support of source code IP shall only include          **
--  **  standard level Xilinx Hotline support, and will only address         **
--  **  issues and questions related to the standard released Netlist        **
--  **  version of the core (and thus indirectly, the original core source). **
--  **                                                                       **
--  **  The Xilinx Support Hotline does not have access to source            **
--  **  code and therefore cannot answer specific questions related          **
--  **  to source HDL. The Xilinx Support Hotline will only be able          **
--  **  to confirm the problem in the Netlist version of the core.           **
--  **                                                                       **
--  **  This copyright and support notice must be retained as part           **
--  **  of this text at all times.                                           **
--  ***************************************************************************
--
-------------------------------------------------------------------------------
-- Filename:        watchdog_timer.vhd
-- Version:         v1.03a
-- Description:     This file contains the watchdog timer for the PLB. It will
--                  generate addrAck if the slave has not responded within 16
--                  clocks after PAValid or SAValid asserts.
--
-- VHDL-Standard:   VHDL'93
-------------------------------------------------------------------------------
-- Structure:  (see plb_v46.vhd)
--
-------------------------------------------------------------------------------
-- Author:      BLT
-- History:
--      ALS     02/20/02        -- created from plb_arbiter_v1_01_a
--      ALS     04/16/02        -- Version v1.01a
-- ~~~~~~
--  FLO         06/08/05        
-- ^^^^^^
-- Start of changes for plb_v46.
-- -Switched to v2_00_a for proc_common.
-- -Eliminated aspects related to the watchdog timer completing hanshakes on
--  timeout.
-- -Removed component declarations in favor of direct entity instantiation.
-- -Changed structure section to a reference to plb_v46.vhd.
-- ~~~~~~
--  FLO         07/19/05        
-- ^^^^^^
-- -Changed to active low WdtMTimeout_n
-- ~~~~~~
--  FLO         08/26/05        
-- ^^^^^^
-- -Added port WdtMTimeout_n_p1.
-- ~~~~~~
--  JLJ         07/17/07    v1.00a   
-- ^^^^^^
--  Add C_SIZE parameter.  Allows watchdog timer size to change for
--  shared and P2P bus modes.
-- ~~~~~
--  JLJ         09/14/07    v1.01a  
-- ^^^^^^
--  Update to v1.01a.
-- ~~~~~~
--  JLJ         10/09/07    v1.02a  
-- ^^^^^^
--  Update to v1.02a.
-- ~~~~~~
--  JLJ         10/31/07    v1.02a  
-- ^^^^^^
--  Remove Abort input signal and usage.
-- ~~~~~~
--  JLJ         03/17/08    v1.03a  
-- ^^^^^^
--  Upgraded to v1.03a. 
-- ~~~~~~
---------------------------------------------------------------------------------
-- 
-- Naming Conventions:
--      active low signals:                     "*_n"
--      clock signals:                          "clk", "clk_div#", "clk_#x" 
--      reset signals:                          "rst", "rst_n" 
--      generics:                               "C_*" 
--      user defined types:                     "*_TYPE" 
--      state machine next state:               "*_ns" 
--      state machine current state:            "*_cs" 
--      combinatorial signals:                  "*_cmb" 
--      pipelined or register delay signals:    "*_d#" 
--      counter signals:                        "*cnt*"
--      clock enable signals:                   "*_ce" 
--      internal version of output port         "*_i"
--      device pins:                            "*_pin" 
--      ports:                                  - Names begin with Uppercase 
--      processes:                              "*_PROCESS" 
--      component instantiations:               "<ENTITY_>I_<#|FUNC>
-------------------------------------------------------------------------------
-- 
 
library ieee;
use ieee.std_logic_1164.all;

library proc_common_v2_00_a;

-------------------------------------------------------------------------------
-- Port Declaration
-------------------------------------------------------------------------------
-------------------------------------------------------------------------------
-- Definition of Generics:
--      No generics used
--
-- Definition of Ports:
--      -- Clock and Reset
--      input  Clk                      
--      input  ArbReset   
--
--      -- Priority Encoder Results
--      input  ArbAddrValidReg          
--
--      -- PLB Transfer signals
--      input  PLB_size                 
--      input  PLB_PAValid          --PLB primary address valid
--      input  PLB_abort                
--
--      -- Slave transfer responses
--      input  Sl_addrAck               
--      input  Sl_rearbitrate           
--      input  Sl_wait                  
--
--      -- Watchdog timer transfer responses
--      output WdtMTimeout_n
--      output WdtMTimeout_n_p1
--
--      -- Watchdog timer status signals 
-------------------------------------------------------------------------------
 
-------------------------------------------------------------------------------
-- Entity Section
-------------------------------------------------------------------------------
entity watchdog_timer is
  generic (
        C_SIZE    : integer   := 4
        );
  port (
        Clk                 : in std_logic;
        ArbReset            : in std_logic;
        PLB_PAValid         : in std_logic;
        --PLB_abort           : in std_logic;
        Sl_addrAck          : in std_logic;
        Sl_rearbitrate      : in std_logic;
        Sl_wait             : in std_logic;
        WdtMTimeout_n       : out std_logic;
        WdtMTimeout_n_p1    : out std_logic
        );
end watchdog_timer;
 
-------------------------------------------------------------------------------
-- Architecture Section
-------------------------------------------------------------------------------
architecture simulation of watchdog_timer is

-------------------------------------------------------------------------------
-- Signal Declarations
-------------------------------------------------------------------------------
-- define internal versions of output signals

signal wdtMTimeout_n_i      : std_logic;
signal wdtMTimeout_n_p1_i   : std_logic;
signal wdtCountIsZero_i     : std_logic;
 
-- TimeOut Counter Signals
signal wdtTimeOutCntrReset  : std_logic;
signal wdtTimeOutCount      : std_logic_vector(0 to C_SIZE-1 );
signal wdtTimeOutCntrEnable : std_logic;
signal wdtTimeOutCntrLoad   : std_logic;
signal wdtTimeOutCntrDin    : std_logic_vector(0 to C_SIZE-1 );


-------------------------------------------------------------------------------
-- Begin architecture
-------------------------------------------------------------------------------
  
begin
 
  WdtMTimeout_n       <= wdtMTimeout_n_i;
  WdtMTimeout_n_p1    <= wdtMTimeout_n_p1_i;
 
-------------------------------------------------------------------------------
--    Transfer Time Out Counter
--
--    This 4-bit down-counter is used to determine when a timeout has occured on
--    the PLB bus.  A timeout on the PLB bus is defined as the slaves not asserting
--    Sl_addrAck, Sl_rearbitrate or Sl_wait or the master not asserting M_abort
--    within 16 cycles of the assertion of the PLB_PAValid
--    signal by the PLB arbiter.
-------------------------------------------------------------------------------

wdtTimeOutCntrLoad <=      not(PLB_PAValid)
--                        or PLB_abort
                        or Sl_addrAck
                        or Sl_rearbitrate
                        or not wdtMTimeout_n_i
                        or ArbReset;

wdtTimeOutCntrReset  <= '0';

wdtTimeOutCntrEnable <= not(Sl_wait);

wdtTimeOutCntrDin   <= (others => '1');

WDT_TIMEOUT_CNTR_I: entity proc_common_v2_00_a.down_counter 
   generic map  (   C_CNT_WIDTH => C_SIZE )
   port map (
                    Din     => wdtTimeOutCntrDin,
                    Load    => wdtTimeOutCntrLoad,
                    Cnt_en  => wdtTimeOutCntrEnable,         
                    Cnt_out => wdtTimeOutCount,
                    Clk     => Clk,
                    Rst     => wdtTimeOutCntrReset
            );
  

-------------------------------------------------------------------------------
--    wdtAddrAck generation
-------------------------------------------------------------------------------

GEN_SIZE_EQ_4: if (C_SIZE = 4) generate
    wdtCountIsZero_i <= not(wdtTimeOutCount(0)) 
                        and not(wdtTimeOutCount(1))
                        and not(wdtTimeOutCount(2)) 
                        and not(wdtTimeOutCount(3));
end generate GEN_SIZE_EQ_4;

--coverage off
GEN_SIZE_EQ_8: if (C_SIZE = 8) generate
    wdtCountIsZero_i <= not(wdtTimeOutCount(0)) 
                        and not(wdtTimeOutCount(1))
                        and not(wdtTimeOutCount(2)) 
                        and not(wdtTimeOutCount(3)) 
                        and not(wdtTimeOutCount(4)) 
                        and not(wdtTimeOutCount(5)) 
                        and not(wdtTimeOutCount(6)) 
                        and not(wdtTimeOutCount(7));
end generate GEN_SIZE_EQ_8;
--coverage on
                    
wdtMTimeout_n_p1_i    <= not(     wdtCountIsZero_i
                              and not(Sl_addrAck)
--                              and not(PLB_abort)
                              and not(Sl_rearbitrate)
                              and not(Sl_wait)
                            );

WDTMTIMEOUT_PROC : process(Clk)
begin
    if Clk'event and Clk = '1' then
        wdtMTimeout_n_i    <= wdtMTimeout_n_p1_i;
    end if;
end process;
                    

end ;


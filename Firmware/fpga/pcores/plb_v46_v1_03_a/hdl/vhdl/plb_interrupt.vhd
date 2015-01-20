-------------------------------------------------------------------------------
--  $Id: plb_interrupt.vhd,v 1.1.2.1 2008/04/03 15:41:24 jennj Exp $
-------------------------------------------------------------------------------
-- plb_interrupt.vhd - entity/architecture pair
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
--  Filename:     plb_interrupt.vhd
--  Version:      v1.03a
--  Description:
--    This module generates an edge-type interrupt whenever a bus error is 
--    detected. The active state of the interrupt is determined by the
--    C_IRQ_ACTIVE parameter. The pulse is one-clock wide.
--
-- VHDL-Standard:   VHDL'93
-------------------------------------------------------------------------------
-- Structure:  (see plb_v46.vhd)
--
-------------------------------------------------------------------------------
-- Author:      
-- History:
--      ALS     02/20/02        -- created from plb_arbiter_v1_01_a
--      ALS     04/16/02        -- Version v1.01a
-- ~~~~~~
--  FLO         06/08/05        
-- ^^^^^^
-- Start of changes for plb_v46.
-- -Switched to v2_00_a for proc_common.
-- -Converted to use new signal WdtMTimeout.
-- -Changed structure section to a reference to plb_v46.vhd.
-- ~~~~~~
--  FLO         07/19/05        
-- ^^^^^^
-- -Changed to active low WdtMTimeout_n
-- ~~~~~~
--  JLJ         09/14/07    v1.01a  
-- ^^^^^^
--  Update to v1.01a.
-- ~~~~~~
--  JLJ         10/09/07    v1.02a  
-- ^^^^^^
--  Update to v1.02a.
-- ~~~~~~
--  JLJ         03/17/08    v1.03a  
-- ^^^^^^
--  Upgraded to v1.03a. 
-- ~~~~~~
------------------------------------------------------------------------------- 
-- Naming Conventions:
--      active low signals:                     "*_n"
--      clock signals:                          "clk", "clk_div#", "clk_#x" 
--      reset signals:                          "rst", "rst_n" 
--      generics:                               "C_*" 
--      user defined types:                     "*_TYPE" 
--      state machine next state:               "*_ns" 
--      state machine current state:            "*_cs" 
--      combinatorial signals:                  "*_com" 
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
 
library unisim;
use unisim.vcomponents.FDRE;
use unisim.vcomponents.FDSE;

library proc_common_v2_00_a;
use proc_common_v2_00_a.proc_common_pkg.RESET_ACTIVE;

-------------------------------------------------------------------------------
-- Port Declaration
-------------------------------------------------------------------------------
-------------------------------------------------------------------------------
-- Definition of Generics:
--      C_IRQ_ACTIVE                -- active edge of interrupt - rising or falling
--
-- Definition of Ports:
--      input  Clk                  -- clock
--      input  Rst                  -- reset
--      input  WdtMTimeout_n        -- watchdog timer timeout
--      input  Intr_en              -- interrupt enable
--
--      output Bus_Error_Det        -- edge interrupt
-------------------------------------------------------------------------------

-------------------------------------------------------------------------------
-- Entity Section
-------------------------------------------------------------------------------
entity plb_interrupt is
    generic( C_IRQ_ACTIVE   : std_logic := '1'
            );
  port (
        Clk             : in std_logic;
        Rst             : in std_logic;
        WdtMTimeout_n   : in std_logic;
        Intr_en         : in std_logic;
        Bus_Error_Det   : out std_logic
        );
end plb_interrupt;
 
-------------------------------------------------------------------------------
-- Architecture Section
-------------------------------------------------------------------------------
architecture plb_interrupt of plb_interrupt is

-------------------------------------------------------------------------------
-- Signal Declarations
-------------------------------------------------------------------------------
-- internal signal for output
signal bus_error_det_i      : std_logic;

-- d input to registers
signal bus_error_det_d      : std_logic;

-- PLB Bus pipeline signals
signal wdtmtimeout_d1       : std_logic;

-------------------------------------------------------------------------------
-- Component Declarations
-------------------------------------------------------------------------------

-------------------------------------------------------------------------------
-- Begin architecture
-------------------------------------------------------------------------------
begin
-- assign internal signal to output
Bus_Error_Det <= bus_error_det_i;

-- register PLB bus input signals
PLB_BUS_REGS: process (Clk)
begin
    if Clk'event and Clk = '1' then
        if Rst = RESET_ACTIVE then
            wdtmtimeout_d1 <= '0';
        else
            wdtmtimeout_d1 <= (not wdtMTimeout_n);
        end if;
    end if;
end process PLB_BUS_REGS;

RISING_EDGE_GEN: if C_IRQ_ACTIVE = '1' generate

    bus_error_det_d <=     wdtmtimeout_d1;
    
    INTERRUPT_REFF_I: component FDRE
      port map (
                Q   => bus_error_det_i,     --[out]
                C   => Clk,                 --[in]
                CE  => Intr_en,             --[in]
                D   => bus_error_det_d,     --[in]
                R   => Rst                  --[in]
                );
end generate RISING_EDGE_GEN;

FALLING_EDGE_GEN: if C_IRQ_ACTIVE = '0' generate

    bus_error_det_d <= not(wdtmtimeout_d1);

    INTERRUPT_FEFF_I: component FDSE
      port map (
                Q   => bus_error_det_i,     --[out]
                C   => Clk,                 --[in]
                CE  => Intr_en,             --[in]
                D   => bus_error_det_d,     --[in]
                S   => Rst                  --[in]
                );
end generate FALLING_EDGE_GEN;

end;


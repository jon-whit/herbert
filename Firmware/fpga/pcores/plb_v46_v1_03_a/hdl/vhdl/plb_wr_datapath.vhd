-------------------------------------------------------------------------------
--  $Id: plb_wr_datapath.vhd,v 1.1.2.1 2008/04/03 15:41:24 jennj Exp $
-------------------------------------------------------------------------------
-- plb_wr_datapath.vhd - entity/architecture pair
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
-- Filename:        plb_wr_datapath.vhd
-- Version:         v1.03a
-- Description:     This file contains the multiplexors that choose which
--                  master's write data bus is output to the slaves. It also
--                  routes either the slave write data ack or the watchdog
--                  timer write data ack to the requesting master.
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
--  FLO         12/02/05        
-- ^^^^^^
-- -Added C_FAMILY generic.
-- -Changed mux_onehot instances to mux_onehot_f.
-- ~~~~~
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
use ieee.STD_LOGIC_1164.all;

library proc_common_v2_00_a;
 
-------------------------------------------------------------------------------
-- Port Declaration
-------------------------------------------------------------------------------
-------------------------------------------------------------------------------
-- Definition of Generics:
--      C_NUM_MASTERS               -- number of masters
--      C_PLB_DWIDTH                -- data bus width
--
-- Definition of Ports:
--      input Sl_wrDack             -- slave write data acknowledge
--      input ArbPriWrMasterReg     -- register holding write master id
--      input M_wrDBus              -- Master write data bus
--
--      output PLB_MWrDAck          -- write data ack to master
--      output PLB_wrDBus           -- write data bus to slaves
-------------------------------------------------------------------------------
 
-------------------------------------------------------------------------------
-- Entity Section
-------------------------------------------------------------------------------
entity plb_wr_datapath is
  generic (C_NUM_MASTERS    : integer := 8;
           C_PLB_DWIDTH     : integer := 128;
           C_FAMILY         : string);
  port (
        Sl_wrDAck           : in STD_LOGIC;
        ArbPriWrMasterReg   : in STD_LOGIC_VECTOR(0 to C_NUM_MASTERS - 1 );
        M_wrDBus            : in STD_LOGIC_VECTOR(0 to (C_NUM_MASTERS*C_PLB_DWIDTH) - 1 );
        PLB_MWrDAck         : out STD_LOGIC_VECTOR(0 to C_NUM_MASTERS - 1 );
        PLB_wrDBus          : out STD_LOGIC_VECTOR(0 to C_PLB_DWIDTH - 1 )
        );
end plb_wr_datapath;
 
 
-------------------------------------------------------------------------------
-- Architecture Section
-------------------------------------------------------------------------------
architecture simulation of plb_wr_datapath is
-------------------------------------------------------------------------------
-- Signal Declarations
-------------------------------------------------------------------------------
-- define a signal for the mux select line
signal wr_dack      : std_logic_vector(0 to 0);


-------------------------------------------------------------------------------
-- Begin architecture
-------------------------------------------------------------------------------
begin

-------------------------------------------------------------------------------
-- Component Instantiations
-------------------------------------------------------------------------------
-- Instantiate the one-hot carry chain mux to multiplex the winning master's 
-- write data onto the PLB_wrDBus. The data width of the bus is the width
-- of the PLB data bus. The number of buses is equal to the number of masters

I_WRDBUS_MUX: entity proc_common_v2_00_a.mux_onehot_f
        generic map (   C_DW => C_PLB_DWIDTH,
                        C_NB => C_NUM_MASTERS,
                        C_FAMILY => C_FAMILY)
        port map    (
                        D => M_wrDBus,
                        S => ArbPriWrMasterReg,
                        Y => PLB_wrDBus
                    );
 
-- Send the wrDAck (from the slave or the WDT) to the winning master
MSTR_WRDACK_GENERATE: for i in 0 to C_NUM_MASTERS-1 generate
    
    PLB_MWrDAck(i) <= ArbPriWrMasterReg(i) and (Sl_wrDAck);

end generate MSTR_WRDACK_GENERATE;

                    
end simulation;


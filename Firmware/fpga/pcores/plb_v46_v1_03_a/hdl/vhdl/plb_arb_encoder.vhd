-------------------------------------------------------------------------------
-- $Id: plb_arb_encoder.vhd,v 1.1.2.1 2008/04/03 15:41:24 jennj Exp $
------------------------------------------------------------------------------
-- plb_arb_encoder.vhd - entity/architecture pair
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
-- Filename:        plb_arb_encoder.vhd
-- Version:         v1.03a
-- Description:     Arb encoder selects between fixed priority and round robin
--                  arbitration.  In fixed priority, the master with highest 
--                  priority on the M_priority bits. If there are two masters 
--                  with the same priority inputs, Master 0 has the highest priority
--                  followed by Master 1, Master 2, etc. 
--                  In round robin arbitration, the highest priority master
--                  is rotated across all masters in the system.  If the
--                  highest level priority master is not requesting the bus,
--                  then the next highest level master can be selected.
--              
--
-- VHDL-Standard:   VHDL'93
-------------------------------------------------------------------------------
-- Structure:  (see plb_v46.vhd)
--
-------------------------------------------------------------------------------
-- Author:      ALS
-- History:
--      ALS     02/20/02        -- created from plb_arbiter_v1_01_a
--      ALS     02/22/02        -- corrected init string on avoid_map_error LUT
--      ALS     04/16/02        -- Version v1.01a
--      LCW     10/15/04        -- updated for NCSim
-- ^^^^^^
--  FLO         06/03/05        
-- ^^^^^^
-- Start of changes for plb_v46.
-- -PLB_pendPri split into PLB_rdPendPri and PLB_wrPendPri (Rd and Wr versions).
-- -PLB_pendReq split into PLB_rdPendReq and PLB_wrPendReq (Rd and Wr versions).
-- -Changed structure section to a reference to plb_v46.vhd.
-- ~~~~~~
--  FLO         08/18/05        
-- ^^^^^^
-- -Optimized the single-master case by replacing the instantiation of
--  priority_encoder with simple logic. Saved about 9 LUTs.
-- ~~~~~~
--  FLO         08/26/05        
-- ^^^^^^
-- -Added generic C_OPTIMIZE_1M and attendant optimizations.
-- ~~~~~~
--  FLO         09/03/05        
-- ^^^^^^
-- -arbaddrsel FF brought into this file.
-- ~~~~~~
--  FLO         12/02/05        
-- ^^^^^^
-- -Added C_FAMILY generic.
-- -Changed mux_onehot instances to mux_onehot_f.
-- ~~~~~
--  JLJ         09/14/07    v1.01a  
-- ^^^^^^
--  Update to v1.01a.  Renamed from plb_priority_encoder to plb_arb_encoder 
--  to support both arbitration schemes.
-- ~~~~~~
--  JLJ         10/02/07    v1.01a  
-- ^^^^^^
--  Update parameter on C_NUM_MASTERS on rr_select module.
-- ~~~~~~
--  JLJ         10/19/07    v1.02a  
-- ^^^^^^
--  Update to v1.02a (and merge with edits to v1.01a).
-- ~~~~~~
--  JLJ         10/31/07    v1.02a  
-- ^^^^^^
--  Clean up unused ports.
--  Remove input signal, M_abort and usage.
-- ~~~~~~
--  JLJ         12/18/07    v1.02a  
-- ^^^^^^
--  Code cleanup.
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

-- PROC_COMMON library contains mux_onehot_f component
library proc_common_v2_00_a;
use proc_common_v2_00_a.proc_common_pkg.RESET_ACTIVE;

library unisim;
use unisim.vcomponents.LUT4;

library plb_v46_v1_03_a;

-------------------------------------------------------------------------------
-- Port Declaration
-------------------------------------------------------------------------------
-------------------------------------------------------------------------------
-- Definition of Generics:
--          C_NUM_MASTERS               -- number of masters
--          C_NUM_MSTRS_PAD             -- number of masters padded to next
--                                      -- power of 2
--          C_OPTIMIZE_1M               -- true iff the one-master case is to be
--                                      -- optimized
--
--
-- Definition of Ports:
--      -- Master Signals
--          input M_abort               -- array containing all master aborts
--          input M_priority            -- array containing all master priority
--          input M_request             -- array containing all master requests
--          input M_RNW                 -- array containing all master RNW
--
--      -- Bus State Signals
--          input ArbDisMReqReg         -- masters with disabled requests
--          input ArbSecRdInProgReg     -- secondary read in progress
--          input SecRdInProgPriorReg   -- priority of secondary read
--          input ArbSecWrInProgReg     -- secondary write in progress
--          input SecWrInProgPriorReg   -- priority of secondary write
--          input LoadAddrSelReg        -- indicates when priority encoder
--                                      -- output can be registered
--      -- Outputs
--          output ArbAddrSelReg        -- one-hot register indicating which
--                                      -- master has the bus
--          output PLB_rdPendPri        -- highest priority of all pending rqsts
--          output PLB_wrPendPri        -- highest priority of all pending rqsts
--          output PLB_rdPendReq        -- indicates a read  request is pending
--          output PLB_wrPendReq        -- indicates a write request is pending
--          output PLB_reqPri           -- indicates the priority of the rqst
--
--      -- Clock and Reset
--          input Clk
--          input ArbReset
-------------------------------------------------------------------------------

-------------------------------------------------------------------------------
-- Entity Section
-------------------------------------------------------------------------------
entity plb_arb_encoder is
    generic (
             C_NUM_MASTERS  : integer   := 8;
             C_NUM_MSTRS_PAD: integer   := 8;
             C_OPTIMIZE_1M  : boolean;
             C_FAMILY       : string;
             C_ARB_TYPE     : integer   := 0    -- 0: fixed, 1: round-robin
             );
    port (
          ArbAddrSelReg     : out std_logic_vector(0 to C_NUM_MASTERS - 1 );
          ArbDisMReqReg     : in std_logic_vector(0 to C_NUM_MSTRS_PAD - 1 );
          ArbReset          : in std_logic;
          ArbSecRdInProgReg : in std_logic;
          ArbSecWrInProgReg : in std_logic;
          Clk               : in std_logic;
          LoadAddrSelReg    : in std_logic;
          M_busLock         : in std_logic_vector(0 to C_NUM_MSTRS_PAD - 1 );
          M_priority        : in std_logic_vector(0 to C_NUM_MSTRS_PAD * 2 - 1 );
          M_request         : in std_logic_vector(0 to C_NUM_MSTRS_PAD - 1 );
          M_RNW             : in std_logic_vector(0 to C_NUM_MASTERS - 1 );
          PLB_rdPendPri     : out std_logic_vector(0 to 1 );
          PLB_wrPendPri     : out std_logic_vector(0 to 1 );
          PLB_rdPendReq     : out std_logic;
          PLB_wrPendReq     : out std_logic;
          PLB_reqPri        : out std_logic_vector(0 to 1 );
          SecRdInProgPriorReg : in std_logic_vector(0 to 1 );
          SecWrInProgPriorReg : in std_logic_vector(0 to 1 )
          );

end plb_arb_encoder;

-------------------------------------------------------------------------------
-- Architecture Section
-------------------------------------------------------------------------------
architecture implementation of plb_arb_encoder is

-------------------------------------------------------------------------------
-- Signal Declarations
-------------------------------------------------------------------------------

signal arbAddrSelReg_i  : std_logic_vector(0 to C_NUM_MASTERS - 1 );
signal plb_rdPendReq_i  : std_logic;
signal plb_wrPendReq_i  : std_logic;
signal prioencdrOutput      : std_logic_vector(0 to C_NUM_MSTRS_PAD - 1 );
signal prioencdrOutput_pri  : std_logic_vector(0 to C_NUM_MSTRS_PAD - 1 );
signal prioencdrOutput_rr   : std_logic_vector(0 to C_NUM_MASTERS - 1 );

-------------------------------------------------------------------------------
-- Component Declarations
-------------------------------------------------------------------------------

-------------------------------------------------------------------------------
-- Begin architecture
-------------------------------------------------------------------------------
begin

ArbAddrSelReg <= arbAddrSelReg_i;
PLB_rdPendReq <= plb_rdPendReq_i;
PLB_wrPendReq <= plb_wrPendReq_i;

-------------------------------------------------------------------------------
-- Component instantiations
-------------------------------------------------------------------------------
-- Priority encoder determines winning master based on priority bits. If there
-- is a tie, Master 0 has priority, then Master 1, Master 2, etc. This component
-- uses the padded number of masters and padded buses.
GTR_ONE_MASTER: if not (C_OPTIMIZE_1M and (C_NUM_MASTERS = 1)) generate 

    -- If FIXED mode arbitration type
    FIXED_ARB_GEN: if (C_ARB_TYPE = 0) generate

        -- Priority encoder selects master with highest priority bits. If there are two
        -- masters with the same priority inputs, Master 0 has the highest priority
        -- followed by Master 1, Master 2, etc.

        I_PRIOR_ENC: entity plb_v46_v1_03_a.priority_encoder
        generic map (C_NUM_MASTERS  => C_NUM_MSTRS_PAD)
        port map (
            M_request         => M_request(0 to C_NUM_MSTRS_PAD-1),
            M_priority        => M_priority(0 to C_NUM_MSTRS_PAD*2-1),
            ArbDisMReqReg     => ArbDisMReqReg(0 to C_NUM_MSTRS_PAD-1),
            PrioencdrOutput   => prioencdrOutput_pri
            );
                  
       prioencdrOutput <= prioencdrOutput_pri;

    end generate FIXED_ARB_GEN;
              
    -- If Round Robin mode arbitration type
    RR_ARB_GEN: if (C_ARB_TYPE = 1) generate

        -- RR select module determines next master based on a round robin
        -- arbitration.  In RR mode, the highest select master rotates
        -- among all connected master modules.
        I_RR_SELECT: entity plb_v46_v1_03_a.rr_select
        generic map (C_NUM_MASTERS  => C_NUM_MASTERS,
                     C_FAMILY       => C_FAMILY)
        port map (
            Clk               =>    Clk,
            ArbReset          =>    ArbReset,
            LoadAddrSelReg    =>    LoadAddrSelReg,
            M_request         =>    M_request(0 to C_NUM_MASTERS-1),
            ArbDisMReqReg     =>    ArbDisMReqReg(0 to C_NUM_MASTERS-1),
            PrioencdrOutput   =>    prioencdrOutput_rr
            );

       prioencdrOutput(0 to C_NUM_MASTERS-1) <= prioencdrOutput_rr;
              
    end generate RR_ARB_GEN;
              
end generate GTR_ONE_MASTER;

-- If only one master in system
ONE_MASTER: if C_OPTIMIZE_1M and (C_NUM_MASTERS = 1) generate
    prioencdrOutput(0) <=  '1';
end generate;

ARBADDRSELREG_I_GEN : if not (C_OPTIMIZE_1M and (C_NUM_MASTERS = 1)) generate
ARBADDRSELREG_PROCESS:process (Clk)
  begin
    if (Clk'event and Clk = '1') then
        if (ArbReset)= RESET_ACTIVE then
            arbAddrSelReg_i <= (others => '0');
        elsif (LoadAddrSelReg)='1' then
            arbAddrSelReg_i <= prioencdrOutput(0 to C_NUM_MASTERS-1);
        else
            arbAddrSelReg_i <= arbAddrSelReg_i;
        end if;
    end if;
  end process ARBADDRSELREG_PROCESS;
end generate; 

ARBADDRSELREG_I_1M_GEN : if (C_OPTIMIZE_1M and (C_NUM_MASTERS = 1)) generate
    arbAddrSelReg_i(0) <= '1';
end generate;
 
-- The pending priority component determines the highest level priority bits
-- of all requesting masters and secondary transactions. It doesn't need the
-- padded version of buses or number of masters.

-- In RR mode, this function is the same.  The PLB_rdPendPri and PLB_wrPendPri
-- signals are asserted to indicate the highest priority level on all requesting
-- masters in the system.  This doesn't affect arbitration, but allows other
-- masters in the system to view priority levels.
I_PEND_PRIOR: entity plb_v46_v1_03_a.pending_priority
    generic map (C_NUM_MASTERS  => C_NUM_MASTERS)
    port map (
              M_request             => M_request(0 to C_NUM_MASTERS-1),
              M_RNW                 => M_RNW(0 to C_NUM_MASTERS-1),
              M_priority            => M_priority(0 to C_NUM_MASTERS*2 -1),
              ArbSecRdInProgReg     => ArbSecRdInProgReg,
              SecRdInProgPriorReg   => SecRdInProgPriorReg,
              ArbSecWrInProgReg     => ArbSecWrInProgReg,
              SecWrInProgPriorReg   => SecWrInProgPriorReg,
              PLB_rdPendPri         => PLB_rdPendPri,
              PLB_wrPendPri         => PLB_wrPendPri
              );

-- The pend_request component asserts PLB_pendReq if any masters are requesting
-- the bus or there is a secondary transaction in progress. It doesn't operate
-- on the padded version of buses or masters.
I_PEND_REQ: entity plb_v46_v1_03_a.pend_request
    generic map (C_NUM_MASTERS  => C_NUM_MASTERS)
    port map (
              M_request         => M_request(0 to C_NUM_MASTERS-1),
              M_RNW             => M_RNW(0 to C_NUM_MASTERS-1),
              ArbSecRdInProgReg => ArbSecRdInProgReg,
              ArbSecWrInProgReg => ArbSecWrInProgReg,
              PLB_rdPendReq     => plb_rdPendReq_i,
              PLB_wrPendReq     => plb_wrPendReq_i
              );

-- The mux_onehot_f component implements a mux with one-hot select signals
-- using the carry chain multiplexors. PLB_reqPri reflects the priority
-- of the master controlling the bus. It doesn't operate on the padded
-- version of buses or number of masters.

-- This module is the same in RR mode, the selected master M_priority signal
-- is routed out to PLB_reqPri.
I_REQ_PRIOR: entity proc_common_v2_00_a.mux_onehot_f
    generic map (C_DW       => 2,
                 C_NB       => C_NUM_MASTERS,
                 C_FAMILY   => C_FAMILY)
    port map (
              D             => M_priority(0 to C_NUM_MASTERS*2 -1),
              S             => arbAddrSelReg_i,
              Y             => PLB_reqPri
              );

end implementation;


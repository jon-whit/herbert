##############################################################################
## Filename:          C:\Git\herbert\Firmware\fpga/drivers/switch_controller_v1_00_a/data/switch_controller_v2_1_0.tcl
## Description:       Microprocess Driver Command (tcl)
## Date:              Thu Feb 05 16:40:02 2015 (by Create and Import Peripheral Wizard)
##############################################################################

#uses "xillib.tcl"

proc generate {drv_handle} {
  xdefine_include_file $drv_handle "xparameters.h" "switch_controller" "NUM_INSTANCES" "DEVICE_ID" "C_BASEADDR" "C_HIGHADDR" 
}

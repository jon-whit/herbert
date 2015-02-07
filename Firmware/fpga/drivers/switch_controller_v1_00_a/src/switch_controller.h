//////////////////////////////////////////////////////////////////////////////
// Filename:          C:\Git\herbert\Firmware\fpga/drivers/switch_controller_v1_00_a/src/switch_controller.h
// Version:           1.00.a
// Description:       switch_controller Driver Header File
// Date:              Thu Feb 05 16:40:02 2015 (by Create and Import Peripheral Wizard)
//////////////////////////////////////////////////////////////////////////////

#ifndef SWITCH_CONTROLLER_H
#define SWITCH_CONTROLLER_H

/***************************** Include Files *******************************/

#include "xbasic_types.h"
#include "xstatus.h"
#include "xio.h"

/************************** Constant Definitions ***************************/


/**
 * User Logic Slave Space Offsets
 * -- SLV_REG0 : user logic slave module register 0
 * -- SLV_REG1 : user logic slave module register 1
 * -- SLV_REG2 : user logic slave module register 2
 * -- SLV_REG3 : user logic slave module register 3
 * -- SLV_REG4 : user logic slave module register 4
 * -- SLV_REG5 : user logic slave module register 5
 * -- SLV_REG6 : user logic slave module register 6
 * -- SLV_REG7 : user logic slave module register 7
 * -- SLV_REG8 : user logic slave module register 8
 * -- SLV_REG9 : user logic slave module register 9
 * -- SLV_REG10 : user logic slave module register 10
 * -- SLV_REG11 : user logic slave module register 11
 * -- SLV_REG12 : user logic slave module register 12
 * -- SLV_REG13 : user logic slave module register 13
 * -- SLV_REG14 : user logic slave module register 14
 * -- SLV_REG15 : user logic slave module register 15
 * -- SLV_REG16 : user logic slave module register 16
 * -- SLV_REG17 : user logic slave module register 17
 * -- SLV_REG18 : user logic slave module register 18
 * -- SLV_REG19 : user logic slave module register 19
 * -- SLV_REG20 : user logic slave module register 20
 * -- SLV_REG21 : user logic slave module register 21
 * -- SLV_REG22 : user logic slave module register 22
 * -- SLV_REG23 : user logic slave module register 23
 */
#define SWITCH_CONTROLLER_USER_SLV_SPACE_OFFSET (0x00000000)
#define SWITCH_CONTROLLER_SLV_REG0_OFFSET (SWITCH_CONTROLLER_USER_SLV_SPACE_OFFSET + 0x00000000)
#define SWITCH_CONTROLLER_SLV_REG1_OFFSET (SWITCH_CONTROLLER_USER_SLV_SPACE_OFFSET + 0x00000004)
#define SWITCH_CONTROLLER_SLV_REG2_OFFSET (SWITCH_CONTROLLER_USER_SLV_SPACE_OFFSET + 0x00000008)
#define SWITCH_CONTROLLER_SLV_REG3_OFFSET (SWITCH_CONTROLLER_USER_SLV_SPACE_OFFSET + 0x0000000C)
#define SWITCH_CONTROLLER_SLV_REG4_OFFSET (SWITCH_CONTROLLER_USER_SLV_SPACE_OFFSET + 0x00000010)
#define SWITCH_CONTROLLER_SLV_REG5_OFFSET (SWITCH_CONTROLLER_USER_SLV_SPACE_OFFSET + 0x00000014)
#define SWITCH_CONTROLLER_SLV_REG6_OFFSET (SWITCH_CONTROLLER_USER_SLV_SPACE_OFFSET + 0x00000018)
#define SWITCH_CONTROLLER_SLV_REG7_OFFSET (SWITCH_CONTROLLER_USER_SLV_SPACE_OFFSET + 0x0000001C)
#define SWITCH_CONTROLLER_SLV_REG8_OFFSET (SWITCH_CONTROLLER_USER_SLV_SPACE_OFFSET + 0x00000020)
#define SWITCH_CONTROLLER_SLV_REG9_OFFSET (SWITCH_CONTROLLER_USER_SLV_SPACE_OFFSET + 0x00000024)
#define SWITCH_CONTROLLER_SLV_REG10_OFFSET (SWITCH_CONTROLLER_USER_SLV_SPACE_OFFSET + 0x00000028)
#define SWITCH_CONTROLLER_SLV_REG11_OFFSET (SWITCH_CONTROLLER_USER_SLV_SPACE_OFFSET + 0x0000002C)
#define SWITCH_CONTROLLER_SLV_REG12_OFFSET (SWITCH_CONTROLLER_USER_SLV_SPACE_OFFSET + 0x00000030)
#define SWITCH_CONTROLLER_SLV_REG13_OFFSET (SWITCH_CONTROLLER_USER_SLV_SPACE_OFFSET + 0x00000034)
#define SWITCH_CONTROLLER_SLV_REG14_OFFSET (SWITCH_CONTROLLER_USER_SLV_SPACE_OFFSET + 0x00000038)
#define SWITCH_CONTROLLER_SLV_REG15_OFFSET (SWITCH_CONTROLLER_USER_SLV_SPACE_OFFSET + 0x0000003C)
#define SWITCH_CONTROLLER_SLV_REG16_OFFSET (SWITCH_CONTROLLER_USER_SLV_SPACE_OFFSET + 0x00000040)
#define SWITCH_CONTROLLER_SLV_REG17_OFFSET (SWITCH_CONTROLLER_USER_SLV_SPACE_OFFSET + 0x00000044)
#define SWITCH_CONTROLLER_SLV_REG18_OFFSET (SWITCH_CONTROLLER_USER_SLV_SPACE_OFFSET + 0x00000048)
#define SWITCH_CONTROLLER_SLV_REG19_OFFSET (SWITCH_CONTROLLER_USER_SLV_SPACE_OFFSET + 0x0000004C)
#define SWITCH_CONTROLLER_SLV_REG20_OFFSET (SWITCH_CONTROLLER_USER_SLV_SPACE_OFFSET + 0x00000050)
#define SWITCH_CONTROLLER_SLV_REG21_OFFSET (SWITCH_CONTROLLER_USER_SLV_SPACE_OFFSET + 0x00000054)
#define SWITCH_CONTROLLER_SLV_REG22_OFFSET (SWITCH_CONTROLLER_USER_SLV_SPACE_OFFSET + 0x00000058)
#define SWITCH_CONTROLLER_SLV_REG23_OFFSET (SWITCH_CONTROLLER_USER_SLV_SPACE_OFFSET + 0x0000005C)

/**************************** Type Definitions *****************************/


/***************** Macros (Inline Functions) Definitions *******************/

/**
 *
 * Write a value to a SWITCH_CONTROLLER register. A 32 bit write is performed.
 * If the component is implemented in a smaller width, only the least
 * significant data is written.
 *
 * @param   BaseAddress is the base address of the SWITCH_CONTROLLER device.
 * @param   RegOffset is the register offset from the base to write to.
 * @param   Data is the data written to the register.
 *
 * @return  None.
 *
 * @note
 * C-style signature:
 * 	void SWITCH_CONTROLLER_mWriteReg(Xuint32 BaseAddress, unsigned RegOffset, Xuint32 Data)
 *
 */
#define SWITCH_CONTROLLER_mWriteReg(BaseAddress, RegOffset, Data) \
 	XIo_Out32((BaseAddress) + (RegOffset), (Xuint32)(Data))

/**
 *
 * Read a value from a SWITCH_CONTROLLER register. A 32 bit read is performed.
 * If the component is implemented in a smaller width, only the least
 * significant data is read from the register. The most significant data
 * will be read as 0.
 *
 * @param   BaseAddress is the base address of the SWITCH_CONTROLLER device.
 * @param   RegOffset is the register offset from the base to write to.
 *
 * @return  Data is the data from the register.
 *
 * @note
 * C-style signature:
 * 	Xuint32 SWITCH_CONTROLLER_mReadReg(Xuint32 BaseAddress, unsigned RegOffset)
 *
 */
#define SWITCH_CONTROLLER_mReadReg(BaseAddress, RegOffset) \
 	XIo_In32((BaseAddress) + (RegOffset))


/**
 *
 * Write/Read 32 bit value to/from SWITCH_CONTROLLER user logic slave registers.
 *
 * @param   BaseAddress is the base address of the SWITCH_CONTROLLER device.
 * @param   RegOffset is the offset from the slave register to write to or read from.
 * @param   Value is the data written to the register.
 *
 * @return  Data is the data from the user logic slave register.
 *
 * @note
 * C-style signature:
 * 	void SWITCH_CONTROLLER_mWriteSlaveRegn(Xuint32 BaseAddress, unsigned RegOffset, Xuint32 Value)
 * 	Xuint32 SWITCH_CONTROLLER_mReadSlaveRegn(Xuint32 BaseAddress, unsigned RegOffset)
 *
 */
#define SWITCH_CONTROLLER_mWriteSlaveReg0(BaseAddress, RegOffset, Value) \
 	XIo_Out32((BaseAddress) + (SWITCH_CONTROLLER_SLV_REG0_OFFSET) + (RegOffset), (Xuint32)(Value))
#define SWITCH_CONTROLLER_mWriteSlaveReg1(BaseAddress, RegOffset, Value) \
 	XIo_Out32((BaseAddress) + (SWITCH_CONTROLLER_SLV_REG1_OFFSET) + (RegOffset), (Xuint32)(Value))
#define SWITCH_CONTROLLER_mWriteSlaveReg2(BaseAddress, RegOffset, Value) \
 	XIo_Out32((BaseAddress) + (SWITCH_CONTROLLER_SLV_REG2_OFFSET) + (RegOffset), (Xuint32)(Value))
#define SWITCH_CONTROLLER_mWriteSlaveReg3(BaseAddress, RegOffset, Value) \
 	XIo_Out32((BaseAddress) + (SWITCH_CONTROLLER_SLV_REG3_OFFSET) + (RegOffset), (Xuint32)(Value))
#define SWITCH_CONTROLLER_mWriteSlaveReg4(BaseAddress, RegOffset, Value) \
 	XIo_Out32((BaseAddress) + (SWITCH_CONTROLLER_SLV_REG4_OFFSET) + (RegOffset), (Xuint32)(Value))
#define SWITCH_CONTROLLER_mWriteSlaveReg5(BaseAddress, RegOffset, Value) \
 	XIo_Out32((BaseAddress) + (SWITCH_CONTROLLER_SLV_REG5_OFFSET) + (RegOffset), (Xuint32)(Value))
#define SWITCH_CONTROLLER_mWriteSlaveReg6(BaseAddress, RegOffset, Value) \
 	XIo_Out32((BaseAddress) + (SWITCH_CONTROLLER_SLV_REG6_OFFSET) + (RegOffset), (Xuint32)(Value))
#define SWITCH_CONTROLLER_mWriteSlaveReg7(BaseAddress, RegOffset, Value) \
 	XIo_Out32((BaseAddress) + (SWITCH_CONTROLLER_SLV_REG7_OFFSET) + (RegOffset), (Xuint32)(Value))
#define SWITCH_CONTROLLER_mWriteSlaveReg8(BaseAddress, RegOffset, Value) \
 	XIo_Out32((BaseAddress) + (SWITCH_CONTROLLER_SLV_REG8_OFFSET) + (RegOffset), (Xuint32)(Value))
#define SWITCH_CONTROLLER_mWriteSlaveReg9(BaseAddress, RegOffset, Value) \
 	XIo_Out32((BaseAddress) + (SWITCH_CONTROLLER_SLV_REG9_OFFSET) + (RegOffset), (Xuint32)(Value))
#define SWITCH_CONTROLLER_mWriteSlaveReg10(BaseAddress, RegOffset, Value) \
 	XIo_Out32((BaseAddress) + (SWITCH_CONTROLLER_SLV_REG10_OFFSET) + (RegOffset), (Xuint32)(Value))
#define SWITCH_CONTROLLER_mWriteSlaveReg11(BaseAddress, RegOffset, Value) \
 	XIo_Out32((BaseAddress) + (SWITCH_CONTROLLER_SLV_REG11_OFFSET) + (RegOffset), (Xuint32)(Value))
#define SWITCH_CONTROLLER_mWriteSlaveReg12(BaseAddress, RegOffset, Value) \
 	XIo_Out32((BaseAddress) + (SWITCH_CONTROLLER_SLV_REG12_OFFSET) + (RegOffset), (Xuint32)(Value))
#define SWITCH_CONTROLLER_mWriteSlaveReg13(BaseAddress, RegOffset, Value) \
 	XIo_Out32((BaseAddress) + (SWITCH_CONTROLLER_SLV_REG13_OFFSET) + (RegOffset), (Xuint32)(Value))
#define SWITCH_CONTROLLER_mWriteSlaveReg14(BaseAddress, RegOffset, Value) \
 	XIo_Out32((BaseAddress) + (SWITCH_CONTROLLER_SLV_REG14_OFFSET) + (RegOffset), (Xuint32)(Value))
#define SWITCH_CONTROLLER_mWriteSlaveReg15(BaseAddress, RegOffset, Value) \
 	XIo_Out32((BaseAddress) + (SWITCH_CONTROLLER_SLV_REG15_OFFSET) + (RegOffset), (Xuint32)(Value))
#define SWITCH_CONTROLLER_mWriteSlaveReg16(BaseAddress, RegOffset, Value) \
 	XIo_Out32((BaseAddress) + (SWITCH_CONTROLLER_SLV_REG16_OFFSET) + (RegOffset), (Xuint32)(Value))
#define SWITCH_CONTROLLER_mWriteSlaveReg17(BaseAddress, RegOffset, Value) \
 	XIo_Out32((BaseAddress) + (SWITCH_CONTROLLER_SLV_REG17_OFFSET) + (RegOffset), (Xuint32)(Value))
#define SWITCH_CONTROLLER_mWriteSlaveReg18(BaseAddress, RegOffset, Value) \
 	XIo_Out32((BaseAddress) + (SWITCH_CONTROLLER_SLV_REG18_OFFSET) + (RegOffset), (Xuint32)(Value))
#define SWITCH_CONTROLLER_mWriteSlaveReg19(BaseAddress, RegOffset, Value) \
 	XIo_Out32((BaseAddress) + (SWITCH_CONTROLLER_SLV_REG19_OFFSET) + (RegOffset), (Xuint32)(Value))
#define SWITCH_CONTROLLER_mWriteSlaveReg20(BaseAddress, RegOffset, Value) \
 	XIo_Out32((BaseAddress) + (SWITCH_CONTROLLER_SLV_REG20_OFFSET) + (RegOffset), (Xuint32)(Value))
#define SWITCH_CONTROLLER_mWriteSlaveReg21(BaseAddress, RegOffset, Value) \
 	XIo_Out32((BaseAddress) + (SWITCH_CONTROLLER_SLV_REG21_OFFSET) + (RegOffset), (Xuint32)(Value))
#define SWITCH_CONTROLLER_mWriteSlaveReg22(BaseAddress, RegOffset, Value) \
 	XIo_Out32((BaseAddress) + (SWITCH_CONTROLLER_SLV_REG22_OFFSET) + (RegOffset), (Xuint32)(Value))
#define SWITCH_CONTROLLER_mWriteSlaveReg23(BaseAddress, RegOffset, Value) \
 	XIo_Out32((BaseAddress) + (SWITCH_CONTROLLER_SLV_REG23_OFFSET) + (RegOffset), (Xuint32)(Value))

#define SWITCH_CONTROLLER_mReadSlaveReg0(BaseAddress, RegOffset) \
 	XIo_In32((BaseAddress) + (SWITCH_CONTROLLER_SLV_REG0_OFFSET) + (RegOffset))
#define SWITCH_CONTROLLER_mReadSlaveReg1(BaseAddress, RegOffset) \
 	XIo_In32((BaseAddress) + (SWITCH_CONTROLLER_SLV_REG1_OFFSET) + (RegOffset))
#define SWITCH_CONTROLLER_mReadSlaveReg2(BaseAddress, RegOffset) \
 	XIo_In32((BaseAddress) + (SWITCH_CONTROLLER_SLV_REG2_OFFSET) + (RegOffset))
#define SWITCH_CONTROLLER_mReadSlaveReg3(BaseAddress, RegOffset) \
 	XIo_In32((BaseAddress) + (SWITCH_CONTROLLER_SLV_REG3_OFFSET) + (RegOffset))
#define SWITCH_CONTROLLER_mReadSlaveReg4(BaseAddress, RegOffset) \
 	XIo_In32((BaseAddress) + (SWITCH_CONTROLLER_SLV_REG4_OFFSET) + (RegOffset))
#define SWITCH_CONTROLLER_mReadSlaveReg5(BaseAddress, RegOffset) \
 	XIo_In32((BaseAddress) + (SWITCH_CONTROLLER_SLV_REG5_OFFSET) + (RegOffset))
#define SWITCH_CONTROLLER_mReadSlaveReg6(BaseAddress, RegOffset) \
 	XIo_In32((BaseAddress) + (SWITCH_CONTROLLER_SLV_REG6_OFFSET) + (RegOffset))
#define SWITCH_CONTROLLER_mReadSlaveReg7(BaseAddress, RegOffset) \
 	XIo_In32((BaseAddress) + (SWITCH_CONTROLLER_SLV_REG7_OFFSET) + (RegOffset))
#define SWITCH_CONTROLLER_mReadSlaveReg8(BaseAddress, RegOffset) \
 	XIo_In32((BaseAddress) + (SWITCH_CONTROLLER_SLV_REG8_OFFSET) + (RegOffset))
#define SWITCH_CONTROLLER_mReadSlaveReg9(BaseAddress, RegOffset) \
 	XIo_In32((BaseAddress) + (SWITCH_CONTROLLER_SLV_REG9_OFFSET) + (RegOffset))
#define SWITCH_CONTROLLER_mReadSlaveReg10(BaseAddress, RegOffset) \
 	XIo_In32((BaseAddress) + (SWITCH_CONTROLLER_SLV_REG10_OFFSET) + (RegOffset))
#define SWITCH_CONTROLLER_mReadSlaveReg11(BaseAddress, RegOffset) \
 	XIo_In32((BaseAddress) + (SWITCH_CONTROLLER_SLV_REG11_OFFSET) + (RegOffset))
#define SWITCH_CONTROLLER_mReadSlaveReg12(BaseAddress, RegOffset) \
 	XIo_In32((BaseAddress) + (SWITCH_CONTROLLER_SLV_REG12_OFFSET) + (RegOffset))
#define SWITCH_CONTROLLER_mReadSlaveReg13(BaseAddress, RegOffset) \
 	XIo_In32((BaseAddress) + (SWITCH_CONTROLLER_SLV_REG13_OFFSET) + (RegOffset))
#define SWITCH_CONTROLLER_mReadSlaveReg14(BaseAddress, RegOffset) \
 	XIo_In32((BaseAddress) + (SWITCH_CONTROLLER_SLV_REG14_OFFSET) + (RegOffset))
#define SWITCH_CONTROLLER_mReadSlaveReg15(BaseAddress, RegOffset) \
 	XIo_In32((BaseAddress) + (SWITCH_CONTROLLER_SLV_REG15_OFFSET) + (RegOffset))
#define SWITCH_CONTROLLER_mReadSlaveReg16(BaseAddress, RegOffset) \
 	XIo_In32((BaseAddress) + (SWITCH_CONTROLLER_SLV_REG16_OFFSET) + (RegOffset))
#define SWITCH_CONTROLLER_mReadSlaveReg17(BaseAddress, RegOffset) \
 	XIo_In32((BaseAddress) + (SWITCH_CONTROLLER_SLV_REG17_OFFSET) + (RegOffset))
#define SWITCH_CONTROLLER_mReadSlaveReg18(BaseAddress, RegOffset) \
 	XIo_In32((BaseAddress) + (SWITCH_CONTROLLER_SLV_REG18_OFFSET) + (RegOffset))
#define SWITCH_CONTROLLER_mReadSlaveReg19(BaseAddress, RegOffset) \
 	XIo_In32((BaseAddress) + (SWITCH_CONTROLLER_SLV_REG19_OFFSET) + (RegOffset))
#define SWITCH_CONTROLLER_mReadSlaveReg20(BaseAddress, RegOffset) \
 	XIo_In32((BaseAddress) + (SWITCH_CONTROLLER_SLV_REG20_OFFSET) + (RegOffset))
#define SWITCH_CONTROLLER_mReadSlaveReg21(BaseAddress, RegOffset) \
 	XIo_In32((BaseAddress) + (SWITCH_CONTROLLER_SLV_REG21_OFFSET) + (RegOffset))
#define SWITCH_CONTROLLER_mReadSlaveReg22(BaseAddress, RegOffset) \
 	XIo_In32((BaseAddress) + (SWITCH_CONTROLLER_SLV_REG22_OFFSET) + (RegOffset))
#define SWITCH_CONTROLLER_mReadSlaveReg23(BaseAddress, RegOffset) \
 	XIo_In32((BaseAddress) + (SWITCH_CONTROLLER_SLV_REG23_OFFSET) + (RegOffset))

/************************** Function Prototypes ****************************/


/**
 *
 * Run a self-test on the driver/device. Note this may be a destructive test if
 * resets of the device are performed.
 *
 * If the hardware system is not built correctly, this function may never
 * return to the caller.
 *
 * @param   baseaddr_p is the base address of the SWITCH_CONTROLLER instance to be worked on.
 *
 * @return
 *
 *    - XST_SUCCESS   if all self-test code passed
 *    - XST_FAILURE   if any self-test code failed
 *
 * @note    Caching must be turned off for this function to work.
 * @note    Self test may fail if data memory and device are not on the same bus.
 *
 */
XStatus SWITCH_CONTROLLER_SelfTest(void * baseaddr_p);

#endif // SWITCH_CONTROLLER_H

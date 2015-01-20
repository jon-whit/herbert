//-----------------------------------------------------------------------------
//! \file
//!
//! \brief Plate Cycler Serial Error List
//!
//! This file contains the list of Plate Cycler Serial errors.  Each error
//! has a name and a string description of the error.
//!
//! This file should not be included directly in files that use errors.
//! Rather, the file errors.h should be included.
//!
//! The DEF_ERROR macro used in this file is defined twice for two
//! different purposes.  The file errors.h defines DEF_ERROR to generate an
//! enumerated type containing the names of the errors.  The file
//! error_strings.c defines DEF_ERROR to generate a table of error strings
//! that the packet handling code can use to send error descriptions to the
//! host.
//!
//! Copyright (c) 2009 Idaho Technology Inc.
//-----------------------------------------------------------------------------

//-------- Name ------------------------------------- Description -------------------------------------- Code
DEF_ERROR( err_noError,                               "No Error"                                     ) // 000
DEF_ERROR( err_unknownCommand,                        "Unknown Command"                              ) // 001
DEF_ERROR( err_invalidParameter,                      "Invalid Parameter"                            ) // 002
DEF_ERROR( err_invalidParameterCount,                 "Invalid Parameter Count"                      ) // 003
DEF_ERROR( err_parameterOutOfRange,                   "Parameter Out Of Range"                       ) // 004
DEF_ERROR( err_connectionTimeout,                     "Connection Timeout"                           ) // 005
DEF_ERROR( err_systemBusy,                            "System Busy"                                  ) // 006
DEF_ERROR( err_operationAborted,                      "Operation Aborted"                            ) // 007
DEF_ERROR( err_invalidUpgradeImage,                   "Invalid Upgrade Image"                        ) // 008
DEF_ERROR( err_missingFileData,                       "Missing File Data"                            ) // 009
DEF_ERROR( err_invalidFileData,                       "Invalid File Data"                            ) // 010


#ifndef BOOTLOADER

//Application specific errors

DEF_ERROR( err_deviceFault,                           "Device Fault"                                 ) // 011

// Self Test & Initialization Errors
DEF_ERROR( err_selfTestFailure,                       "Self Test Failure"                            ) // 012
DEF_ERROR( err_selfTestSkipped,                       "Self Test Skipped"                            ) // 013

DEF_ERROR( err_initializationSkipped,                 "Initialization Skipped"                       ) // 014

DEF_ERROR( err_filterNotInitialized,                  "Filter Not Initialized"                       ) // 015
DEF_ERROR( err_filterInitializationFailure,           "Filter Initialization Failure"                ) // 016
DEF_ERROR( err_filterCannotMoveStepperBusy,           "Filter Cannot Move - Stepper Busy"            ) // 017
DEF_ERROR( err_filterMoveFailure,                     "Filter Move Failure"                          ) // 018

DEF_ERROR( err_lidNotInitialized,                     "Lid Not Initialized"                          ) // 019
DEF_ERROR( err_lidInitializationFailure,              "Lid Initialization Failure"                   ) // 020
DEF_ERROR( err_lidCannotMoveDoorNotInitialized,       "Lid Cannot Move - Door Not Initialized"       ) // 021
DEF_ERROR( err_lidCannotMoveDoorNotClosed,            "Lid Cannot Move - Door Not Closed"            ) // 022
DEF_ERROR( err_lidCannotMoveStepperBusy,              "Lid Cannot Move - Stepper Busy"               ) // 023
DEF_ERROR( err_lidRaiseFailure,                       "Lid Raise Failure"                            ) // 024
DEF_ERROR( err_lidLowerFailure,                       "Lid Lower Failure"                            ) // 025

DEF_ERROR( err_doorNotInitialized,                    "Door Not Initialized"                         ) // 026
DEF_ERROR( err_doorInitializationFailure,             "Door Initialization Failure"                  ) // 027
DEF_ERROR( err_doorCannotMoveLidNotInitialized,       "Door Cannot Move - Lid Not Initialized"       ) // 028
DEF_ERROR( err_doorCannotMoveLidNotRaised,            "Door Cannot Move - Lid Not Raised"            ) // 029
DEF_ERROR( err_doorCannotMoveStepperBusy,             "Door Cannot Move - Stepper Busy"              ) // 030
DEF_ERROR( err_doorOpenFailure,                       "Door Open Failure"                            ) // 031
DEF_ERROR( err_doorCloseFailure,                      "Door Close Failure"                           ) // 032

DEF_ERROR( err_ambientSensorNotCalibrated,            "Ambient Sensor Not Calibrated"                ) // 033
DEF_ERROR( err_ambientSensorAmbientTestFailure,       "Ambient Sensor Ambient Test Failure"          ) // 034
DEF_ERROR( err_ambientSensorFailure,                  "Ambient Sensor Failure"                       ) // 035
DEF_ERROR( err_ambientSensorOverTemperature,          "Ambient Sensor Over Temperature"              ) // 036
DEF_ERROR( err_ambientSensorUnderTemperature,         "Ambient Sensor Under Temperature"             ) // 037

DEF_ERROR( err_blockNotCalibrated,                    "Block Not Calibrated"                         ) // 038
DEF_ERROR( err_blockAmbientTestFailure,               "Block Ambient Test Failure"                   ) // 039
DEF_ERROR( err_blockHeatingTestFailure,               "Block Heating Test Failure"                   ) // 040
DEF_ERROR( err_blockCoolingTestFailure,               "Block Cooling Test Failure"                   ) // 041
DEF_ERROR( err_blockSensorFailure,                    "Block Sensor Failure"                         ) // 042
DEF_ERROR( err_blockOverTemperature,                  "Block Over Temperature"                       ) // 043
DEF_ERROR( err_blockUnderTemperature,                 "Block Under Temperature"                      ) // 044
DEF_ERROR( err_blockTargetTemperatureNotReached,      "Block Target Temperature Not Reached"         ) // 045
DEF_ERROR( err_blockTemperatureHoldError,             "Block Temperature Hold Error"                 ) // 046
DEF_ERROR( err_blockZoneTemperatureVarianceError,     "Block Zone Temperature Variance Error"        ) // 047
DEF_ERROR( err_blockHeatSinkSensorNotCalibrated,      "Block Heat Sink Sensor Not Calibrated"        ) // 048
DEF_ERROR( err_blockHeatSinkSensorAmbientTestFailure, "Block Heat Sink Sensor Ambient Test Failure"  ) // 049
DEF_ERROR( err_blockHeatSinkSensorFailure,            "Block Heat Sink Sensor Failure"               ) // 050
DEF_ERROR( err_blockHeatSinkOverTemperature,          "Block Heat Sink Over Temperature"             ) // 051
DEF_ERROR( err_blockHeatSinkFanLowRPM,                "Block Heat Sink Fan Low RPM"                  ) // 052
DEF_ERROR( err_blockControlFailure,                   "Block Control Failure"                        ) // 053

DEF_ERROR( err_lidThermalNotCalibrated,               "Lid Thermal Not Calibrated"                   ) // 054
DEF_ERROR( err_lidThermalAmbientTestFailure,          "Lid Thermal Ambient Test Failure"             ) // 055
DEF_ERROR( err_lidThermalHeatingTestFailure,          "Lid Thermal Heating Test Failure"             ) // 056
DEF_ERROR( err_lidSensorFailure,                      "Lid Sensor Failure"                           ) // 057
DEF_ERROR( err_lidOverTemperature,                    "Lid Over Temperature"                         ) // 058
DEF_ERROR( err_lidTargetTemperatureNotReached,        "Lid Target Temperature Not Reached"           ) // 059
DEF_ERROR( err_lidTemperatureHoldError,               "Lid Temperature Hold Error"                   ) // 060
DEF_ERROR( err_lidControlFailure,                     "Lid Control Failure"                          ) // 061

DEF_ERROR( err_ledNotCalibrated,                      "LED Not Calibrated"                           ) // 062
DEF_ERROR( err_ledPhotoSensorFailure,                 "LED Photo Sensor Failure"                     ) // 063
DEF_ERROR( err_ledControlFailure,                     "LED Control Failure"                          ) // 064
DEF_ERROR( err_ledTemperatureSensorFailure,           "LED Temperature Sensor Failure"               ) // 065
DEF_ERROR( err_ledHeatSinkTemperatureSensorFailure,   "LED Heat Sink Temperature Sensor Failure"     ) // 066
DEF_ERROR( err_ledBoardTemperatureSensorFailure,      "LED Board Temperature Sensor Failure"         ) // 067
DEF_ERROR( err_ledOverTemperature,                    "LED Over Temperature"                         ) // 068
DEF_ERROR( err_ledHeatSinkOverTemperature,            "LED Heat Sink Over Temperature"               ) // 069
DEF_ERROR( err_ledBoardOverTemperature,               "LED Board Over Temperature"                   ) // 070
DEF_ERROR( err_ledOverIntensity,                      "LED Over Intensity"                           ) // 071
DEF_ERROR( err_ledUnderIntensity,                     "LED Under Intensity"                          ) // 072
DEF_ERROR( err_ledCommFailure,                        "LED Comm Failure"                             ) // 073

DEF_ERROR( err_performanceTestComponentSkipped,       "Performance Test Component Skipped"           ) // 074
DEF_ERROR( err_performanceTestComponentMinor,         "Performance Test Component Minor Degradation" ) // 075
DEF_ERROR( err_performanceTestComponentMajor,         "Performance Test Component Major Degradation" ) // 076
DEF_ERROR( err_performanceTestComponentFailure,       "Performance Test Component Failure"           ) // 077
DEF_ERROR( err_performanceTestComponentBadBaseline,   "Performance Test Component Bad Baseline"      ) // 078
DEF_ERROR( err_performanceTestFailure,                "Performance Test Failure"                     ) // 079

DEF_ERROR( err_transitioningCalibration,              "SDA Transition in Progress"                   ) // 080

DEF_ERROR( err_exhaustFanLowRPM,                      "Exhaust Fan Low RPM"                          ) // 081

DEF_ERROR( err_modelParamNotSet,                      "Model Parameter Not Set"                      ) // 082
DEF_ERROR( err_motorNotCalibrated,                    "Motor Not Calibrated"                         ) // 083
#endif

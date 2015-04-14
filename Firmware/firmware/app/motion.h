/////////////////////////////////////////////////////////////
//
//  motion.h
//
//  High-level instrument motion control for door, lid and filters

#ifndef motion_h
#define motion_h

#include <comm.h>
#include <steppers.h>


typedef enum
{
    filter_clear = 0,
    filter_blue,
    filter_green,
    filter_orange,
    filter_red,
    filter_crimson,

    filter_moving,    
    filter_unknownPosition,
} Filters;


typedef enum
{
    door_opened,
    door_closed,
    door_opening,
    door_closing,
    door_unknownPosition,
} DoorPosition;


typedef enum
{
    lid_raised,
    lid_lowered,
    lid_raising,
    lid_lowering,
    lid_unknownPosition,
} LidPosition;

typedef enum
{
    plateType_unknown,
    plateType_none,
    plateType_sda,
    plateType_pcr,
} PlateType;


enum MotionConstants
{
    num_filters = 6
};

typedef enum
{
    rotation_clockwise,
    rotation_counterClockwise,
} RotationDirection;

typedef enum
{
    turn_quarter,
    turn_half,
} TurnSize;




void         motionInit(void);
void         motionRestoreDefaults(void);
void         motionAbort(void);
bool         motionProcess(void* unused);

bool         motionInitFilter(OfflineTaskCompleteCallback callbackFunc, int callbackRef);
bool         motionInitLid(OfflineTaskCompleteCallback callbackFunc, int callbackRef);
bool         motionInitDoor(OfflineTaskCompleteCallback callbackFunc, int callbackRef);

void         disableDoor(void);
void         setCurrentDoorClosedPosition(void);
void         setCurrentLidLoweredPosition(void);
void         setDoorClosedPosition(int position);
void         setLidLoweredPosition(int position);
bool         getDoorClosedPosition(int *position);
bool         getLidLoweredPosition(int *position);
PlateType    getPlateType(void);
const char*  getPlateTypeName(void);

Filters      getFilterPosition(void);
void         setFilter(Filters filter, OfflineTaskCompleteCallback callbackFunc, int callbackRef);


DoorPosition getDoorPosition(void);
void         openDoor(OfflineTaskCompleteCallback callbackFunc, int callbackRef);
void         closeDoor(OfflineTaskCompleteCallback callbackFunc, int callbackRef);

LidPosition  getLidPosition(void);
void         raiseLid(OfflineTaskCompleteCallback callbackFunc, int callbackRef);
void         lowerLid(OfflineTaskCompleteCallback callbackFunc, int callbackRef);

void         openCavity(OfflineTaskCompleteCallback callbackFunc, int callbackRef);
void         closeCavity(OfflineTaskCompleteCallback callbackFunc, int callbackRef);



// Debug Functions

int          getDoorStartPosition(void);
bool         getDoorHomePinFound(void);
bool         getDoorAltPinFound(void);
int          getDoorHomePinFoundPos(void);
int          getDoorAltPinFoundPos(void);

int          getLidStartPosition(void);
bool         getLidHomePinFound(void);
int          getLidAltPinCount(void);
int          getLidAltSpaceCount(void);
int          getLidHomePinMakePos(void);
int          getLidAltPinMake1Pos(void);
int          getLidAltPinBreak1Pos(void);
int          getLidAltPinMake2Pos(void);
int          getLidAltPinBreak2Pos(void);

int          getStepperParameter(StepperMotor motor, StepperParameter stepperParam);
void         setStepperParameter(StepperMotor motor, StepperParameter stepperParam, int value);
void         RotateArm(StepperMotor stepperMotor, RotationDirection direction, TurnSize turnSize, int rotations);


#endif

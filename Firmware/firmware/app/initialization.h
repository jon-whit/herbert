/////////////////////////////////////////////////////////////
//
//  initialization.h
//
//  System initialization

#ifndef initialization_h
#define initialization_h

#include <types.h>
#include <comm.h>



void initializationInit();
void initializationAbort();

void resetSystemToDefaults();


///////////////////////////////////
// System Init Function
//   completedCallbackFunc - must be specified
//   infoCallbackFunc      - may be NULL
//   haltOnError
//     - If true, initialization will halt on first error and the error message
//       will be reported via completedCallbackFunc.
//     - If false, each initialization step (error or success) will be reported
//       via infoCallbackFunc (if not NULL) and an OK or general initialization error will be
//       reported via the completed callback.  Tests will continue after a failure.


void startSystemInitialization(OfflineTaskCompleteCallback completedCallbackFunc,
                               OfflineTaskInfoCallback     infoCallbackFunc,
                               int                         callbackRef,
                               bool                        haltOnError);


#endif

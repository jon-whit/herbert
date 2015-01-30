/////////////////////////////////////////////////////////////
//
//  Messages.h
//
//  ReaderControl windows message defines
//
//  Copyright 2009 Idaho Technology
//  Created by Brett Gilbert


#ifndef Messages_h
#define Messages_h



#define WM_COMM_TRANSACTION_COMPLETE   (WM_APP)
#define WM_COMM_PENDING_RESPONSE       (WM_APP + 1)
#define WM_COMM_INFO_RESPONSE          (WM_APP + 2)
#define WM_FLY_ACQUISITION_COMPLETE    (WM_APP + 3)
#define WM_FLY_IMAGE_DATA              (WM_APP + 4)
#define WM_PROTOCOL_STATUS             (WM_APP + 5)
#define WM_INSTRUMENT_EVENT            (WM_APP + 6)
#define WM_STATUS_UPDATE               (WM_APP + 7)
#define WM_STATUS_COMPLETION           (WM_APP + 8)
#define WM_STATUS_ERROR                (WM_APP + 9)
#define WM_STATUS_STOPPED              (WM_APP + 10)
#define WM_RACHIS_CONFIG_UPDATE        (WM_APP + 11)

#endif

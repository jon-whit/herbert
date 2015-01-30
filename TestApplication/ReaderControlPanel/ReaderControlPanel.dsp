# Microsoft Developer Studio Project File - Name="ReaderControlPanel" - Package Owner=<4>
# Microsoft Developer Studio Generated Build File, Format Version 6.00
# ** DO NOT EDIT **

# TARGTYPE "Win32 (x86) Application" 0x0101

CFG=ReaderControlPanel - Win32 Debug
!MESSAGE This is not a valid makefile. To build this project using NMAKE,
!MESSAGE use the Export Makefile command and run
!MESSAGE 
!MESSAGE NMAKE /f "ReaderControlPanel.mak".
!MESSAGE 
!MESSAGE You can specify a configuration when running NMAKE
!MESSAGE by defining the macro CFG on the command line. For example:
!MESSAGE 
!MESSAGE NMAKE /f "ReaderControlPanel.mak" CFG="ReaderControlPanel - Win32 Debug"
!MESSAGE 
!MESSAGE Possible choices for configuration are:
!MESSAGE 
!MESSAGE "ReaderControlPanel - Win32 Release" (based on "Win32 (x86) Application")
!MESSAGE "ReaderControlPanel - Win32 Debug" (based on "Win32 (x86) Application")
!MESSAGE 

# Begin Project
# PROP AllowPerConfigDependencies 0
# PROP Scc_ProjName "Perforce Project"
# PROP Scc_LocalPath "."
CPP=cl.exe
MTL=midl.exe
RSC=rc.exe

!IF  "$(CFG)" == "ReaderControlPanel - Win32 Release"

# PROP BASE Use_MFC 5
# PROP BASE Use_Debug_Libraries 0
# PROP BASE Output_Dir "Release"
# PROP BASE Intermediate_Dir "Release"
# PROP BASE Target_Dir ""
# PROP Use_MFC 5
# PROP Use_Debug_Libraries 0
# PROP Output_Dir "Release"
# PROP Intermediate_Dir "Release"
# PROP Ignore_Export_Lib 0
# PROP Target_Dir ""
# ADD BASE CPP /nologo /MT /W3 /GX /O2 /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /Yu"stdafx.h" /FD /c
# ADD CPP /nologo /MT /W3 /GX /O2 /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /D "_MBCS" /Yu"stdafx.h" /FD /c
# ADD BASE MTL /nologo /D "NDEBUG" /mktyplib203 /win32
# ADD MTL /nologo /D "NDEBUG" /mktyplib203 /win32
# ADD BASE RSC /l 0x409 /d "NDEBUG"
# ADD RSC /l 0x409 /d "NDEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 /nologo /subsystem:windows /machine:I386
# ADD LINK32 /nologo /subsystem:windows /machine:I386

!ELSEIF  "$(CFG)" == "ReaderControlPanel - Win32 Debug"

# PROP BASE Use_MFC 5
# PROP BASE Use_Debug_Libraries 1
# PROP BASE Output_Dir "Debug"
# PROP BASE Intermediate_Dir "Debug"
# PROP BASE Target_Dir ""
# PROP Use_MFC 5
# PROP Use_Debug_Libraries 1
# PROP Output_Dir "Debug"
# PROP Intermediate_Dir "Debug"
# PROP Ignore_Export_Lib 0
# PROP Target_Dir ""
# ADD BASE CPP /nologo /MTd /W3 /Gm /GX /ZI /Od /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /Yu"stdafx.h" /FD /GZ /c
# ADD CPP /nologo /MTd /W3 /Gm /GR /GX /ZI /Od /I ".\\" /I ".\Comm" /I ".\Utility" /I ".\Controls" /I ".\Controls\XListCtrl" /I ".\PGR" /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /D "_MBCS" /D "XLISTCTRLLIB_STATIC" /YX"stdafx.h" /FD /GZ /c
# ADD BASE MTL /nologo /D "_DEBUG" /mktyplib203 /win32
# ADD MTL /nologo /D "_DEBUG" /mktyplib203 /win32
# ADD BASE RSC /l 0x409 /d "_DEBUG"
# ADD RSC /l 0x409 /d "_DEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 /nologo /subsystem:windows /debug /machine:I386 /pdbtype:sept
# ADD LINK32 nafxcwd.lib libcpmtd.lib pgrflycapture.lib /nologo /subsystem:windows /debug /machine:I386 /pdbtype:sept /libpath:".\PGR"

!ENDIF 

# Begin Target

# Name "ReaderControlPanel - Win32 Release"
# Name "ReaderControlPanel - Win32 Debug"
# Begin Group "Source Files"

# PROP Default_Filter "cpp;c;cxx;rc;def;r;odl;idl;hpj;bat"
# Begin Source File

SOURCE=.\CameraCalibrationDlg.cpp
# End Source File
# Begin Source File

SOURCE=.\CameraDlg.cpp
# End Source File
# Begin Source File

SOURCE=.\ColorComboBox.cpp
# End Source File
# Begin Source File

SOURCE=.\ImageConfigEditorDlg.cpp
# End Source File
# Begin Source File

SOURCE=.\ManualDlg.cpp
# End Source File
# Begin Source File

SOURCE=.\ManualProtocolDlg.cpp
# End Source File
# Begin Source File

SOURCE=.\PcrDlg.cpp
# End Source File
# Begin Source File

SOURCE=.\ProgramEditorDlg.cpp
# End Source File
# Begin Source File

SOURCE=.\Protocol.cpp
# End Source File
# Begin Source File

SOURCE=.\ReaderControl.cpp
# End Source File
# Begin Source File

SOURCE=.\ReaderControl.rc
# End Source File
# Begin Source File

SOURCE=.\ReaderControlDlg.cpp
# End Source File
# Begin Source File

SOURCE=.\ReaderControlTabDlg.cpp
# End Source File
# Begin Source File

SOURCE=.\SdaDlg.cpp
# End Source File
# Begin Source File

SOURCE=.\SegmentEditorDlg.cpp
# End Source File
# Begin Source File

SOURCE=.\StdAfx.cpp
# ADD CPP /Yc"stdafx.h"
# End Source File
# Begin Source File

SOURCE=.\TemperatureDlg.cpp
# End Source File
# End Group
# Begin Group "Header Files"

# PROP Default_Filter "h;hpp;hxx;hm;inl"
# Begin Source File

SOURCE=.\CameraCalibrationDlg.h
# End Source File
# Begin Source File

SOURCE=.\CameraDlg.h
# End Source File
# Begin Source File

SOURCE=.\Controls\CMemDC.h
# End Source File
# Begin Source File

SOURCE=.\ColorComboBox.h
# End Source File
# Begin Source File

SOURCE=.\ImageConfigEditorDlg.h
# End Source File
# Begin Source File

SOURCE=.\ManualDlg.h
# End Source File
# Begin Source File

SOURCE=.\ManualProtocolDlg.h
# End Source File
# Begin Source File

SOURCE=.\Messages.h
# End Source File
# Begin Source File

SOURCE=.\PcrDlg.h
# End Source File
# Begin Source File

SOURCE=.\ProgramEditorDlg.h
# End Source File
# Begin Source File

SOURCE=.\Protocol.h
# End Source File
# Begin Source File

SOURCE=.\ReaderControl.h
# End Source File
# Begin Source File

SOURCE=.\ReaderControlDlg.h
# End Source File
# Begin Source File

SOURCE=.\ReaderControlTabDlg.h
# End Source File
# Begin Source File

SOURCE=.\Resource.h
# End Source File
# Begin Source File

SOURCE=.\SdaDlg.h
# End Source File
# Begin Source File

SOURCE=.\SegmentEditorDlg.h
# End Source File
# Begin Source File

SOURCE=.\StdAfx.h
# End Source File
# Begin Source File

SOURCE=.\TemperatureDlg.h
# End Source File
# Begin Source File

SOURCE=.\Version.h
# End Source File
# End Group
# Begin Group "Comm"

# PROP Default_Filter ""
# Begin Source File

SOURCE=.\Comm\Comm.cpp
# End Source File
# Begin Source File

SOURCE=.\Comm\Comm.h
# End Source File
# Begin Source File

SOURCE=.\Comm\Commands.cpp
# End Source File
# Begin Source File

SOURCE=.\Comm\Commands.h
# End Source File
# Begin Source File

SOURCE=.\Comm\CommPkt.cpp
# End Source File
# Begin Source File

SOURCE=.\Comm\CommPkt.h
# End Source File
# Begin Source File

SOURCE=.\Comm\crc16.c
# End Source File
# Begin Source File

SOURCE=.\Comm\crc16.h
# End Source File
# Begin Source File

SOURCE=.\Comm\crc32.c
# End Source File
# Begin Source File

SOURCE=.\Comm\crc32.h
# End Source File
# Begin Source File

SOURCE=.\Comm\Motor.cpp
# End Source File
# Begin Source File

SOURCE=.\Comm\Motor.h
# End Source File
# Begin Source File

SOURCE=.\Comm\Serial.cpp
# End Source File
# Begin Source File

SOURCE=.\Comm\Serial.h
# End Source File
# Begin Source File

SOURCE=.\Comm\SerialEx.cpp
# End Source File
# Begin Source File

SOURCE=.\Comm\SerialEx.h
# End Source File
# End Group
# Begin Group "Controls"

# PROP Default_Filter ""
# Begin Group "XListCtrl"

# PROP Default_Filter ""
# Begin Source File

SOURCE=.\Controls\XListCtrl\XComboList.cpp
# End Source File
# Begin Source File

SOURCE=.\Controls\XListCtrl\XComboList.h
# End Source File
# Begin Source File

SOURCE=.\Controls\XListCtrl\XHeaderCtrl.cpp
# End Source File
# Begin Source File

SOURCE=.\Controls\XListCtrl\XHeaderCtrl.h
# End Source File
# Begin Source File

SOURCE=.\Controls\XListCtrl\XListCtrl.cpp
# End Source File
# Begin Source File

SOURCE=.\Controls\XListCtrl\XListCtrl.h
# End Source File
# End Group
# Begin Source File

SOURCE=.\Controls\BCMenu.cpp
# End Source File
# Begin Source File

SOURCE=.\Controls\BCMenu.h
# End Source File
# Begin Source File

SOURCE=.\Controls\BtnST.cpp
# End Source File
# Begin Source File

SOURCE=.\Controls\BtnST.h
# End Source File
# Begin Source File

SOURCE=.\Controls\DialogEx.cpp
# End Source File
# Begin Source File

SOURCE=.\Controls\DialogEx.h
# End Source File
# Begin Source File

SOURCE=.\Controls\NumberEdit.cpp
# End Source File
# Begin Source File

SOURCE=.\Controls\NumberEdit.h
# End Source File
# Begin Source File

SOURCE=.\Controls\TabCtrlEx.cpp
# End Source File
# Begin Source File

SOURCE=.\Controls\TabCtrlEx.h
# End Source File
# Begin Source File

SOURCE=.\Controls\TextProgressCtrl.cpp
# End Source File
# Begin Source File

SOURCE=.\Controls\TextProgressCtrl.h
# End Source File
# End Group
# Begin Group "Utility"

# PROP Default_Filter ""
# Begin Source File

SOURCE=.\Utility\AutoFilePointer.h
# End Source File
# Begin Source File

SOURCE=.\Utility\AutoPointer.h
# End Source File
# Begin Source File

SOURCE=.\Utility\File.cpp
# End Source File
# Begin Source File

SOURCE=.\Utility\File.h
# End Source File
# Begin Source File

SOURCE=.\Utility\Mutex.cpp
# End Source File
# Begin Source File

SOURCE=.\Utility\Mutex.h
# End Source File
# Begin Source File

SOURCE=.\Utility\Semaphore.cpp
# End Source File
# Begin Source File

SOURCE=.\Utility\Semaphore.h
# End Source File
# Begin Source File

SOURCE=.\Utility\Thread.h
# End Source File
# Begin Source File

SOURCE=.\Utility\ThreadData.h
# End Source File
# Begin Source File

SOURCE=.\Utility\ThreadQueue.h
# End Source File
# End Group
# Begin Group "Resource Files"

# PROP Default_Filter "ico;cur;bmp;dlg;rc2;rct;bin;rgs;gif;jpg;jpeg;jpe"
# Begin Source File

SOURCE=.\bitmaps\background.bmp
# End Source File
# Begin Source File

SOURCE=.\bitmaps\blue_off.bmp
# End Source File
# Begin Source File

SOURCE=.\bitmaps\blue_on.bmp
# End Source File
# Begin Source File

SOURCE=.\bitmaps\circle_slash.bmp
# End Source File
# Begin Source File

SOURCE=.\bitmaps\clear_off.bmp
# End Source File
# Begin Source File

SOURCE=.\bitmaps\clear_on.bmp
# End Source File
# Begin Source File

SOURCE=.\bitmaps\crimson_off.bmp
# End Source File
# Begin Source File

SOURCE=.\bitmaps\crimson_on.bmp
# End Source File
# Begin Source File

SOURCE=.\bitmaps\green_off.bmp
# End Source File
# Begin Source File

SOURCE=.\bitmaps\green_on.bmp
# End Source File
# Begin Source File

SOURCE=.\bitmaps\heat_block.bmp
# End Source File
# Begin Source File

SOURCE=.\bitmaps\heated_lid.bmp
# End Source File
# Begin Source File

SOURCE=.\res\iti_large.ico
# End Source File
# Begin Source File

SOURCE=.\res\iti_small.ico
# End Source File
# Begin Source File

SOURCE=.\bitmaps\orange_off.bmp
# End Source File
# Begin Source File

SOURCE=.\bitmaps\orange_on.bmp
# End Source File
# Begin Source File

SOURCE=.\res\ReaderControl.ico
# End Source File
# Begin Source File

SOURCE=.\res\ReaderControl.rc2
# End Source File
# Begin Source File

SOURCE=.\bitmaps\red_off.bmp
# End Source File
# Begin Source File

SOURCE=.\bitmaps\red_on.bmp
# End Source File
# Begin Source File

SOURCE=.\bitmaps\stop.bmp
# End Source File
# Begin Source File

SOURCE=.\bitmaps\x.bmp
# End Source File
# End Group
# Begin Group "PGR"

# PROP Default_Filter ""
# Begin Source File

SOURCE=.\PGR\FlyCapture.cpp
# End Source File
# Begin Source File

SOURCE=.\PGR\FlyCapture.h
# End Source File
# Begin Source File

SOURCE=.\PGR\PGRFlyCapture.h
# End Source File
# Begin Source File

SOURCE=.\PGR\PGRFlyCapturePlus.h
# End Source File
# End Group
# Begin Source File

SOURCE=.\ReadMe.txt
# End Source File
# End Target
# End Project

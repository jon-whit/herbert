#ifndef ReaderControlTabDlg_h
#define ReaderControlTabDlg_h

#if _MSC_VER > 1000
#pragma once
#endif //ReaderControlTabDlg _MSC_VER > 1000
// ReaderControlTabDlg.h : header file
//

#include <DialogEx.h>
#include <FpgaComm.h>
#include <TabCtrlEx.h>
#include <TextProgressCtrl.h>


/////////////////////////////////////////////////////////////////////////////
// ReaderControlTabDlg dialog

class ReaderControlTabDlg : public TabCtrlExDlg
{
// Construction
public:
    ReaderControlTabDlg(UINT nIDTemplate, CWnd* pParent);   // standard constructor

    // Pure virtual functions to assure they are implemented in
    // derived classes
    virtual BOOL SaveSettingsAndExit() = 0;
    virtual void DisplayFPGAConnected(BOOL connected) = 0;
    virtual void DisplayPCBConnected(BOOL connected) = 0;
    virtual void OnAbort() = 0;

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(ReaderControlTabDlg)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:

	// Generated message map functions
	//{{AFX_MSG(ReaderControlTabDlg)
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()

// Interface
protected:
};



class ProtocolProgressCtrl : public CTextProgressCtrl
{
public:
    ProtocolProgressCtrl() {}

    void init()
    {
        SetWindowText("");
        SetShowPercent(FALSE);
        SetRange(0, 100);
        SetPos(0);
    }

    void updateProgress(int elapsedTime_ms, int executionTime_ms)
    {
        int percentComplete = elapsedTime_ms * 100 / executionTime_ms;
        int elapsedTime_s   = (elapsedTime_ms + 500) / 1000;                     // Round to sec
        int remainingTime_s = (executionTime_ms - elapsedTime_ms + 500) / 1000;  // Round to sec

        SetPos(percentComplete);

        if(remainingTime_s > 60)
        {
            SetWindowText("Remaining time: %d min", remainingTime_s / 60);
        }
        else
        {
            SetWindowText("Remaining time: %d sec", remainingTime_s);
        }
    }

    void startUserPrompt()
    {
        SetMarquee(TRUE, 50);
        SetWindowText("Waiting for user input...");
    }

    void endUserPrompt()
    {
        SetMarquee(FALSE, 50);
        SetWindowText("");
        SetPos(0);
    }

    void completeProtocol()
    {
        SetPos(100);
        SetWindowText("Complete");
    }

    void clear()
    {
        SetPos(0);
        SetWindowText("");
    }
};




#endif

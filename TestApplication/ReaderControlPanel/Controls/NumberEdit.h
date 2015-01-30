#if !defined(AFX_NUMBEREDIT_H__FE81317E_7C08_4478_A88E_1B4F46007F6F__INCLUDED_)
#define AFX_NUMBEREDIT_H__FE81317E_7C08_4478_A88E_1B4F46007F6F__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// NumberEdit.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// NumberEdit window

class NumberEdit : public CEdit
{
public:
    enum Base
    {
        BASE_BIN,
        BASE_DEC,
        BASE_HEX,
        BASE_FLOAT
    };
// Construction
public:
    NumberEdit();

// Attributes
public:

// Operations
public:

// Overrides
    // ClassWizard generated virtual function overrides
    //{{AFX_VIRTUAL(NumberEdit)
	public:
    virtual BOOL PreTranslateMessage(MSG* pMsg);
	//}}AFX_VIRTUAL

// Implementation
public:
    virtual ~NumberEdit();

    // Generated message map functions
protected:
    //{{AFX_MSG(NumberEdit)
	afx_msg void OnUpdate();
	//}}AFX_MSG

    DECLARE_MESSAGE_MAP()

public:
    inline void   setBase(Base base)                  {m_base = base;}
    inline Base   getBase()                           {return m_base;}

    inline void   setMaxIntValue(int maxValue)        {m_maxIntValue = maxValue;}
    inline int    getMaxIntValue()                    {return m_maxIntValue;}
    inline void   setMinIntValue(int minValue)        {m_minIntValue = minValue;}
    inline int    getMinIntValue()                    {return m_minIntValue;}

    inline void   setMaxFloatValue(double maxValue)   {m_maxFloatValue = maxValue;}
    inline double getMaxFloatValue()                  {return m_maxFloatValue;}
    inline void   setMinFloatValue(double minValue)   {m_minFloatValue = minValue;}
    inline double getMinFloatValue()                  {return m_minFloatValue;}

    inline void   setMaxDigits(int maxDigits)         {m_maxDigits = maxDigits;}
    inline int    getMaxDigits()                      {return m_maxDigits;}
    inline void   setDecimalPlaces(int decimalPlaces) {m_decimalPlaces = decimalPlaces;}
    inline int    getDecimalPlaces()                  {return m_decimalPlaces;}

    void   setIntNumber(int number);
    int    getIntNumber();
    void   setFloatNumber(double number);
    double getFloatNumber();

private:
    BOOL   ValidateChar(UINT& ch);
    BOOL   ValidateString(CString& str);
    int    ConvertToIntNumber(CString& str);
    double ConvertToFloatNumber(CString& str);

private:
    Base   m_base;
    int    m_maxIntValue;
    int    m_minIntValue;
    double m_maxFloatValue;
    double m_minFloatValue;
    int    m_maxDigits;
    int    m_decimalPlaces;
    BOOL   m_updating;
};

/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_NUMBEREDIT_H__FE81317E_7C08_4478_A88E_1B4F46007F6F__INCLUDED_)

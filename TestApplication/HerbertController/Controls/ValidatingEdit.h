//-----------------------------------------------------------------------------
// Validating Edit control
//
// Created by Brett Gilbert
// Copyright 2012

#pragma once
#include <string>


//-----------------------------------------------------------------------------
// ValidatingEdit

class ValidatingEdit : public CEdit
{
    DECLARE_DYNAMIC(ValidatingEdit)

public:
    ValidatingEdit();
    virtual ~ValidatingEdit();

    virtual CString GetText() const;
    virtual void    SetText(CString text);
    virtual BOOL    IsValueInRange() { return TRUE; }

    void            SetBackgroundColor(COLORREF color);
    void            SetOutOfRangeBackgroundColor(COLORREF color);
    void            SetInvalidInputBackgroundColor(COLORREF color);
    COLORREF        GetBackgroundColor() const;
    COLORREF        GetOutOfRangeBackgroundColor() const;
    COLORREF        GetInvalidInputBackgroundColor() const;

    void            Invalidate();
protected:
    COLORREF        GetBackgroundColorFromBrush(CBrush* brush) const;

    virtual CString GetValidText() const;
    virtual void    HandleControlChar(UINT c, UINT repeatCount, UINT flags);
    virtual BOOL    HandlePrintableChar(UINT c, UINT repeatCount, UINT flags);
    virtual void    NegateText();
    virtual void    UpdateValue() {}
    virtual BOOL    OnChildNotify(UINT message, WPARAM wParam, LPARAM lParam, LRESULT* lResult);

    afx_msg void    OnTimer(UINT_PTR timerEvent);
    afx_msg void    OnChar(UINT c, UINT repeatCount, UINT flags);
    afx_msg void    OnKeyDown(UINT c, UINT repeatCount, UINT flags);
    afx_msg void    OnKillFocus(CWnd* pNewWnd);
    afx_msg LRESULT OnCut(WPARAM wParam, LPARAM lParam);
    afx_msg LRESULT OnPaste(WPARAM wParam, LPARAM lParam);
    afx_msg LRESULT OnClear(WPARAM wParam, LPARAM lParam);

    void            StartInvalidInputFlash();
    CBrush&         GetCurrentBackgroundBrush();
    static CString  FilterString(const CString& source, const CString& allowedChars);
    static BOOL     IsStringNegative(const CString& string);

    DECLARE_MESSAGE_MAP()

    COLORREF m_textColor;
    CBrush	 m_backgroundBrush;
    CBrush	 m_outOfRangeBackgroundBrush;
    CBrush	 m_invalidInputBackgroundBrush;

    BOOL     m_isColorSet;
    BOOL     m_isInvalidInputFlashOn;
};


//-----------------------------------------------------------------------------
// IntEdit

class IntEdit : public ValidatingEdit
{
    DECLARE_DYNAMIC(IntEdit)

public:
    IntEdit();
    virtual ~IntEdit();

    int          GetValue();
    void         SetValue(int value);
    virtual BOOL IsValueInRange();

    void         SetMinValue(int minValue)        {m_minValue = minValue;}
    void         SetMaxValue(int maxValue)        {m_maxValue = maxValue;}
    void         SetMaxDigits(unsigned maxDigits) {m_maxDigits = maxDigits;}
    int          GetMinValue()                    {return m_minValue;}
    int          GetMaxValue()                    {return m_maxValue;}
    unsigned     GetMaxDigits()                   {return m_maxDigits;}


protected:
    DECLARE_MESSAGE_MAP()

    virtual CString GetValidText() const;
    virtual BOOL    HandlePrintableChar(UINT c, UINT repeatCount, UINT flags);
    virtual void    UpdateValue();

    int      m_value;
    int      m_minValue;
    int      m_maxValue;
    unsigned m_maxDigits;
};


//-----------------------------------------------------------------------------
// UnsignedEdit

class UnsignedEdit : public ValidatingEdit
{
    DECLARE_DYNAMIC(UnsignedEdit)

public:
    UnsignedEdit();
    virtual ~UnsignedEdit();

    unsigned     GetValue();
    void         SetValue(unsigned value);
    virtual BOOL IsValueInRange();

    void         SetMinValue(unsigned minValue)   {m_minValue = minValue;}
    void         SetMaxValue(unsigned maxValue)   {m_maxValue = maxValue;}
    void         SetMaxDigits(unsigned maxDigits) {m_maxDigits = maxDigits;}
    unsigned     GetMinValue()                    {return m_minValue;}
    unsigned     GetMaxValue()                    {return m_maxValue;}
    unsigned     GetMaxDigits()                   {return m_maxDigits;}

protected:
    DECLARE_MESSAGE_MAP()

    virtual CString GetValidText() const;
    virtual BOOL    HandlePrintableChar(UINT c, UINT repeatCount, UINT flags);
    virtual void    UpdateValue();

    unsigned m_value;
    unsigned m_minValue;
    unsigned m_maxValue;
    unsigned m_maxDigits;
};


//-----------------------------------------------------------------------------
// DoubleEdit

class DoubleEdit : public ValidatingEdit
{
    DECLARE_DYNAMIC(DoubleEdit)

public:
    DoubleEdit();
    virtual ~DoubleEdit();

    double       GetValue();
    void         SetValue(double value, BOOL trimTrailingZeros = TRUE);
    virtual BOOL IsValueInRange();

    void         SetMinValue(double minValue)                   {m_minValue = minValue;}
    void         SetMaxValue(double maxValue)                   {m_maxValue = maxValue;}
    void         SetMaxWholeDigits(unsigned maxWholeDigits)     {m_maxWholeDigits = maxWholeDigits;}
    void         SetMaxDecimalPlaces(unsigned maxDecimalPlaces) {m_maxDecimalPlaces = maxDecimalPlaces;}
    double       GetMinValue(double minValue)                   {return m_minValue;}
    double       GetMaxValue(double maxValue)                   {return m_maxValue;}
    unsigned     GetMaxWholeDigits()                            {return m_maxWholeDigits;}
    unsigned     GetMaxDecimalPlaces()                          {return m_maxDecimalPlaces;}

protected:
    DECLARE_MESSAGE_MAP()

    virtual CString GetValidText() const;
    virtual BOOL    HandlePrintableChar(UINT c, UINT repeatCount, UINT flags);
    virtual void    UpdateValue();

    double   m_value;
    double   m_minValue;
    double   m_maxValue;
    unsigned m_maxWholeDigits;
    unsigned m_maxDecimalPlaces;
};


//-----------------------------------------------------------------------------
// LetterEdit

class LetterEdit : public ValidatingEdit
{
    DECLARE_DYNAMIC(LetterEdit)

public:
    LetterEdit();
    virtual ~LetterEdit();

    std::string  GetValue();
    void         SetValue(std::string value);
    virtual BOOL IsValueInRange();

    void         SetMinCharacterCount(int minCharacterCount) {m_minCharacterCount = minCharacterCount;}
    void         SetMaxCharacterCount(int maxCharacterCount) {m_maxCharacterCount = maxCharacterCount;}
    int          GetMinCharacterCount()                      {return m_minCharacterCount;}
    int          GetMaxCharacterCount()                      {return m_maxCharacterCount;}

protected:
    DECLARE_MESSAGE_MAP()

    virtual CString GetValidText() const;
    virtual BOOL    HandlePrintableChar(UINT c, UINT repeatCount, UINT flags);
    virtual void    UpdateValue();

    std::string m_value;
    unsigned    m_minCharacterCount;
    unsigned    m_maxCharacterCount;
};

//-----------------------------------------------------------------------------
// StringEdit

class StringEdit : public ValidatingEdit
{
    DECLARE_DYNAMIC(StringEdit)

public:
    StringEdit();
    virtual ~StringEdit();

    std::string  GetValue();
    void         SetValue(std::string value);
    virtual BOOL IsValueInRange();

    void         SetMinCharacterCount(int minCharacterCount) {m_minCharacterCount = minCharacterCount;}
    void         SetMaxCharacterCount(int maxCharacterCount) {m_maxCharacterCount = maxCharacterCount;}
    int          GetMinCharacterCount()                      {return m_minCharacterCount;}
    int          GetMaxCharacterCount()                      {return m_maxCharacterCount;}

protected:
    DECLARE_MESSAGE_MAP()

    virtual CString GetValidText() const;
    virtual BOOL    HandlePrintableChar(UINT c, UINT repeatCount, UINT flags);
    virtual void    UpdateValue();

    BOOL            IsCharacter(UINT c);

    std::string m_value;
    unsigned    m_minCharacterCount;
    unsigned    m_maxCharacterCount;
};


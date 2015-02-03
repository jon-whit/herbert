// ValidatingEdit.cpp

#include "stdafx.h"
#include <ValidatingEdit.h>

#include <ctype.h>

//-----------------------------------------------------------------------------
// ValidatingEdit

IMPLEMENT_DYNAMIC(ValidatingEdit, CEdit)

ValidatingEdit::ValidatingEdit() :
    m_isColorSet(FALSE),
    m_isInvalidInputFlashOn(FALSE),
    m_textColor(RGB(0,0,0))
{
    SetOutOfRangeBackgroundColor(RGB(255,100,100));
    SetInvalidInputBackgroundColor(RGB(255,255,100));
}

ValidatingEdit::~ValidatingEdit()
{
}


BEGIN_MESSAGE_MAP(ValidatingEdit, CEdit)
    ON_WM_CHAR()
    ON_WM_KEYDOWN()
    ON_WM_TIMER()
    ON_MESSAGE(WM_CUT, OnCut)
    ON_MESSAGE(WM_PASTE, OnPaste)
    ON_MESSAGE(WM_CLEAR, OnClear)
//    ON_MESSAGE(WM_SETTEXT, OnSetText)
END_MESSAGE_MAP()


void ValidatingEdit::Invalidate()
{
    if(::IsWindow(m_hWnd))
    {
        CEdit::Invalidate();
    }
}

void ValidatingEdit::SetBackgroundColor(COLORREF color)
{
    m_isColorSet = TRUE;
    m_backgroundBrush.DeleteObject();
    m_backgroundBrush.CreateSolidBrush(color);
    Invalidate();
}


COLORREF ValidatingEdit::GetBackgroundColor() const
{
    return GetBackgroundColorFromBrush(&const_cast<ValidatingEdit*>(this)->m_backgroundBrush);
}


void ValidatingEdit::SetOutOfRangeBackgroundColor(COLORREF color)
{
    m_isColorSet = TRUE;
    m_outOfRangeBackgroundBrush.DeleteObject();
    m_outOfRangeBackgroundBrush.CreateSolidBrush(color);
    Invalidate();
}


COLORREF ValidatingEdit::GetOutOfRangeBackgroundColor() const
{
    return GetBackgroundColorFromBrush(&const_cast<ValidatingEdit*>(this)->m_outOfRangeBackgroundBrush);
}


void ValidatingEdit::SetInvalidInputBackgroundColor(COLORREF color)
{
    m_isColorSet = TRUE;
    m_invalidInputBackgroundBrush.DeleteObject();
    m_invalidInputBackgroundBrush.CreateSolidBrush(color);
    Invalidate();
}



COLORREF ValidatingEdit::GetInvalidInputBackgroundColor() const
{
    return GetBackgroundColorFromBrush(&const_cast<ValidatingEdit*>(this)->m_invalidInputBackgroundBrush);
}



COLORREF ValidatingEdit::GetBackgroundColorFromBrush(CBrush* brush) const
{
    if (!brush->GetSafeHandle())
    {
        COLORREF color = const_cast<ValidatingEdit*>(this)->GetDC()->GetBkColor();
        brush->CreateSolidBrush(color);
        return color;
    }

    LOGBRUSH lb;
    brush->GetLogBrush(&lb);
    return lb.lbColor;
}


CString ValidatingEdit::GetText() const
{
    CString strText;
    GetWindowText(strText);
    return strText;
}


void ValidatingEdit::SetText(CString text)
{
    SetWindowText(text);
    text = GetValidText();
    SetWindowText(text);
}


void ValidatingEdit::HandleControlChar(UINT c, UINT repeatCount, UINT flags)
{
    CEdit::OnChar(c, repeatCount, flags);
}


BOOL ValidatingEdit::HandlePrintableChar(UINT c, UINT repeatCount, UINT flags)
{
    // See: http://support.microsoft.com/kb/92394
    // DefWindowProc(WM_CHAR, nChar, MAKELONG(nRepCnt, nFlags));
    // instead of CEdit::OnChar(uChar, nRepCnt, nFlags)
    // if changing uChar
    CEdit::OnChar(c, repeatCount, flags);
    return TRUE;
}


void ValidatingEdit::NegateText()
{
    CString text = GetText();

    if(text.GetLength() > 0 && text[0] == '-')
    {
        text.Delete(0);
    }
    else
    {
        text.Insert(0, '-');
    }

    SetText(text);
    SetSel(text.GetLength(), text.GetLength());
}


BOOL ValidatingEdit::OnChildNotify(UINT message, WPARAM wParam, LPARAM lParam, LRESULT* lResult) 
{
    if (m_isColorSet && (message == WM_CTLCOLOREDIT || message == WM_CTLCOLORSTATIC))
    {
        CBrush& brush = GetCurrentBackgroundBrush();
        CDC*    dc    = CDC::FromHandle((HDC)wParam);

        if (m_textColor != RGB(0,0,0))
            dc->SetTextColor(m_textColor);

        // Set the text background to the requested background color
        dc->SetBkColor(GetBackgroundColorFromBrush(&brush));

        *lResult = (LRESULT)brush.GetSafeHandle();
        return TRUE;
    }

    return CEdit::OnChildNotify(message, wParam, lParam, lResult);
}


void ValidatingEdit::OnChar(UINT c, UINT repeatCount, UINT flags)
{
    if(!isprint(c))
    {
        HandleControlChar(c, repeatCount, flags);
    }
    else if(!HandlePrintableChar(c, repeatCount, flags))
    {
        StartInvalidInputFlash();
    }

    Invalidate();
    UpdateValue();
}


void ValidatingEdit::OnKeyDown(UINT c, UINT repeatCount, UINT flags) 
{
    CEdit::OnKeyDown(c, repeatCount, flags);
}


CString ValidatingEdit::GetValidText() const
{
    return GetText();
}


LRESULT ValidatingEdit::OnCut(WPARAM, LPARAM)
{
    int start, end;
    GetSel(start, end);

    if (start < end)
    {
        SendMessage(WM_COPY);
        ReplaceSel("");
    }

    UpdateValue();

    return 0;
}


LRESULT ValidatingEdit::OnPaste(WPARAM, LPARAM)
{
    int start, end;
    GetSel(start, end);

    CEdit::Default();
    CString strText = GetValidText();

    if (strText != GetText())
    {
        SetWindowText(strText);
        SetSel(start, end);
    }

    UpdateValue();

    return 0;
}


LRESULT ValidatingEdit::OnClear(WPARAM, LPARAM)
{
    int start, end;
    GetSel(start, end);

    if (start < end)
    {
        SendMessage(WM_KEYDOWN, VK_DELETE); // delete the selection
    }

    UpdateValue();

    return 0;
}

void ValidatingEdit::OnTimer(UINT_PTR timerEvent)
{
    if(timerEvent == 1)
    {
        KillTimer(timerEvent);
        m_isInvalidInputFlashOn = FALSE;
        Invalidate();
    }
    else
    {
        CEdit::OnTimer(timerEvent);
    }
}


void ValidatingEdit::StartInvalidInputFlash()
{
    SetTimer(1, 50, 0);
    m_isInvalidInputFlashOn = TRUE;
    Invalidate();
}


CBrush& ValidatingEdit::GetCurrentBackgroundBrush()
{
    if(m_isInvalidInputFlashOn && m_invalidInputBackgroundBrush.GetSafeHandle())
    {
        return m_invalidInputBackgroundBrush;
    }
    else if(!IsValueInRange() && m_outOfRangeBackgroundBrush.GetSafeHandle())
    {
        return m_outOfRangeBackgroundBrush;
    }
    else
    {
        return m_backgroundBrush;
    }
}


CString ValidatingEdit::FilterString(const CString& source, const CString& allowedChars)
{
    CString filteredString;

    for(int i = 0; i < source.GetLength(); ++i)
    {
        char c = source[i];

        if(allowedChars.Find(c) != -1)
        {
            filteredString += c;
        }
    }

    return filteredString;
}


BOOL ValidatingEdit::IsStringNegative(const CString& string)
{
    return string.GetLength() > 0 && string[0] == '-';
}

//-----------------------------------------------------------------------------
// IntEdit

IntEdit::IntEdit() :
    m_maxDigits(10),
    m_value(0),
    m_minValue(0),
    m_maxValue(0)
{
}

IntEdit::~IntEdit()
{
}

IMPLEMENT_DYNAMIC(IntEdit, ValidatingEdit)

BEGIN_MESSAGE_MAP(IntEdit, ValidatingEdit)
END_MESSAGE_MAP()

int IntEdit::GetValue()
{
    return m_value;
}

void IntEdit::SetValue(int value)
{
    m_value = value;
    CString text;
    text.Format("%d", m_value);
    SetText(text);
}


CString IntEdit::GetValidText() const
{
    CString text       = GetText();
    BOOL    isNegative = IsStringNegative(text);

    text = FilterString(text.Right(text.GetLength() - (isNegative ? 1 : 0)), "0123456789").Left(m_maxDigits);

    if(isNegative)
    {
        text.Insert(0, '-');
    }

    return text;
}


BOOL IntEdit::HandlePrintableChar(UINT c, UINT repeatCount, UINT flags)
{
    if(isdigit(c))
    {
        int selStart;
        int selEnd;

        GetSel(selStart, selEnd);

        CString text = GetText();

        if((unsigned)text.GetLength() < m_maxDigits ||
           (text[0] == '-' && (unsigned)text.GetLength() - 1 < m_maxDigits) ||
           (selStart < selEnd))
        {
            return ValidatingEdit::HandlePrintableChar(c, repeatCount, flags);
        }

        return FALSE;
    }

    if(c == '-')
    {
        NegateText();
        return TRUE;
    }

    return FALSE;
}


void IntEdit::UpdateValue()
{
    CString text = GetValidText();
    m_value = atoi(text);
}


BOOL IntEdit::IsValueInRange()
{
    UpdateValue();

    if(m_minValue != m_maxValue)
    {
        return m_value >= m_minValue && m_value <= m_maxValue;
    }

    return TRUE;
}

//-----------------------------------------------------------------------------
// UnsignedEdit

UnsignedEdit::UnsignedEdit() :
    m_maxDigits(10),
    m_value(0),
    m_minValue(0),
    m_maxValue(0)
{
}

UnsignedEdit::~UnsignedEdit()
{
}

IMPLEMENT_DYNAMIC(UnsignedEdit, ValidatingEdit)

BEGIN_MESSAGE_MAP(UnsignedEdit, ValidatingEdit)
END_MESSAGE_MAP()

unsigned UnsignedEdit::GetValue()
{
    return m_value;
}

void UnsignedEdit::SetValue(unsigned value)
{
    m_value = value;
    CString text;
    text.Format("%d", m_value);
    SetText(text);
}


CString UnsignedEdit::GetValidText() const
{
    return FilterString(GetText(), "0123456789").Left(m_maxDigits);
}


BOOL UnsignedEdit::HandlePrintableChar(UINT c, UINT repeatCount, UINT flags)
{
    if(isdigit(c))
    {
        int selStart;
        int selEnd;

        GetSel(selStart, selEnd);

        CString text = GetText();

        if((unsigned)text.GetLength() < m_maxDigits || (selStart < selEnd))
        {
            return ValidatingEdit::HandlePrintableChar(c, repeatCount, flags);
        }

        return FALSE;
    }

    return FALSE;
}


void UnsignedEdit::UpdateValue()
{
    CString text = GetValidText();
    m_value = atoi(text);
}


BOOL UnsignedEdit::IsValueInRange()
{
    UpdateValue();

    if(m_minValue != m_maxValue)
    {
        return m_value >= m_minValue && m_value <= m_maxValue;
    }

    return TRUE;
}





//-----------------------------------------------------------------------------
// DoubleEdit

DoubleEdit::DoubleEdit() :
    m_value(0),
    m_minValue(0),
    m_maxValue(0),
    m_maxWholeDigits(3),
    m_maxDecimalPlaces(3)
{
}

DoubleEdit::~DoubleEdit()
{
}

IMPLEMENT_DYNAMIC(DoubleEdit, ValidatingEdit)

BEGIN_MESSAGE_MAP(DoubleEdit, ValidatingEdit)
END_MESSAGE_MAP()

double DoubleEdit::GetValue()
{
    return m_value;
}

void DoubleEdit::SetValue(double value, BOOL trimTrailingZeros)
{
    m_value = value;

    CString format;
    format.Format("%%0.%df", m_maxDecimalPlaces);
    CString text;
    text.Format(format, m_value);

    if(trimTrailingZeros)
    {
        text.TrimRight('0');
        text.TrimRight('.');
    }

    SetText(text);
}


CString DoubleEdit::GetValidText() const
{
    const CString allowedDigitChars = "0123456789";

    CString text       = GetText();
    BOOL    isNegative = IsStringNegative(text);

    if(isNegative)
    {
        text.Delete(0);
    }

    int decimalPos = text.Find('.');

    if(decimalPos != -1)
    {
        CString wholeDigits = FilterString(text.Left(decimalPos), allowedDigitChars).Left(m_maxWholeDigits);
        CString decimalDigits = FilterString(text.Right(text.GetLength() - decimalPos), allowedDigitChars).Left(m_maxDecimalPlaces);
        text = wholeDigits + '.' + decimalDigits;
    }
    else
    {
        text = FilterString(text, allowedDigitChars).Left(m_maxWholeDigits);
    }

    if(isNegative)
    {
        text.Insert(0, '-');
    }

    return text;
}


BOOL DoubleEdit::HandlePrintableChar(UINT c, UINT repeatCount, UINT flags)
{
    if(isdigit(c))
    {
        int selStart;
        int selEnd;

        GetSel(selStart, selEnd);

        CString text = GetText();

        int negativePos = (text[0] == '-') ? 1 : 0;
        int decimalPos  = text.Find('.');
        decimalPos = (decimalPos == -1) ? text.GetLength() : decimalPos;

        int wholeDigits   = decimalPos - negativePos;
        int decimalPlaces = text.GetLength() - decimalPos;

        if(selStart <= decimalPos && selEnd <= decimalPos &&
           decimalPos - negativePos - (selEnd - selStart) + repeatCount <= m_maxWholeDigits)
        {
            return ValidatingEdit::HandlePrintableChar(c, repeatCount, flags);
        }

        if(selStart <= decimalPos && selEnd > decimalPos &&
           text.GetLength() - negativePos - (selEnd - selStart) + repeatCount <= m_maxWholeDigits)
        {
            return ValidatingEdit::HandlePrintableChar(c, repeatCount, flags);
        }

        if(selStart > decimalPos && selEnd > decimalPos &&
           text.GetLength() - decimalPos <= (int)m_maxDecimalPlaces)
        {
            return ValidatingEdit::HandlePrintableChar(c, repeatCount, flags);
        }

        return FALSE;
    }

    if(c == '.')
    {
        CString text = GetText();
        if(text.Find('.') == -1)
        {
            int textLen = text.GetLength();

            if(textLen == 0 || (textLen == 1 && text[0] == '-'))
            {
                DefWindowProc(WM_CHAR, '0', MAKELONG(1, flags));
            }

            return ValidatingEdit::HandlePrintableChar(c, repeatCount, flags);
        }
        return FALSE;
    }

    if(c == '-')
    {
        NegateText();
        return TRUE;
    }

    return FALSE;
}


void DoubleEdit::UpdateValue()
{
    CString text = GetValidText();
    m_value = atof(text);
}


BOOL DoubleEdit::IsValueInRange()
{
    UpdateValue();

    if(m_minValue != m_maxValue)
    {
        return m_value >= m_minValue && m_value <= m_maxValue;
    }

    return TRUE;
}

//-----------------------------------------------------------------------------
// LetterEdit

LetterEdit::LetterEdit() :
      m_value("")
    , m_minCharacterCount(0)
    , m_maxCharacterCount(0)
{
}

LetterEdit::~LetterEdit()
{
}

IMPLEMENT_DYNAMIC(LetterEdit, ValidatingEdit)

BEGIN_MESSAGE_MAP(LetterEdit, ValidatingEdit)
END_MESSAGE_MAP()

std::string LetterEdit::GetValue()
{
    return m_value;
}

void LetterEdit::SetValue(std::string value)
{
    m_value = value;

    SetText(m_value.c_str());
}


CString LetterEdit::GetValidText() const
{
    const CString allowedDigitChars = "abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ";

    CString text = GetText();

    text = FilterString(text, allowedDigitChars).Left(m_maxCharacterCount);

    return text;
}


BOOL LetterEdit::HandlePrintableChar(UINT c, UINT repeatCount, UINT flags)
{    
    if(isalpha((int)c))
    {
        int selStart;
        int selEnd;

        GetSel(selStart, selEnd);

        CString text = GetText();

        if((unsigned)text.GetLength() < m_maxCharacterCount || (selStart < selEnd))
        {
            return ValidatingEdit::HandlePrintableChar(c, repeatCount, flags);
        }

        return FALSE;
    }

    return FALSE;
}

void LetterEdit::UpdateValue()
{
    CString text = GetValidText();
    m_value = ((LPCTSTR)text);
}


BOOL LetterEdit::IsValueInRange()
{
    UpdateValue();

    if(m_minCharacterCount != m_maxCharacterCount)
    {
        return m_value.length() >= m_minCharacterCount && m_value.length() <= m_maxCharacterCount;
    }

    return TRUE;
}

//-----------------------------------------------------------------------------
// StringEdit

StringEdit::StringEdit() :
      m_value("")
    , m_minCharacterCount(0)
    , m_maxCharacterCount(0)
{
}

StringEdit::~StringEdit()
{
}

IMPLEMENT_DYNAMIC(StringEdit, ValidatingEdit)

BEGIN_MESSAGE_MAP(StringEdit, ValidatingEdit)
END_MESSAGE_MAP()

std::string StringEdit::GetValue()
{
    return m_value;
}

void StringEdit::SetValue(std::string value)
{
    m_value = value;

    SetText(m_value.c_str());
}


CString StringEdit::GetValidText() const // Not filtered
{
    CString text = GetText();

    return text.Left(m_maxCharacterCount);
}


BOOL StringEdit::HandlePrintableChar(UINT c, UINT repeatCount, UINT flags)
{    
    int selStart;
    int selEnd;

    GetSel(selStart, selEnd);

    CString text = GetText();

    if((unsigned)text.GetLength() < m_maxCharacterCount || (selStart < selEnd))
    {
        return ValidatingEdit::HandlePrintableChar(c, repeatCount, flags);
    }

    return FALSE;

}

void StringEdit::UpdateValue()
{
    CString text = GetValidText();
    m_value = ((LPCTSTR)text);
}


BOOL StringEdit::IsValueInRange()
{
    UpdateValue();

    if(m_minCharacterCount != m_maxCharacterCount)
    {
        return m_value.length() >= m_minCharacterCount && m_value.length() <= m_maxCharacterCount;
    }

    return TRUE;
}

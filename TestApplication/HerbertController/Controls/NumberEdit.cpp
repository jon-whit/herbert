// NumberEdit.cpp : implementation file
//

#include "stdafx.h"
#include "NumberEdit.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// NumberEdit

NumberEdit::NumberEdit() :
    m_updating(FALSE),
    m_base(BASE_DEC),
    m_maxIntValue(INT_MAX),
    m_minIntValue(INT_MIN),
    m_maxFloatValue(100e100),
    m_minFloatValue(-100e100),
    m_maxDigits(-1),
    m_decimalPlaces(5)
{
}

NumberEdit::~NumberEdit()
{
}


BEGIN_MESSAGE_MAP(NumberEdit, CEdit)
    //{{AFX_MSG_MAP(NumberEdit)
	ON_CONTROL_REFLECT(EN_UPDATE, OnUpdate)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()



/////////////////////////////////////////////////////////////////////////////
// NumberEdit message handlers

BOOL NumberEdit::PreTranslateMessage(MSG* pMsg) 
{
    if(pMsg->message == WM_CHAR)
    {
        m_updating = TRUE;
        if(isprint(pMsg->wParam) && !ValidateChar(pMsg->wParam))
        {
            //Ignore char
            m_updating = FALSE;
            return true;
        }

        m_updating = FALSE;
    }
    
    return CEdit::PreTranslateMessage(pMsg);
}



void NumberEdit::OnUpdate() 
{
    if(!m_updating)
    {
        m_updating = TRUE;

        CString currText;
        CString newText;
        int startChar;
        int endChar;

        GetSel(startChar, endChar);
        if(startChar == 0 && endChar == 0)
        {
            SetSel(0, -1);
        }
        GetWindowText(currText);

        for(int i = 0; i < currText.GetLength(); i++)
        {
            UINT ch = currText[i];

            if(ValidateChar(ch) && ValidateString(newText + CString((char)ch)))
            {
                newText += (char)ch;
            }
        }

        SetWindowText(newText);
        SetSel(startChar, endChar);

        m_updating = FALSE;
    }
}



/////////////////////////////////////////////////////////////////////////////
// NumberEdit API
void NumberEdit::setIntNumber(int number)
{
    CString str;

    switch(m_base)
    {
        case BASE_HEX:
            str.Format("%X", number);
            break;

        case BASE_DEC:
            str.Format("%d", number);
            break;

        case BASE_BIN:
            for(int i = 0; i < 32; i++, number <<= 1)
            {
                if(number & 0x80000000)
                {
                    str += "1";
                }
                else
                {
                    if(str.GetLength())
                    {
                        str += "0";
                    }
                }
            }

            break;
    }

    SetWindowText(str);

    OnUpdate();
}



void NumberEdit::setFloatNumber(double number)
{
    CString str;

    if(m_base == BASE_FLOAT)
    {
        CString format;
        CString str;

        format.Format("%%.%df", m_decimalPlaces);
        str.Format(format, number);
        SetWindowText(str);
    }

    OnUpdate();
}



int NumberEdit::getIntNumber()
{
    CString str;
    GetWindowText(str);
    return ConvertToIntNumber(str);
}



double NumberEdit::getFloatNumber()
{
    CString str;
    GetWindowText(str);
    return ConvertToFloatNumber(str);
}



////////////////////////////////////////////////////////////////////////////
// Processing

BOOL NumberEdit::ValidateChar(UINT& ch)
{
    switch(m_base)
    {
        case BASE_HEX:
            if(ch >= 'a' && ch <= 'f')
            {
                ch -= ('a' - 'A');
            }

            if((ch < 'A' || ch > 'F') &&
               (ch < '0' || ch > '9'))
            {
                return false;
            }

            break;

        case BASE_DEC:
            if(ch < '0' || ch > '9')
            {
                return false;
            }

            break;

        case BASE_BIN:
            if(ch != '0' && ch != '1')
            {
                return false;
            }

            break;

        case BASE_FLOAT:
            if((ch < '0' || ch > '9') && (ch != '.') && (ch != '-'))
            {
                return false;
            }

            break;

        default:
            ASSERT(false);
    }

    return true;
}



BOOL NumberEdit::ValidateString(CString& str)
{
    if(m_base == BASE_FLOAT)
    {
        double value = ConvertToFloatNumber(str);

        int decimalIndex = str.Find('.');
        if(str.ReverseFind('.') != decimalIndex)
        {
            return false;
        }

        if(str.Find('-') > 0 || str.ReverseFind('-') > 0)
        {
            return false;
        }

        int decimalPlaces;
        if(decimalIndex >= 0)
        {
            decimalPlaces = str.GetLength() - decimalIndex - 1;
        }
        else
        {
            decimalPlaces = 0;
        }

        return value >= m_minFloatValue && value <= m_maxFloatValue &&
               (str.GetLength() <= m_maxDigits || m_maxDigits <= 0) &&
               decimalPlaces <= m_decimalPlaces;
    }
    else
    {
        int value = ConvertToIntNumber(str);
        return value >= m_minIntValue && value <= m_maxIntValue &&
               (str.GetLength() <= m_maxDigits || m_maxDigits <= 0);
    }
}



int NumberEdit::ConvertToIntNumber(CString& str)
{
    switch(m_base)
    {
        case BASE_HEX:
            return strtol(str, NULL, 16);

        case BASE_DEC:
            return strtol(str, NULL, 10);

        case BASE_BIN:
            return strtol(str, NULL, 2);

        default:
            ASSERT(FALSE);
    }

    return 0;
}



double NumberEdit::ConvertToFloatNumber(CString& str)
{
    if(m_base == BASE_FLOAT)
    {
        return strtod(str, NULL);
    }

    return 0;
}




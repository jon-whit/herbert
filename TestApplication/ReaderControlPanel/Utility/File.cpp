


#include "File.h"
#include "Shellapi.h"
#include "Shlobj.h"
#include <sys/stat.h>


char    pszBuffer[MAX_PATH]        = { 0 };
char    pszInitialFolder[MAX_PATH] = { 0 };


int CALLBACK browseCallbackProc(HWND hwnd,UINT uMsg,LPARAM lp, LPARAM pData) 
{
    switch(uMsg) 
    {
        case BFFM_INITIALIZED:
            SendMessage(hwnd, BFFM_SETSELECTION, 1,(LPARAM) pszInitialFolder);
            break;

        case BFFM_SELCHANGED: 
        {
           // Set the status window to the currently selected path.
           if (SHGetPathFromIDList((LPITEMIDLIST) lp, pszBuffer)) 
              SendMessage(hwnd,BFFM_SETSTATUSTEXT, 0, (LPARAM)pszBuffer);
           break;
        }
        default:
           break;
    }
    return 0;
}





LPITEMIDLIST convertPathToLpItemIdList(const CString &path)
{
    LPITEMIDLIST  pidl;
    LPSHELLFOLDER pDesktopFolder;
    OLECHAR       olePath[MAX_PATH];
    ULONG         chEaten;
    ULONG         dwAttributes;
    HRESULT       hr;

    if (SUCCEEDED(SHGetDesktopFolder(&pDesktopFolder)))
    {
        MultiByteToWideChar(CP_ACP, MB_PRECOMPOSED, path, -1, olePath, MAX_PATH);
        hr = pDesktopFolder->ParseDisplayName(NULL,NULL,olePath,&chEaten, &pidl,&dwAttributes);
        pDesktopFolder->Release();
    }
    return pidl;
}



int getFolder(const CString& title, CString& folder, const CString& initialFolder)
{
    int          rc = IDABORT;

    LPMALLOC     pMalloc;
    BROWSEINFO   bi;
    LPITEMIDLIST pidl;
    CString      strPath1;
    CString      strPath2;

    strncpy(pszInitialFolder, initialFolder, sizeof(pszInitialFolder));

    bi.hwndOwner      = GetDesktopWindow();
    bi.pidlRoot       = NULL;
    bi.pszDisplayName = pszBuffer;
    bi.lpszTitle      = title;
    bi.ulFlags        = BIF_RETURNFSANCESTORS | BIF_RETURNONLYFSDIRS;
    bi.lpfn           = browseCallbackProc;
    bi.lParam         = 0;

    if (::SHGetMalloc(&pMalloc) == NOERROR)
    {
        if ((pidl = ::SHBrowseForFolder(&bi)) != NULL)
        {
            if (::SHGetPathFromIDList(pidl, pszBuffer))
            {
                folder = pszBuffer;
                rc = IDOK;
            }
            else
            {
                rc = IDABORT;
            }

            pMalloc->Free(pidl);
        }
        else
        {
            rc = IDCANCEL;
        }

        pMalloc->Release();
    }

    return rc;
}

void getFileNamesInFolder(std::vector<CString>& fileNames, const CString& path, const CString& filter)
{
    WIN32_FIND_DATA findFileData;
    HANDLE          handle;
    CString         fullPath = path;

    if(fullPath.Right(1) != "\\")
    {
        fullPath += "\\";
    }
    handle = FindFirstFile(fullPath + filter, &findFileData);
    if (handle != INVALID_HANDLE_VALUE) 
    {
        do
        {
            fileNames.push_back(fullPath + findFileData.cFileName);
        } while (FindNextFile(handle, &findFileData));

        FindClose(handle);
    }
}


BOOL verifyPath(const CString &path)
{
    BOOL         valid = FALSE;
    LPMALLOC     pMalloc;
    LPITEMIDLIST pidl;

    int loc = path.Find(':');
    if(loc != -1 && loc != 1 || path.Find(':', 2) >= 0)
    {
        return FALSE;
    }

    if (::SHGetMalloc(&pMalloc) == NOERROR)
    {
        if ((pidl = convertPathToLpItemIdList(path)) != NULL)
        {
            pMalloc->Free(pidl);
            valid = TRUE;
        }

        pMalloc->Release();
    }

    return valid;
}


BOOL verifyFileExists(const CString &fileName)
{
    struct _stat statBuf;
    return _stat(fileName, &statBuf) == 0;
}



CString getPath(const char* path)
{
   char drive[_MAX_DRIVE];
   char dir[_MAX_DIR];
   char fname[_MAX_FNAME];
   char ext[_MAX_EXT];

   _splitpath(path, drive, dir, fname, ext);

   return CString(dir);
}

CString getDriveAndPath(const char* path)
{
   char drive[_MAX_DRIVE];
   char dir[_MAX_DIR];
   char fname[_MAX_FNAME];
   char ext[_MAX_EXT];

   _splitpath(path, drive, dir, fname, ext);

   return CString(drive) + CString(dir);
}


CString getDrivePathAndFile(const char* path)
{
   char drive[_MAX_DRIVE];
   char dir[_MAX_DIR];
   char fname[_MAX_FNAME];
   char ext[_MAX_EXT];

   _splitpath(path, drive, dir, fname, ext);

   return CString(drive) + CString(dir) + CString(fname);
}


CString getFileName(const char* path)
{
   char drive[_MAX_DRIVE];
   char dir[_MAX_DIR];
   char fname[_MAX_FNAME];
   char ext[_MAX_EXT];

   _splitpath(path, drive, dir, fname, ext);

   return CString(fname);
}


CString getFileNameAndExt(const char* path)
{
   char drive[_MAX_DRIVE];
   char dir[_MAX_DIR];
   char fname[_MAX_FNAME];
   char ext[_MAX_EXT];

   _splitpath(path, drive, dir, fname, ext);

   return CString(fname) + CString(ext);
}


CString getFileExt(const char* path)
{
   char drive[_MAX_DRIVE];
   char dir[_MAX_DIR];
   char fname[_MAX_FNAME];
   char ext[_MAX_EXT];

   _splitpath(path, drive, dir, fname, ext);

   return CString(ext);
}



CString fullPath(const char* relativePath)
{
    char fpath[_MAX_PATH];

    return CString(_fullpath(fpath, relativePath, sizeof(fpath)));
}

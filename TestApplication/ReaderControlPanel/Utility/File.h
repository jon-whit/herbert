#ifndef GetFolder_h
#define GetFolder_h

#include <afxwin.h>
#include <vector>

int     getFolder(const CString& title, CString& folder, const CString& initialFolder = "");
void    getFileNamesInFolder(std::vector<CString>& fileNames, const CString& path, const CString& filter = "*.*");
BOOL    verifyPath(const CString &path);
BOOL    verifyFileExists(const CString &fileName);

CString getPath(const char* path);
CString getDriveAndPath(const char* path);
CString getDrivePathAndFile(const char* path);
CString getFileName(const char* path);
CString getFileNameAndExt(const char* path);
CString getFileExt(const char* path);

CString fullPath(const char* relativePath);

#endif

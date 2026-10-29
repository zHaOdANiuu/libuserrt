/*
 * Copyright (C) 2026 zhaodaniu <zhaodaniu1@gmail.com>
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 3 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, see
 * <https://www.gnu.org/licenses/>.
 */

#ifdef _WIN32
#define UNICODE
#define NOMINMAX
#define VC_EXTRALEAN
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <wchar.h>
#include <shobjidl.h>
#else
#error "ERROR"
#endif

#define LIBUSERRT_KEEP
#include "libuserrt.h"

#include <assert.h>


#ifdef _WIN32
#define W(fun) _w##fun
#define T(s) L##s
#else
#define W(fun) fun
#define T(s) s
#endif


#define likely(x) __builtin_expect(!!(x), 1)
#define unlikely(x) __builtin_expect(!!(x), 0)

#define DIRECTORY_SEP '/'
#ifdef _WIN32
#define IS_DEVICE_SEP(_c_) ((_c_) == ':')
#define IS_DIRECTORY_SEP(_c_) ((_c_) == '/' || (_c_) == '\\')
#define IS_PATH_SEP(_c_) (IS_DIRECTORY_SEP(_c_) || IS_DEVICE_SEP(_c_))
#else
#define IS_DEVICE_SEP(_c_) ((_c_) == ':')
#define IS_DIRECTORY_SEP(_c_) ((_c_) == DIRECTORY_SEP)
#endif

#ifndef offsetof
#define offsetof(type, field) ((size_t)&((type*)0)->field)
#endif
#ifndef countof
#define countof(x) (sizeof(x) / sizeof((x)[0]))
#endif
#ifndef container_of
/* return the pointer of type 'type *' containing 'ptr' as field 'member' */
#define container_of(ptr, type, member) \
  ((type*)((uint8_t*)(ptr) - offsetof(type, member)))
#endif


bool executableDirectory(tchar* dir, u64 size) {
  DWORD len = GetModuleFileNameW(NULL, dir, (DWORD)size);

  if (len == 0 || len >= size)
    return false;

  tchar* result = wcsrchr(dir, L'\\');
  if (result == NULL)
    return false;

  *result = L'\0';
  return true;
}

tchar* getCurrentWorkingDirectory(tchar* buf, u64 bufSize) {
  return W(getcwd)(buf, bufSize);
}


/**************************************
 * File
 **************************************/


/*
 * @test
 * tchar buf[260];
 * const tchar* dir[] = { T("~/.emacs.d"), T("init.el") };
 * FileNameConcat(buf, dir, sizeof(dir) / sizeof(dir[0]));
 * assert(
 * #ifdef _WIN32
 *   wcscmp
 * #else
 *   strcmp
 * #endif
 *   (buf, L"~/.emacs.d/init.el") == 0
 * );
 */
void FileNameConcat(tchar* buf, const tchar* dir[], u64 count) {
  for (u64 i = 0; i < count; ++i) {
    const tchar* path = dir[i];

    assert(path);
    if (!*path)
      continue;

    while (*path)
      *buf++ = *path++;

    if (!IS_DIRECTORY_SEP(*(buf - 1)))
      if (i < count - 1)
        *buf++ = DIRECTORY_SEP;
  }
  *buf = T('\0');
}

const tchar* FileNameNoDirectory(const tchar* path, u64 pathLen) {
  const tchar* end = path + pathLen;
  while (end != path && !IS_DIRECTORY_SEP(*(end - 1)))
    --end;
  return end;
}

FILE* FileOpen(const tchar* path, const tchar* mode) {
  return W(fopen)(path, mode);
}

bool FileRead(FILE* f, char* buf, u64 chars) {
  return fread(buf, 1, chars, f) == chars;
}

bool FileWrite(FILE* f, tchar* text, u64 len) {
  return fwrite(text, 1, len, f) == len && !ferror(f);
}

bool FileClose(FILE* f) {
  return fclose(f) == 0;
}

bool FileEof(FILE* f) {
  return feof(f) != 0;
}

bool FileCreateLink(
  const tchar*   path,
  const tchar*   targetPath,
  FILE_LINK_TYPE type
) {
  if (type == FL_SYMBOLIC)
    return CreateSymbolicLinkW(path, targetPath, 0) != 0;
  assert(type == FL_HARD);
  return CreateHardLinkW(path, targetPath, 0) != 0;
}

bool FileCopy(const tchar* path, const tchar* targetPath) {
  return CopyFileW(path, targetPath, false) != 0;
}

bool FileMove(const tchar* path, const tchar* targetPath) {
  return W(rename)(path, targetPath) == 0;
}

bool FileRemove(const tchar* path) {
  return W(remove)(path) == 0;
}

bool FileMetaDataQuery(const tchar* path, FileMetaData* d) {
  return GetFileAttributesExW(path, GetFileExInfoStandard, d);
}

FILE_META_TYPE FileMetaType(const FileMetaData d) {
  DWORD type = d.dwFileAttributes;
  if (type == INVALID_FILE_ATTRIBUTES)
    return FM_UNKNOWN;
  if (type & FILE_ATTRIBUTE_REPARSE_POINT)
    return FM_LINK;
  if (type & FILE_ATTRIBUTE_DIRECTORY)
    return FM_DIRECTORY;
  return FM_FILE;
}

u64 FileMetaSize(const FileMetaData d) {
  if (d.dwFileAttributes == INVALID_FILE_ATTRIBUTES)
    return 0;

  ULARGE_INTEGER size;
  size.LowPart  = d.nFileSizeLow;
  size.HighPart = d.nFileSizeHigh;
  return size.QuadPart;
}

u64 FileMetaCreated(const FileMetaData d) {
  if (d.dwFileAttributes == INVALID_FILE_ATTRIBUTES)
    return 0;
  ULARGE_INTEGER v;
  v.LowPart  = d.ftCreationTime.dwLowDateTime;
  v.HighPart = d.ftCreationTime.dwHighDateTime;
  return v.QuadPart;
}

u64 FileMetaModified(const FileMetaData d) {
  if (d.dwFileAttributes == INVALID_FILE_ATTRIBUTES)
    return 0;
  ULARGE_INTEGER v;
  v.LowPart  = d.ftLastWriteTime.dwLowDateTime;
  v.HighPart = d.ftLastWriteTime.dwHighDateTime;
  return v.QuadPart;
}

bool FileMetaReadonly(const FileMetaData d) {
  if (d.dwFileAttributes == INVALID_FILE_ATTRIBUTES)
    return false;
  return (d.dwFileAttributes & FILE_ATTRIBUTE_READONLY) != 0;
}

bool FileMetaExists(const FileMetaData d) {
  return d.dwFileAttributes != INVALID_FILE_ATTRIBUTES;
}

/*
 * @global
 * bool testFileOpenDialogEnumCallback(const tchar* name, void* ctx) {
 *   (void)ctx;
 *   printf(
 * #ifdef _WIN32
 *     "%ls\n",
 * #else
 *     "%s\n"
 * #endif
 *     name
 *   );
 *   return false;
 * }
 */

/*
 * @no-auto
 * @test
 * const tchar* filter[][2] = {
 *   { T("text file"), T("*.txt") },
 *   { T("All Files"), T("*.*") },
 * };
 * FileDialogOpen(
 *   T("File open dialog test"),
 *   filter, sizeof(filter) / sizeof(filter[0]),
 *   false, true, testFileOpenDialogEnumCallback, NULL
 * );
 */
no_inline i32 FileDialogOpen(
  const tchar*               prompt,
  const tchar*               filter[][2],
  u64                        filterNum,
  bool                       isDirType,
  bool                       multiSelect,
  FileOpenDialogEnumCallback callback,
  void*                      ctx
) {
  HRESULT               hr;
  FILEOPENDIALOGOPTIONS options;
  IFileOpenDialog*      pFileOpen = NULL;
  IShellItemArray*      pResults  = NULL;
  IEnumShellItems*      pEnum     = NULL;
  IShellItem*           pItem     = NULL;
  PWSTR                 pszPath   = NULL;
  bool                  enumStop  = false;

#if defined(NTDDI_WIN10_RS1) && (NTDDI_VERSION >= NTDDI_WIN10_RS1)
  DPI_AWARENESS_CONTEXT oldContext
    = SetThreadDpiAwarenessContext(DPI_AWARENESS_CONTEXT_PER_MONITOR_AWARE_V2);
#endif

  hr = CoInitializeEx(NULL, COINIT_APARTMENTTHREADED | COINIT_DISABLE_OLE1DDE);
  if (unlikely(FAILED(hr)))
    goto clean0;

  hr = CoCreateInstance(
    &CLSID_FileOpenDialog, NULL, CLSCTX_ALL, &IID_IFileOpenDialog,
    (void**)&pFileOpen
  );
  if (unlikely(FAILED(hr)))
    goto clean1;

  hr = pFileOpen->lpVtbl->GetOptions(pFileOpen, &options);
  if (unlikely(FAILED(hr)))
    goto clean1;

  // args fill
  hr = pFileOpen->lpVtbl->SetTitle(pFileOpen, prompt);
  if (unlikely(FAILED(hr)))
    goto clean1;

  hr = pFileOpen->lpVtbl->SetFileTypes(
    pFileOpen, filterNum, (COMDLG_FILTERSPEC*)filter
  );
  if (unlikely(FAILED(hr)))
    goto clean1;

  if (isisDirType) {
    hr = pFileOpen->lpVtblSetOptions(options | FOS_PICKFOLDERS);
    if (unlikely(FAILED(hr)))
      goto clean1;
  }

  if (multiSelect) {
    hr = pFileOpen->lpVtbl->SetOptions(pFileOpen, opt | FOS_ALLOWMULTISELECT);
    if (unlikely(FAILED(hr)))
      goto clean1;
  }

  // start
  hr = pFileOpen->lpVtbl->Show(pFileOpen, NULL);
  if (unlikely(FAILED(hr)))
    goto clean1;

  hr = pFileOpen->lpVtbl->GetResults(pFileOpen, &pResults);
  if (unlikely(FAILED(hr)))
    goto clean2;

  hr = pResults->lpVtbl->EnumItems(pResults, &pEnum);
  if (unlikely(FAILED(hr)))
    goto clean3;

  for (;;) {
    hr = pEnum->lpVtbl->Next(pEnum, 1, &pItem, NULL);
    if (hr == S_FALSE) {
      hr = S_OK;
      break;
    }

    if (unlikely(FAILED(hr)))
      break;

    hr = pItem->lpVtbl->GetDisplayName(pItem, SIGDN_FILESYSPATH, &pszPath);
    pItem->lpVtbl->Release(pItem);
    pItem = NULL;

    if (unlikely(FAILED(hr)))
      enumStop = TRUE;
    else
      enumStop = callback(pszPath, ctx) != 0;

    if (pszPath) {
      CoTaskMemFree(pszPath);
      pszPath = NULL;
    }

    if (enumStop)
      break;
  }

clean3:
  if (pEnum)
    pEnum->lpVtbl->Release(pEnum);
clean2:
  if (pResults)
    pResults->lpVtbl->Release(pResults);
clean1:
  if (pFileOpen)
    pFileOpen->lpVtbl->Release(pFileOpen);

  CoUninitialize();

clean0:
#if defined(NTDDI_WIN10_RS1) && (NTDDI_VERSION >= NTDDI_WIN10_RS1)
  SetThreadDpiAwarenessContext(oldContext);
#endif
  return hr;
}


/**************************************
 * Directory
 **************************************/


/*
 * @test
 * DirectoryEnum(T("."), T("*"), testFileOpenDialogEnumCallback, NULL);
 */
i32 DirectoryEnum(
  const tchar*          path,
  const tchar*          mask,
  DirectoryEnumCallback callback,
  void*                 ctx
) {
  tchar        searchPath[MAX_PATH];
  const tchar* dir[] = { path, mask ? mask : L"*" };
  FileNameConcat(searchPath, dir, sizeof(dir) / sizeof(dir[0]));

  HANDLE           hFind;
  WIN32_FIND_DATAW ffd;

  hFind = FindFirstFileExW(
    searchPath, FindExInfoBasic, &ffd, FindExSearchNameMatch, NULL,
    FIND_FIRST_EX_LARGE_FETCH
  );
  if (hFind == INVALID_HANDLE_VALUE)
    return GetLastError();

  i32 err = 0;
  do {
    // clang-format off
    if (ffd.cFileName[0] == L'.'
        && (ffd.cFileName[1] == L'\0' || (ffd.cFileName[1] == L'.'
        && ffd.cFileName[2] == L'\0')))
      continue;
    // clang-format on
    if (callback(ffd.cFileName, ctx))
      goto end;
  } while (FindNextFileW(hFind, &ffd));

  i32 err2 = GetLastError();
  if (err2 != ERROR_NO_MORE_FILES)
    err = err2;

end:
  FindClose(hFind);
  return err;
}


/**************************************
 * Process
 **************************************/


static bool makePipe(
  PHANDLE               hParent,
  PHANDLE               hChild,
  LPSECURITY_ATTRIBUTES lpPipeAttributes,
  DWORD                 nSize,
  bool                  parentWrites
) {
  SECURITY_ATTRIBUTES saCli = { 0 };
  if (lpPipeAttributes)
    saCli = *lpPipeAttributes;
  saCli.nLength = sizeof(saCli);

  if (nSize == 0)
    nSize = 64 * 1024;

  static LONG counter = 0;
  tchar       name[256];
  _stprintf_s(
    name, _countof(name), _T("\\\\.\\pipe\\anonymous_%lu_%ld"),
    GetCurrentProcessId(), InterlockedIncrement(&counter)
  );

  DWORD clientAccess = parentWrites ? GENERIC_READ : GENERIC_WRITE;
  DWORD serverMode
    = FILE_FLAG_OVERLAPPED
      | (parentWrites ? PIPE_ACCESS_OUTBOUND : PIPE_ACCESS_INBOUND);

  HANDLE hServer = CreateNamedPipe(
    name, serverMode, PIPE_TYPE_BYTE | PIPE_READMODE_BYTE | PIPE_WAIT, 1,
    nSize, nSize, 0, &saCli
  );
  if (hServer == INVALID_HANDLE_VALUE)
    return false;

  HANDLE hClient = CreateFile(
    name, clientAccess, 0, &saCli, OPEN_EXISTING, FILE_FLAG_OVERLAPPED, NULL
  );
  if (hClient == INVALID_HANDLE_VALUE) {
    CloseHandle(hServer);
    return false;
  }

  *hParent = hServer;
  *hChild  = hClient;
  return true;
}

bool ProcessCreate(Process* p) {
  if (!p || !p->command || p->command[0] == '\0')
    return false;

  p->stdInput  = INVALID_HANDLE_VALUE;
  p->stdOutput = INVALID_HANDLE_VALUE;
  p->stdError  = INVALID_HANDLE_VALUE;
  p->process   = NULL;

  SECURITY_ATTRIBUTES sa  = { 0 };
  sa.nLength              = sizeof(sa);
  sa.bInheritHandle       = TRUE;
  sa.lpSecurityDescriptor = NULL;

  HANDLE hInRead, hInWrite;
  if (!makePipe(&hInWrite, &hInRead, &sa, 0, true))
    goto clean0;

  HANDLE hOutRead, hOutWrite;
  if (!makePipe(&hOutRead, &hOutWrite, &sa, 0, false))
    goto clean1;

  HANDLE hErrRead, hErrWrite;
  if (!makePipe(&hErrRead, &hErrWrite, &sa, 0, false))
    goto clean2;

  STARTUPINFO si = { 0 };
  si.cb          = sizeof(si);
  si.dwFlags     = STARTF_USESTDHANDLES;
  si.hStdInput   = hInRead;
  si.hStdOutput  = hOutWrite;
  si.hStdError   = hErrWrite;

  PROCESS_INFORMATION pi = { 0 };

  if (!CreateProcess(
        NULL, p->command, NULL, NULL, TRUE, CREATE_NO_WINDOW, NULL, NULL, &si,
        &pi
      ))
    goto clean3;

  CloseHandle(hInRead);
  CloseHandle(hOutWrite);
  CloseHandle(hErrWrite);
  CloseHandle(pi.hThread);

  p->stdInput  = hInWrite;
  p->stdOutput = hOutRead;
  p->stdError  = hErrRead;
  p->process   = pi.hProcess;

  return true;

clean3:
  CloseHandle(pi.hProcess);
clean2:
  CloseHandle(hErrRead);
  CloseHandle(hErrWrite);
clean1:
  CloseHandle(hOutRead);
  CloseHandle(hOutWrite);
clean0:
  CloseHandle(hInRead);
  CloseHandle(hInWrite);

  return false;
}

i32 ProcessRead(ProcessHandle stream, char* buf, u32 size) {
  if (stream == INVALID_HANDLE_VALUE || !buf || size == 0)
    return -1;

  DWORD      n  = 0;
  OVERLAPPED ov = { 0 };

  if (!ReadFile(stream, buf, size, &n, &ov)) {
    DWORD err = GetLastError();
    if (err == ERROR_IO_PENDING) {
      if (!GetOverlappedResult(stream, &ov, &n, TRUE)) {
        err = GetLastError();
        if (err == ERROR_BROKEN_PIPE || err == ERROR_HANDLE_EOF)
          return 0;
        return -1;
      }
    } else if (err == ERROR_BROKEN_PIPE || err == ERROR_HANDLE_EOF) {
      return 0;
    } else {
      return -1;
    }
  }

  return n;
}

i32 ProcessWrite(Process* p, const char* str, u32 len) {
  if (!p || p->stdInput == INVALID_HANDLE_VALUE || !str || len == 0)
    return -1;

  DWORD total = 0;

  while (len > 0) {
    OVERLAPPED ov = { 0 };
    DWORD      n  = 0;

    if (!WriteFile(p->stdInput, (LPVOID)(str + total), len, &n, &ov)) {
      DWORD err = GetLastError();
      if (err == ERROR_IO_PENDING) {
        if (!GetOverlappedResult(p->stdInput, &ov, &n, TRUE)) {
          err = GetLastError();
          if (err == ERROR_BROKEN_PIPE || err == ERROR_NO_DATA)
            return total;
          return -1;
        }
      } else if (err == ERROR_BROKEN_PIPE || err == ERROR_NO_DATA) {
        return total;
      } else {
        return -1;
      }
    }

    if (n == 0)
      break;

    total += n;
    len -= n;
  }

  return total;
}

bool ProcessClose(Process* p) {
  if (!p)
    return false;

  if (p->stdInput != INVALID_HANDLE_VALUE && p->stdInput != NULL) {
    CloseHandle(p->stdInput);
    p->stdInput = INVALID_HANDLE_VALUE;
  }
  if (p->stdOutput != INVALID_HANDLE_VALUE && p->stdOutput != NULL) {
    CloseHandle(p->stdOutput);
    p->stdOutput = INVALID_HANDLE_VALUE;
  }
  if (p->stdError != INVALID_HANDLE_VALUE && p->stdError != NULL) {
    CloseHandle(p->stdError);
    p->stdError = INVALID_HANDLE_VALUE;
  }
  if (p->process != NULL) {
    CloseHandle(p->process);
    p->process = NULL;
  }

  return true;
}

bool ProcessKill(Process* p) {
  if (!p || p->process == NULL)
    return false;

  if (!TerminateProcess(p->process, 0))
    return false;

  WaitForSingleObject(p->process, INFINITE);
  return true;
}

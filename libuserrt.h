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

#pragma once

#ifndef LIBUSERRT_H
#define LIBUSERRT_H

#include <stdint.h>
#include <stdbool.h>
#include <stdio.h>
#ifdef _WIN32
#include <fileapi.h>
#endif


#define no_inline __attribute__((noinline))
#define force_inline inline __attribute__((always_inline))


/**************************************
 * Basic type define
 **************************************/


#ifdef _WIN32
#define tchar wchar_t
#else
#define tchar char
#endif

#define f32 float
#define f64 double
#define i8 int8_t
#define i16 int16_t
#define i32 int32_t
#define i64 int64_t
#define u8 uint8_t
#define u16 uint16_t
#define u32 uint32_t
#define u64 uint64_t


#ifdef __cplusplus
extern "C" {
#endif

/**************************************
 * executable
 **************************************/


/***/
extern bool executableDirectory(tchar* dir, u64 size);


/**************************************
 * extra
 **************************************/


/***/
extern force_inline tchar* getCurrentWorkingDirectory(tchar* buf, u64 bufSize);


/**************************************
 * File
 **************************************/


typedef enum {
  FM_UNKNOWN,
  FM_FILE,
  FM_DIRECTORY,
  FM_LINK,
} FILE_META_TYPE;

typedef enum {
  FL_SYMBOLIC,
  FL_HARD,
} FILE_LINK_TYPE;

#ifdef _WIN32
typedef WIN32_FILE_ATTRIBUTE_DATA FileMetaData;
#else
typedef struct stat FileMetaData;
#endif

/***/
extern void FileNameConcat(tchar* buf, const tchar* paths[], u64 count);

/***/
extern const tchar* FileNameNoDirectory(const tchar* path, u64 pathLen);

/***/
extern force_inline FILE* FileOpen(const tchar* path, const tchar* mode);

/***/
extern force_inline bool FileRead(FILE* f, char* buf, u64 chars);

/***/
extern force_inline bool FileWrite(FILE* f, tchar* text, u64 len);

/***/
extern force_inline bool FileClose(FILE* f);

/***/
extern force_inline bool FileEof(FILE* f);

/***/
extern force_inline bool FileCreateLink(
  const tchar*   path,
  const tchar*   targetPath,
  FILE_LINK_TYPE type
);

/***/
extern force_inline bool FileCopy(const tchar* path, const tchar* targetPath);

/***/
extern force_inline bool FileMove(const tchar* path, const tchar* targetPath);

/***/
extern force_inline bool FileRemove(const tchar* path);

/***/
extern force_inline bool FileMetaDataQuery(const tchar* path, FileMetaData* d);

/***/
extern FILE_META_TYPE FileMetaType(const FileMetaData d);

/***/
extern force_inline u64 FileMetaSize(const FileMetaData d);

/***/
extern force_inline u64 FileMetaCreated(const FileMetaData d);

/***/
extern force_inline u64 FileMetaModified(const FileMetaData d);

/***/
extern force_inline bool FileMetaReadonly(const FileMetaData d);

/***/
extern force_inline bool FileMetaExists(const FileMetaData d);

/***/
typedef bool (*FileOpenDialogEnumCallback)(const tchar* name, void* ctx);

/***/
extern no_inline i32 FileDialogOpen(
  const tchar*               prompt,
  const tchar*               filter[][2],
  u64                        filterNum,
  bool                       isDirType,
  bool                       multiSelect,
  FileOpenDialogEnumCallback callback,
  void*                      ctx
);


/**************************************
 * Directory
 **************************************/


/***/
typedef bool (*DirectoryEnumCallback)(const tchar* name, void* ctx);
/***/
extern i32 DirectoryEnum(
  const tchar*          path,
  const tchar*          mask,
  DirectoryEnumCallback callback,
  void*                 ctx
);


/**************************************
 * Process
 **************************************/


typedef HANDLE ProcessHandle;

typedef struct {
  ProcessHandle stdInput;
  ProcessHandle stdOutput;
  ProcessHandle stdError;
  ProcessHandle process;
  tchar*        command;
} Process;

/***/
extern bool ProcessCreate(Process* p);
/***/
extern i32 ProcessRead(ProcessHandle stream, char* buf, u32 size);
/***/
extern i32 ProcessWrite(Process* p, const char* str, u32 len);
/***/
extern bool ProcessClose(Process* p);
/***/
extern bool ProcessKill(Process* p);

#ifdef __cplusplus
}
#endif


#ifndef LIBUSERRT_KEEP
#undef tchar
#undef f64
#undef i8
#undef i16
#undef i32
#undef i64
#undef u8
#undef u16
#undef u32
#undef u64
#undef f32
#undef no_inline
#undef force_inline
#endif


#endif

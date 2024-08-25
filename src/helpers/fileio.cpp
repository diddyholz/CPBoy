#include "fileio.h"

#include <stdint.h>
#include <string.h>
#include <sdk/os/file.hpp>
#include <sdk/os/mcs.hpp>
#include "../core/error.h"
#include "functions.h"

uint8_t _write_mcs(const char *dir, const char *name, void *buf, size_t size, 
  const char *err_file, uint32_t err_line)
{
  int32_t ret = MCS_SetVariable(dir, name, VARTYPE_STR, size, buf);

  if (ret != 0)
  {
    char err_info[100] = "mcs\\";
    char tmp[10];

    itoa(ret, tmp, 16);

    strlcat(err_info, dir, sizeof(err_info));
    strlcat(err_info, "\\", sizeof(err_info));
    strlcat(err_info, name, sizeof(err_info));
    strlcat(err_info, ": 0x", sizeof(err_info));
    strlcat(err_info, tmp, sizeof(err_info));

    _set_error(EFWRITE, err_file, err_line, err_info);
    return 1;
  }

  return 0;
}

uint8_t _read_mcs(const char *dir, const char *name, void **buf, uint32_t *size, 
  const char *err_file, uint32_t err_line)
{
  char *name2;
  uint8_t var_type;

  int32_t ret = MCS_GetVariable(dir, name, &var_type, &name2, buf, size);

  if (ret != 0)
  {
    char err_info[100] = "mcs\\";
    char tmp[10];

    itoa(ret, tmp, 16);

    strlcat(err_info, dir, sizeof(err_info));
    strlcat(err_info, "\\", sizeof(err_info));
    strlcat(err_info, name, sizeof(err_info));
    strlcat(err_info, ": 0x", sizeof(err_info));
    strlcat(err_info, tmp, sizeof(err_info));

    _set_error(EFREAD, err_file, err_line, err_info);
    return 1;
  }

  return 0;
}

uint8_t _write_file(const char *file, void *buf, size_t len, const char *err_file, uint32_t err_line)
{
  int32_t fd = File_Open(file, FILE_OPEN_WRITE | FILE_OPEN_CREATE);

  char err_info[ERROR_MAX_INFO_LEN];
  strlcpy(err_info, "w: ", sizeof(err_info));
  strlcat(err_info, file, sizeof(err_info));

  if (fd < 0)
  {
    _set_error(EFOPEN, err_file, err_line, err_info);
    return 1;
  }

	if (File_Write(fd, buf, len) < 0)
  {
	  File_Close(fd);

    _set_error(EFWRITE, err_file, err_line, err_info);
    return 1;
  }

	if (File_Close(fd) < 0)
  {
    _set_error(EFCLOSE, err_file, err_line, err_info);
    return 1;
  }

  return 0;
}

uint8_t _read_file(const char *file, void *buf, size_t len, const char *err_file, uint32_t err_line)
{
  int32_t fd = File_Open(file, FILE_OPEN_READ);

  char err_info[ERROR_MAX_INFO_LEN];
  strlcpy(err_info, "r: ", sizeof(err_info));
  strlcat(err_info, file, sizeof(err_info));

  if (fd < 0)
  {
    _set_error(EFOPEN, err_file, err_line, err_info);
    return 1;
  }

	if (File_Read(fd, buf, len) < 0)
  {
	  File_Close(fd);

    _set_error(EFREAD, err_file, err_line, err_info);
    return 1;
  }

	if (File_Close(fd) < 0)
  {
    _set_error(EFCLOSE, err_file, err_line, err_info);
    return 1;
  }

  return 0;
}

uint8_t _delete_file(const char *file, const char *err_file, uint32_t err_line) 
{
	if (File_Remove(file) < 0)
  {
    _set_error(EFCLOSE, err_file, err_line, file);
    return 1;
  }

  return 0;
}

uint8_t _get_file_size(const char *file, size_t *size, const char *err_file, uint32_t err_line)
{
	int32_t fd = File_Open(file, FILE_OPEN_READ);
  
  char err_info[ERROR_MAX_INFO_LEN];
  strlcpy(err_info, "r: ", sizeof(err_info));
  strlcat(err_info, file, sizeof(err_info));
	
	if (fd < 0)
  {
    _set_error(EFOPEN, err_file, err_line, err_info);
		return 1;
  }

	struct File_Stat file_stat;

	if (File_Fstat(fd, &file_stat) < 0)
  {
    _set_error(EFREAD, err_file, err_line, err_info);
		return 1;
  }

	if (File_Close(fd) < 0)
  {
    _set_error(EFCLOSE, err_file, err_line, err_info);
		return 1;
  }  

  *size = file_stat.fileSize;

  return 0;
}

uint8_t find_files(const char *path, char (*buf)[MAX_FILENAME_LEN], uint8_t max)
{
  if (max == 0)
  {
    return 0;
  }

  wchar_t wpath[MAX_FILENAME_LEN];
  wchar_t filename[MAX_FILENAME_LEN];
	struct File_FindInfo info;
	int handle;
  int32_t ret;
	uint8_t file_count = 0;

  char_to_wchar(wpath, path);

	ret = File_FindFirst(wpath, &handle, filename, &info);

	while(ret >= 0) 
	{
    // Check if this is a file
    if (info.type == info.EntryTypeFile)
    {
      // Check if filename begins with . (is hidden)
      if (filename[0] != '.')
      {
        // Copy file name
        wchar_to_char(buf[file_count], filename);
        file_count++;
      }
    }
		
		//serch the next
		ret = File_FindNext(handle, filename, &info);
	}

	File_FindClose(handle);

  return file_count;
}

// SPDX-FileCopyrightText: 2020 W. Traylor <wolfgang.traylor@senckenberg.de>
//
// SPDX-License-Identifier: LGPL-3.0-or-later

/**
 * \file
 * \brief Interaction with the filesystem on Unix and Windows platforms.
 * \copyright LGPL-3.0-or-later
 * \date 2019
 */
#include "fileystem.h"
#include <errno.h>
#include <sys/stat.h>
#include <fstream>
#include <iostream>
#include <stdexcept>
#if defined(_WIN32)
#include <direct.h>
#endif

void Fauna::create_directories(const std::string path, const mode_t mode) {
  bool failed = false;
  std::string error_msg;

#if defined(_WIN32)
  static const char PATH_SEPARATOR = '\\';
  int ret = _mkdir(path.c_str());
#else
  static const char PATH_SEPARATOR = '/';
  int ret = mkdir(path.c_str(), mode);
#endif
  switch (errno) {
    case ENOENT:  // No such file or directory.
      // parent didn't exist, try to create it
      {
        const int pos = path.find_last_of(PATH_SEPARATOR);
        if (pos == std::string::npos) {
          failed = true;
          error_msg = "Cannot define parent directory from path.";
          break;
        }
        create_directories(path.substr(0, pos));
        // now, try to create again
#if defined(_WIN32)
        const int success = (0 == _mkdir(path.c_str()));
#else
        const int success = (0 == mkdir(path.c_str(), mode));
#endif
        if (!success) failed = true;
      }
      break;
    case EEXIST:  // File exists
      break;      // nothing to do
    case EPERM:
      failed = true;
      error_msg = "Operation not permitted.";
      break;
    case EIO:
      failed = true;
      error_msg = "I/O error.";
      break;
    case EINVAL:
      failed = true;
      error_msg = "Invalid argument.";
      break;
    case ENOSPC:
      failed = true;
      error_msg = "No space left on device.";
      break;
    case ENXIO:
      failed = true;
      error_msg = "No such device or address.";
      break;
    case EROFS:
      failed = true;
      error_msg = "Read-only file system.";
      break;
    case EMLINK:
      failed = true;
      error_msg = "Too many links.";
      break;
    default:
      failed = true;
      break;
  }

  if (failed) {
    if (error_msg == "") error_msg = "Unspecified.";
    std::runtime_error(
        "Fauna::create_directories() "
        "Error creating directory '" +
        path +
        "'\n"
        "Message: " +
        error_msg);
  }
}

bool Fauna::directory_exists(const std::string path) {
#if defined(_WIN32)
  struct _stat info;
  if (_stat(path.c_str(), &info) != 0) {
    return false;
  }
  return (info.st_mode & _S_IFDIR) != 0;
#else
  struct stat info;
  if (stat(path.c_str(), &info) != 0) {
    return false;
  }
  return (info.st_mode & S_IFDIR) != 0;
#endif
}

bool Fauna::file_exists(const std::string& path) {
  std::ifstream f(path.c_str());
  return f.good();
}

void Fauna::remove_directory(const std::string& path) {
  if (!system(NULL))
    throw std::runtime_error(
        "Fauna::remove_directory() System shell is not available.");
  if (!directory_exists(path))
    throw std::invalid_argument(
        "Fauna:remove_directory() "
        "Directory does not exist: '" +
        path + "'");
#if defined(_WIN32)
  system("rd /s /q ...");
#else
  system(("rm -r -f '" + path + "'").c_str());
#endif
}

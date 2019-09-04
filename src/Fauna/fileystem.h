/**
 * \file
 * \brief Interaction with the filesystem on Unix and Windows platforms.
 * \copyright ...
 * \date 2019
 */
#ifndef FAUNA_FILESYSTEM_H
#define FAUNA_FILESYSTEM_H

#include <string>

namespace Fauna {

/// Create a folder including all its parent folders.
/**
 * This function works for Windows und Unix systems.
 * \param path The absolute or relative path to the final directory. As a file
 * separator use backslash (\) on Windows and slash (/) on Linux.
 * \param mode_t File permissions (only applicable to Unix filesystems.
 * \throw std::runtime_error If an error occurred.
 */
void create_directories(const std::string path, const mode_t mode = 0755);

/// Check whether a folder in the filesystem exists.
/**
 * This function works for Windows und Unix systems.
 * \param path The absolute or relative path to the directory.
 * \return True if `path` exists, false if not.
 */
bool directory_exists(const std::string path);

/// Check whether a file in the filesystem exists.
/**
 * This function works for Windows und Unix systems.
 * \param path The absolute or relative path to the file in question.
 * \return True if `path` exists, false if not.
 */
bool file_exists(const std::string& path);

/// Delete a directory recursively.
/**
 * \warning Don’t use this function (in its current implementation) for the
 * actual simulation code. It uses `system()` to call to the shell, but might
 * fail without warning.
 * \param path The absolute or relative path to the directory.
 * \throw std::invalid_argument If `path` does not exist.
 * \throw std::runtime_error If the system shell is not available throuh
 * `system()`.
 */
void remove_directory(const std::string& path);

}  // namespace Fauna

#endif  // FAUNA_FILESYSTEM_H

#ifndef FILESYSTEM_H
#define FILESYSTEM_H

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
}  // namespace Fauna

#endif  // FILESYSTEM_H

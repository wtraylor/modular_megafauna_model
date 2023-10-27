// SPDX-FileCopyrightText: 2020 W. Traylor <wolfgang.traylor@senckenberg.de>
//
// SPDX-License-Identifier: LGPL-3.0-or-later

/**
 * \file
 * \brief Unit test for cross-platform filesystem functions.
 * \copyright LGPL-3.0-or-later
 * \date 2019
 */
#include "fileystem.h"

#include <fstream>

#include "catch.hpp"

using namespace Fauna;

#ifdef __gnu_linux__
// On GNU/Linux we can use system() to call POSIX shell commands and the
// mkstemp() function (a POSIX addition to C++).
#include <sys/stat.h>

TEST_CASE("Fauna::create_directories()", "") {
  static const std::string DIRECTORY = "unit_tests/create_directories";
  REQUIRE(system(NULL));  // Check that a shell is available.

  SECTION("Create only one directory.") {
    char temp_dir[1024] = "create_directories_XXXX";
    mkstemp(temp_dir);
    create_directories(temp_dir);
    CHECK(system(((std::string) "dir '" + temp_dir + "'").c_str()) == 0);
    system(((std::string) "rmdir '" + temp_dir + "'").c_str());
  }

  SECTION("Create a directory tree.") {
    char temp_dir[1024] = "create_directories/a/b/c/d/XXXX";
    create_directories(temp_dir);
    CHECK(system(((std::string) "dir '" + temp_dir + "'").c_str()) == 0);
    system(((std::string) "rmdir --parents '" + temp_dir + "'").c_str());
  }
}

TEST_CASE("Fauna::directory_exists()", "") {
  // On a unix system, the /bin directory will always be there.
  CHECK(directory_exists("/bin"));
  CHECK(!directory_exists("/this_is_a_random_string"));
  CHECK(!directory_exists("this_is_a_random_string"));
}

TEST_CASE("Fauna::file_exists()", "") {
  // Check for some files that are certainly there in a UNIX system.
  CHECK(file_exists("/bin/sh"));
  CHECK(file_exists("/usr/bin/env"));
  CHECK(!file_exists("/this_is_a_random_string"));
  CHECK(!file_exists("this_is_a_random_string"));
}

TEST_CASE("Fauna::remove_directory()", "") {
  CHECK_THROWS(remove_directory("/this_is_a_random_string"));
  CHECK_THROWS(remove_directory("this_is_a_random_string"));

  static const std::string FOLDER = "remove_directory";

  create_directories(FOLDER);
  REQUIRE(directory_exists(FOLDER));

  SECTION("Delete empty directory") {
    remove_directory(FOLDER);
    CHECK(!directory_exists(FOLDER));
  }

  SECTION("Delete directory with subdirs") {
    create_directories(FOLDER + "/subdir1");
    REQUIRE(directory_exists(FOLDER + "/subdir1"));
    create_directories(FOLDER + "/subdir2");
    REQUIRE(directory_exists(FOLDER + "/subdir2"));
    remove_directory(FOLDER);
    CHECK(!directory_exists(FOLDER));
  }

  SECTION("Delete directory with files") {
    std::ofstream f1(FOLDER + "/file1");
    f1 << "content";
    f1.close();
    REQUIRE(file_exists(FOLDER + "/file1"));

    std::ofstream f2(FOLDER + "/file2");
    f2 << "content";
    f2.close();
    REQUIRE(file_exists(FOLDER + "/file2"));

    remove_directory(FOLDER);
    CHECK(!directory_exists(FOLDER));
  }
}

#endif  // __gnu_linux__

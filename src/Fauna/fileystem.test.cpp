#include "catch.hpp"
#include "fileystem.h"

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

#endif  // __gnu_linux__


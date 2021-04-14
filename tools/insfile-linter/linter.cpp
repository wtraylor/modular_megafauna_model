// SPDX-FileCopyrightText: 2020 W. Traylor <wolfgang.traylor@senckenberg.de>
//
// SPDX-License-Identifier: LGPL-3.0-or-later

/**
 * \file
 * \brief A small, stand-alone helper tool to check the MMM instruction file.
 * \copyright LGPL-3.0-or-later
 * \date 2020
 */
#include <fstream>
#include <iostream>
#include "megafauna.h"

using namespace Fauna;

/// Check the given instruction file.
/**
 * \param argc Number of CLI parameters. Must be 2.
 * \param argv Vector of CLI parameters, must only contain two
 * elements: 1) the program name, 2) the path to the TOML instruction file.
 */
int main(int argc, char* argv[]) {
  std::cerr << "Welcome to the instruction file linter of the Modular "
               "Megafauna Model."
            << std::endl;
  if (argc != 2) {
    std::cerr
        << "Please provide the path to the TOML file as the only argument."
        << std::endl;
    return EXIT_FAILURE;
  }

  assert(argc == 2);
  const std::string insfile = argv[1];

  std::unique_ptr<Fauna::World> fauna_world;

  // Check if file exists

  std::ifstream f(insfile.c_str());
  if (!f.good()) {
    std::cerr
        << "I cannot read the given file: "
        << "'" << insfile << "'" << std::endl
        << "Please check that the file exists and that we have read permission."
        << std::endl;
    return EXIT_FAILURE;
  }

  try {
    fauna_world.reset(new Fauna::World(insfile, SimMode::Lint));
  } catch (const std::runtime_error& e) {
    std::cerr << "The instruction file looks problematic:" << std::endl
              << std::endl
              << e.what() << std::endl;
    return EXIT_FAILURE;
  } catch (const std::exception& e) {
    std::cerr << "An unknown error occurred:" << std::endl
              << std::endl
              << e.what() << std::endl;
    return EXIT_FAILURE;
  }
  std::cerr << "The instruction file looks good." << std::endl;
  return EXIT_SUCCESS;
}

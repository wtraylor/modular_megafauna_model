#!/usr/bin/env bash

# SPDX-FileCopyrightText: 2020 Wolfgang Traylor <wolfgang.traylor@senckenberg.de>
#
# SPDX-License-Identifier: CC0-1.0

# Run cppclean on the codebase of the Modular Megafauna Model library.
# See: https://github.com/myint/cppclean

if [[ ! -f "README.md" ]] || [[ ! -d "src/" ]]; then
  echo >&2 "You don’t seem to execute this script from the root of the repository."
  exit 1
fi

find -iname '*.h' -or -iname '*.cpp' -not -iname '*.test.cpp' | xargs cppclean -I src/Fauna -I src/Fauna/Output -I tools/demo_simulator -I include -I include/Fauna -I include/Fauna/Output

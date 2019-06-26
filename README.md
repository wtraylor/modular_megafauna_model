Modular Megafauna Model
=======================

Overview
--------

Repository Structure
--------------------

This project follows the [Pitchfork Layout](https://github.com/vector-of-bool/pitchfork).

- `docs/`: Doxygen documentation.
- `examples/`: Exemplary instruction files.
- `include/`: Public API header files.
- `src/`: Source and (private) header files of the project. Subdirectories correspond to C++ namespaces.
- `tests/`: Unit tests.
- `tools/`: A dummy vegetation simulator that integrates the megafauna model.

Usage
-----

### Compile the Doxygen Documentation

### Integrate the Library into a Vegetation Model

Known Bugs and Issues
---------------------

License
-------

- The [Catch2](https://github.com/catchorg/Catch2) test framework (`tests/catch.hpp`) is licensed under the [Boost Software License](http://www.boost.org/LICENSE_1_0.txt).

- The version of the [PlantUML](http://plantuml.com) file (`docs/plantuml.jar`), which is used to render UML diagrams in the Doxygen documentation, is under the [MIT license](http://opensource.org/licenses/MIT).

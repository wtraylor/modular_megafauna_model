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
CMake will automatically compile the doxygen documentation along with the library in a subfolder `docs/` inside the build directory. In there open the `index.html` file with a browser.

```bash
mkdir build
cd build
cmake "<path to the megafauna library>"
make
# Open `docs/index.html`.
```

You can turn off the automatic compilation with the CMake variable `BUILD_DOC`: `cmake -DBUILD_DOC=OFF "<path>"`.
In order to generate _only_ the documentation, provide the path to the `docs` subfolder to `cmake`: `cmake /path/to/megafauna/docs`.

#### Optional Build Requirements for the Documentation
- Java Runtime Environment (JRE)
- [Graphviz](www.graphviz.org), see [here](http://plantuml.com/graphviz-dot) for details.
- [LaTeX](www.latex-project.org), including [BibTeX](www.bibtex.org).

### Integrate the Library into a Vegetation Model

Known Bugs and Issues
---------------------

License
-------

- The [Catch2](https://github.com/catchorg/Catch2) test framework (`tests/catch.hpp`) is licensed under the [Boost Software License](http://www.boost.org/LICENSE_1_0.txt).

- The version of the [PlantUML](http://plantuml.com) file (`docs/plantuml.jar`), which is used to render UML diagrams in the Doxygen documentation, is under the [MIT license](http://opensource.org/licenses/MIT).

# Contributing to the Codebase

This document is a guide for developers who want to contribute to the megafauna library.
It explains the organization of the software project and sets guidelines for code style, structure, and format.
This document is only about *syntax* only.

## Table of Contents

- [Version Control](#version-control)
    - [Branches](#branches)
    - [Release Versioning](#release-versioning)
    - [Commit Messages](#commit-messages)
    - [Continuous Integration](#continuous-integration)
- [Coding Guidelines](#coding-guidelines)
    - [Repository Structure](#repository-structure)
    - [Code Format](#code-format)
        - [Naming Code Elments](#naming-code-elements)
        - [Ordering](#ordering)
        - [File Header](#file-header)
    - [Unit Tests](#unit-tests)
    - [Code Checkers](#code-checkers)
    - [Doxygen Documentation](#doxygen-documentation)
        - [Markdown](#markdown)
        - [BibTeX Bibliography](#bibtex-bibliography)

## Version Control

### Branches
- The `master` branch is reserved for stable releases, tagged with the version numbers.
- This repository follows Vincent Driessen’s [Successful Git Branching Model](https://nvie.com/posts/a-successful-git-branching-model/).
- If you are new to Git branching, check out this tutorial: [Learn Git Branching](https://learngitbranching.js.org/)

### Release Versioning
- This project follows [Semantic Versioning](https://semver.org/spec/v2.0.0.html).
- Each merge into the `master` branch is a release and should have a Git tag.
    - The tag’s name is just the exact version, e.g. “0.1.2”.
    - The tag’s decription should summarize the introduced changes.
- Before merging into `master`:
    - Set the new version in `CMakeLists.txt`.
    - List your changes in `CHANGELOG.md`, following the formatting guidelines there.
- If you cite this code in a journal publication, consider getting a DOI for the specific model version you used.

### Commit Messages
Follow Chris Beams’ guide for crafting your Git commit messages: [How to Write a Git Commit Message](https://chris.beams.io/posts/git-commit/)

> 1. Separate subject from body with a blank line
> 1. Limit the subject line to 50 characters
> 1. Capitalize the subject line
> 1. Do not end the subject line with a period
> 1. Use the imperative mood in the subject line
> 1. Wrap the body at 72 characters
> 1. Use the body to explain what and why vs. how

### Continuous Integration
The repository should always be in a valid state.
A number of tests are defined in the file `.gitlab-ci.yml`.
This file works with GitLab Continuous Integration (CI).

The CI script also runs [Valgrind](https://valgrind.org) memory check.
With the bash script `tools/run_valgrind_memcheck` you can execute a memory check manually on your local machine.
**Always make sure contributions to the codebase don’t have memory leaks.**

## Coding Guidelines

### Repository Structure
This project follows the [Pitchfork Layout](https://github.com/vector-of-bool/pitchfork) for C++ projects.
Here is a summary of the relevant parts:

- **Namespace Folders:** The `src/` directory has subfolders reflecting the namespaces of the contained components. To minimize the danger of name collision, the header **include guards** contain the namespace hierarchy also, e.g. `FAUNA_OUTPUT_HABITAT_DATA_H`.
- **Separate Header Placement:** Header (`*.h`) and source (`*.cpp`) files are kept together in `src/` if they are *private* (not part of the library interface). *Public* headers are placed in `include/` while their corresponding source files remain in `src/`.
- **Coherence:** A header and corresponding source file (= *physical component*)contain code for *logical component.* If in doubt, rather air on the side of granularity and create several individual components.
- **Merged Test Placement:** Any logical/physical unit has its unit test in a file in the same folder with the same file name, but with the suffix `.test.cpp`.

### Code Format
The C++ code layout follows the [Google C++ Style Guide](https://google.github.io/styleguide/cppguide.html).
No worries, you don’t need to read everything.
Just see that your IDE or text editor auto-formats your code using [Clang](http://clang.llvm.org/).
The default Google layout is exported in the [.clang-format](.clang-format) file, ready to be read by `clang`.

This repository has a [.editorconfig](.editorconfig) file to define indentation rule for different file types.
Please install the plugin for your text editor if available: [editorconfig.org/](https://editorconfig.org/)

#### Naming Code Elements

- **Files** are always lower-case with underscores to separate words.
    - Header files end with `.h`, source files with `.cpp`, and the corresponding unit test files with `.test.cpp`.
    - If a file only contains one class, name the file like the class.
    - If a file contains several classes, use a plural like `net_energy_models.h`.
    - If a file contains a collection of functionality, use an abstract grouping noun, e.g. `stochasticity.h` or `nitrogen.h`.
- **Classes** are named in CamelCase with upper-case first letter, e.g. `MyExampleClass`. Don’t repeat the namespace in the class name (avoid something like `Output::OutputDataClass`).
    - **Enum** types are like classes.
- **Functions** are imperative verbs with underscores, e.g. `create_new_herbivores()`.
- Global **constants** as well as static const member and function variables are all-uppercase with underscores, e.g. `MY_GLOBAL_CONSTANT`.
    - C++11-style **enum class elements** don’t have global scope and thus don’t require a prefix. Since the shouting tone of all-uppercase names is distracting, just use CamelCase for the enum members, e.g. `OutputInterval::Annual`.
- **Namespaces** are short and lower-case with first letter capitalized, e.g. `Fauna`.

#### Ordering
An example class definition in a header file:
```cpp
class MyExampleClass{
 public: // -> public members first
  /// Constructor
  MyExampleClass(); // -> constructors are always first

  /// Destructor
  ~MyExampleClass(); // -> destructor comes next

  // -> The following member functions in alphabetical order:
  /// Create new herbivore instance.
  Herbivore* create_herbivore();

  /// Get the type of herbivore.
  HerbivoreType get_herbivore_type();

  /// A public global constant in this class.
  static const int MY_CONSTANT = 10;

 private:
  /// Helper function to perform calculations.
  void my_private_function();

  // -> Finally the private member variables NOT in alphabetical order, but in
  // the order of desired initialization.
  HerbivoreType herbi_type;
  int var = 10;
};
```

In the corresponding source file, *all* function definitions (both private & public) are in alphabetical order, except for the constructors and destructor, which come first.

If there is more than one class in the header file, separate their function definitions in blocks with big comment captions, for example like this:
```cpp
//============================================================
// HftPopulationsMap
//============================================================
```

If there are functions local to this file, put them in an anonymous namespace before any other definitions:
```cpp
namespace {
  int my_local_function(){
  // ...
  };
}
```

#### File Header
Begin each `.h` or `.cpp` file with a doxygen header containing a brief description.
The description will appear in the file list of the generated doxygen documentation.
Ususally the brief description will be the same for a `.h` and its `.cpp` file.
If it is only one class in the header file, you can also copy the `\brief` description from that class.
Here is an example:

<!--TODO: REUSE header-->

```cpp
/**
 * \file
 * \brief Management classes of herbivore populations.
 * \copyright LGPL-3.0-or-later
 * \date <current year>
 */
```

We omit the `\author` field because it might be difficult to keep track of all authors who have contributed.
(That’s what version control is for.)
Instead, all contributors of the project shall be listed collectively in the “Authors” section of the `README.md`.

The `\date` field is only relevant for the copyright. (Use Git to see when the file has been changed.)

### Unit Tests
Make sure to write a unit test for every logical component.
If you create a `.cpp` file, there should most likely also be a corresponding `.test.cpp` file that checks the public functions of the class or classes.

Unit tests use the [Catch2](https://github.com/catchorg/Catch2) framework in the [single header](https://raw.githubusercontent.com/catchorg/Catch2/master/single_include/catch2/catch.hpp) distribution.
(The `tests/catch.hpp` file can be updated from time to time.)
To run the unit tests after building the megafauna library, run `./megafauna_unit_tests` in the build directory.
To disable building the unit tests, you can call `cmake -DBUILD_TESTING=OFF /path/to/repo`.

### Code Checkers

Run [cppclean](https://github.com/myint/cppclean) on the code to find unnecessary `#include`s, unnecessary functions, and a lot more.
Execute the helper script `./tools/cppclean.sh` in the Bash.

### Doxygen Documentation

- The documentation is completely in English, preferably with American spelling.
- Images are placed in `docs/images/`. If the figure was plotted with a script, save the script in the same folder and with the same file name as the image.

#### Markdown
- Overview pages are written in [Markdown](http://www.doxygen.nl/manual/markdown.html) in `*.md`-files in the `docs/` folder.
- Make a new line after each sentence (and perhaps after logical sentence structures): [Inner Sentence Rule](https://cirosantilli.com/markdown-style-guide#option-wrap-inner-sentence).

#### BibTeX Bibliography
[BibTeX](www.bibtex.org) is used for bibliographic references: [docs/bibliography.bib](docs/bibliography.bib).
The Doxygen command `\cite` is used for that.
This makes browsing the Doxygen documentation easier.

In general you should not need to put any references to scientific publications in *comments* in the source code.
Better you explain everything in a narrative form in the Doxygen *documentation* and use the `\cite` command for that.
If you do cite in source code comments, make sure that the reference is uniquely identifiable in `bibliography.bib`.

Write the in-text citation in the doxygen documentation (either in a C++ file or in a Markdown document) in the [APA format](https://www.mendeley.com/guides/apa-citation-guide#2_In_Text) followed by the `\cite` reference:
```plain
Illius & O’Connor (2000) \cite illius2000resource states that ...
Blaxter (1989, p. 123) \cite blaxter1989energy states that ...
(McDonald et al. 2010, p. 123 \cite mcdonald2010animal)
```

Use [bibsort](http://ftp.math.utah.edu/pub/bibsort/) to sort the
bibliography entries by label.
Or you can do it manually, too.
A sorted bibliography is easier to read and better to maintain with
version control software.

- Be parsimonious! Some fields are not handled by Doxygen.
- Include the abstract whenever possible.
- Don’t use `journaltitle` and `date`. They are not recognized by Doxygen. Use `year`, `month`, etc. instead of `date`.
- Use `{...}` brackets instead of `""`.
- Have equal signs (`=`) line up vertically (for prettiness).
- BibTeX identifiers (inspired by the BibTeX export of [Google Scholar](https://scholar.google.com)):
`<author><year><firstword>` (all lowercase and without delimiter)
    - author: Family name of first author as it would be cited (including van/von/…)
    - year: Publication year.
    - firstword: First word of the title excluding ‘the’, ‘a’, ‘an’, ‘of’, ‘is’, ‘are’, ‘were’, and the like. Hyphens, dashes, apostrophes, and slashes within the first (compound) word are simply omitted.
    - If the above produces non-unique IDs, use the second word, or (if even that fails) the third.

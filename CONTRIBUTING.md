Contributing to the Megafauna Model
===================================

Version Control
---------------

### Branches
- The `master` branch is reserved for stable releases, tagged with the version numbers.
- This repository follows Vincent Driessen’s [Successful Git Branching Model](https://nvie.com/posts/a-successful-git-branching-model/).
- If you are new to Git branching, check out this tutorial: [Learn Git Branching](https://learngitbranching.js.org/)

### Release Versioning
- This project follows [Semantic Versioning](https://semver.org/spec/v2.0.0.html).
- Each merge into the `master` branch is a release and should have a Git tag.
    + The tag’s name is just the exact version, e.g. “0.1.2”.
    + The tag’s decription should summarize the introduced changes.
- Before merging into `master`:
    + Set the new version in `CMakeLists.txt`.
    + List your changes in `CHANGELOG.md`, following the formatting guidelines there.
- If you cite this code in a journal publication, consider getting a DOI for the specific model version you used.

### Commit Messages
Follow Chris Beams’ guide for crafting your Git commit messages: [How to Write a Git Commit Message](https://chris.beams.io/posts/git-commit/)

> 1. Separate subject from body with a blank line
> 2. Limit the subject line to 50 characters
> 3. Capitalize the subject line
> 4. Do not end the subject line with a period
> 5. Use the imperative mood in the subject line
> 6. Wrap the body at 72 characters
> 7. Use the body to explain what and why vs. how

### Continuous Integration (CI)
<!--TODO-->

Coding Guidelines
-----------------

### Repository Structure
This project follows the [Pitchfork Layout](https://github.com/vector-of-bool/pitchfork) for C++ projects.
Here is a summary of the relevant parts:

- **Namespace Folders:** The `src/` directory has subfolders reflecting the namespaces of the contained components.

- **Separate Header Placement:** Header (`*.h`) and source (`*.cpp`) files are kept together in `src/` if they are _private_ (not part of the library interface). _Public_ headers are placed in `include/` while their corresponding source files remain in `src/`.

- **Coherence:** A header and corresponding source file (= _physical component_)contain code for _logical component._ If in doubt, rather air on the side of granularity and create several individual components.

- **Merged Test Placement:** Any logical/physical unit has its unit test in a file in the same folder with the same file name, but with the suffix `.test.cpp`.

### Code Format
The C++ code layout follows the [Google C++ Style Guide](https://google.github.io/styleguide/cppguide.html).
No worries, you don’t need to read everything.
Just see that your IDE or text editor auto-formats your code using [Clang](http://clang.llvm.org/).
The default Google layout is exported in the [.clang-format](.clang-format) file, ready to be read by `clang`.

This repository has a [.editorconfig](.editorconfig) file to define indentation rule for different file types.
Please install the plugin for your text editor if available: [editorconfig.org/](https://editorconfig.org/)

#### Naming

- Classes are named in CamelCase with upper-case first letter, e.g. `MyExampleClass`.
    + Enum types are like classes.
- Functions are imperative verbs with underscores, e.g. `create_new_herbivores()`.
- Constants (outside of function bodies) are all-uppercase with underscores, e.g. `MY_GLOBAL_CONSTANT`.
    + C-style enum elements are similar to constants, but have additionally a prefix with the initials of the type name. For instance the elements in the enum `ForageType` will all start with `FT_`, like `FT_GRASS`.
    + C++11-style enum class elements don’t have global scope and thus don’t require a prefix. Since the shouting tone of all-uppercase names is distracting, just use CamelCase for the enum members, e.g. `OutputInterval::Annual`.
- Namespaces are short and lower-case with first letter capitalized, e.g. `Fauna`.

#### Alphabetical Order
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

In the corresponding source file, _all_ function definitions (both private & public) are in alphabetical order, except for the constructors and destructor, which come first.

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

#### Header for Each File
Begin each `.h` or `.cpp` file with a doxygen header containing a brief description.
The description will appear in the file list of the generated doxygen documentation.
Ususally the brief description will be the same for a `.h` and its `.cpp` file.
```cpp
/// \file
/// \brief Management classes of herbivore populations.
```

<!--TODO: Is this file header enough? Author? Date? License? -->

### Unit Tests
Make sure to write a unit test for every logical component.
If you create a `.cpp` file, there should most likely also be a corresponding `.test.cpp` file that checks the public functions of the class or classes.

Please read the Doxygen page about the testing framework.

### Doxygen

#### Bibliography
[BibTeX](www.bibtex.org) is used for bibliographic references: [docs/bibliography.bib](docs/bibliography.bib).
The Doxygen command `\cite` is used for that.
This makes browsing the Doxygen documentation easier.

In general you should not need to put any references to scientific publications in _comments_ in the source code.
Better you explain everything in a narrative form in the Doxygen _documentation_ and use the `\cite` command for that.
If you do cite in source code comments, make sure that the reference is uniquely identifiable in `bibliography.bib`.

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
    + author: Family name of first author as it would be cited (including van/von/…)
    + year: Publication year.
    + firstword: First word of the title excluding ‘the’, ‘a’, ‘an’, ‘of’, ‘is’, ‘are’, ‘were’, and the like. Hyphens, dashes, apostrophes, and slashes within the first (compound) word are simply omitted.
    + If the above produces non-unique IDs, use the second word, or (if even that fails) the third.

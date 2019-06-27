Contributing to the Megafauna Model
===================================

Version Control
---------------

### Branches

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

Coding Guidelines
-----------------

### Code Format

### Object-Oriented Design

### Unit Tests

### Doxygen

#### Bibliography
[BibTeX](www.bibtex.org) is used for bibliographic references (in addition
to a list of references in comments at the bottom of each source file).
The Doxygen command `cite` is used for that.
This makes browsing the Doxygen documentation easier.

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

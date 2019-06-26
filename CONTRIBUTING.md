Contributing to the Megafauna Model
===================================

Branches
--------

Versioning
----------

Commit Messages
---------------

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

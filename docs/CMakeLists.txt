cmake_minimum_required (VERSION 3.10)

find_package (LATEX COMPONENTS BIBTEX)
find_package (Java COMPONENTS Runtime)
find_package (Doxygen OPTIONAL_COMPONENTS dot)

if (DOXYGEN_FOUND)

  set (DOXYGEN_CITE_BIB_FILES "bibliography.bib")
  set (DOXYGEN_DISTRIBUTE_GROUP_DOC YES)
  set (DOXYGEN_EXAMPLE_PATH "../examples")
  set (DOXYGEN_EXTRACT_PRIVATE YES)
  set (DOXYGEN_FORMULA_FONTSIZE 13)
  set (DOXYGEN_IMAGE_PATH "images")
  set (DOXYGEN_LATEX_BIB_STYLE "ieeetr")
  set (DOXYGEN_PLANTUML_INCLUDE_PATH "./")
  set (DOXYGEN_PLANTUML_JAR_PATH "plantuml/plantuml.jar")
  set (DOXYGEN_USE_MDFILE_AS_MAINPAGE "../README.md")

  if (NOT Java_Runtime_FOUND)
    message (
      WARNING "No Java Runtime Environment found. UML diagrams will not be
      rendered."
      )
  endif()

  if (NOT LATEX_BIBTEX_FOUND)
    message (
      WARNING "BiTeX is not installed. The bibliography citations will not be
      rendered correctly."
      )
  endif()

  if (NOT LATEX_FOUND)
    set (DOXYGEN_USE_MATHJAX YES)
    message (
      WARNING "LaTeX is not installed on this system. MathJax will be used to
      render formulas, but it will need an active Internet connection to
      display the formulas."
      )
  endif ()

  doxygen_add_docs (megafauna_docs
    ../CONTRIBUTING.md
    ../README.md
    ../include/
    ../src/
    design.md
    diagrams.iuml
    group.h
    model.md
    objectorientation.md
    quickstart.md
    tests.md
    tutor.md
    ALL
    COMMENT "Generating doxygen documentation..."
    )
else (DOXYGEN_FOUND)
  message (
    WARNING "Doxygen does not seem to be installed. No documentation will be
    generated."
  )
endif (DOXYGEN_FOUND)

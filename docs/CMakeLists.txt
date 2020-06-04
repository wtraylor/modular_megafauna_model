cmake_minimum_required (VERSION 3.10)
set (REQUIRED_DOXYGEN_VERSION 1.8.16)

find_package (LATEX COMPONENTS BIBTEX)
find_package (Java COMPONENTS Runtime)
find_package (Doxygen OPTIONAL_COMPONENTS dot)

if (DOXYGEN_FOUND)

  # For CI (Continuous Integration) this option should be turned on so
  # that the doxygen command fails if there is a warning.
  if (STOP_DOXYGEN_ON_WARNING)
    set (DOXYGEN_WARN_AS_ERROR YES)
  else()
    set (DOXYGEN_WARN_AS_ERROR NO)
  endif()

  set (DOXYGEN_CITE_BIB_FILES "bibliography.bib")
  set (DOXYGEN_DISTRIBUTE_GROUP_DOC YES)
  set (DOXYGEN_EXAMPLE_PATH "../examples")
  set (DOXYGEN_EXCLUDE_PATTERNS "*.test.cpp")
  set (DOXYGEN_EXTRACT_ALL YES)
  set (DOXYGEN_EXTRACT_PRIVATE YES)
  set (DOXYGEN_FORMULA_FONTSIZE 13)
  set (DOXYGEN_GENERATE_TREEVIEW YES)
  set (DOXYGEN_IMAGE_PATH "images")
  set (DOXYGEN_LATEX_BIB_STYLE "ieeetr")
  set (DOXYGEN_PLANTUML_INCLUDE_PATH "./")
  set (DOXYGEN_PLANTUML_JAR_PATH "plantuml/plantuml.jar")
  set (DOXYGEN_USE_MDFILE_AS_MAINPAGE "mainpage.md")

  if (DOXYGEN_VERSION VERSION_LESS ${REQUIRED_DOXYGEN_VERSION})
    if (STOP_DOXYGEN_ON_WARNING)
      message (
        FATAL_ERROR "Your Doxygen version (${DOXYGEN_VERSION}) is too old. In
        order to compile reliably, use at least version
        ${REQUIRED_DOXYGEN_VERSION}."
        )
    else()
      message (
        WARNING "You are using an older version of Doxygen
        (${DOXYGEN_VERSION}). Perhaps errors will occur. This documentation was
        tested with Doxygen version ${REQUIRED_DOXYGEN_VERSION}."
        )
    endif()
  endif()

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
    ../include/
    ../src/
    ../tools/demo_simulator/
    demo_simulator.md
    design.md
    diagrams.iuml
    mainpage.md
    model.md
    objectorientation.md
    quickstart.md
    separate_docs.h
    tutor.md
    ALL
    COMMENT "Generating doxygen documentation..."
    )

  # Print message after "megafauna_docs" is completed.
  add_custom_command (TARGET megafauna_docs POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E cmake_echo_color --cyan "The documentation is ready. Open docs/html/index.html in your browser."
    )
else (DOXYGEN_FOUND)
  message (
    WARNING "Doxygen does not seem to be installed. No documentation will be
    generated."
  )
endif (DOXYGEN_FOUND)

# SPDX-FileCopyrightText: 2021 Wolfgang Traylor <wolfgang.traylor@senckenberg.de>
#
# SPDX-License-Identifier: CC0-1.0

# Configuration file for the Sphinx documentation builder.
#
# This file only contains a selection of the most common options. For a full
# list see the documentation:
# https://www.sphinx-doc.org/en/master/usage/configuration.html

import subprocess

# Go to the repository’s root folder and run Cmake, which then call `doxygen`.
# The `doxygen` executable is taken from Conda, which has the latest version.
subprocess.call(
    '''
    conda install --yes cmake && \
    conda install --yes --channel conda-forge doxygen && \
    PATH=$HOME/.conda/bin:$PATH && \
    cd ../.. && \
    cmake -DBUILD_DOC=ON . && \
    make megafauna_docs
    ''',
    shell=True
)
# CMake + Make have generated the Doxygen documentation in docs/html.
html_extra_path = ['../html']

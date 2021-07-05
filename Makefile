# SPDX-FileCopyrightText: 2021 Wolfgang Traylor <wolfgang.traylor@senckenberg.de>
#
# SPDX-License-Identifier: CC0-1.0

paper.pdf : paper.md paper.bib
	docker run --rm \
		--volume $(PWD):/data \
		--user $(shell id -u):$(shell id -g) \
		--env JOURNAL=joss \
		openjournals/paperdraft

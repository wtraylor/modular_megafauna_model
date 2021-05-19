paper.pdf : paper.md paper.bib
	docker run --rm \
		--volume $(PWD):/data \
		--user $(shell id -u):$(shell id -g) \
		--env JOURNAL=joss \
		openjournals/paperdraft

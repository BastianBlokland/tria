default: build

# --------------------------------------------------------------------------------------------------
# Convenient development utlitities.
# --------------------------------------------------------------------------------------------------

.PHONY: configure.debug
configure.debug:
	./scripts/configure.sh --type Debug

.PHONY: configure.release
configure.release:
	./scripts/configure.sh --type Release

.PHONY: build
build:
	./scripts/build.sh

.PHONY: clean
clean:
	rm -rf build

.SILENT:

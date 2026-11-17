# Build dispatcher for flash_test (on-target app) and loader (.stldr).
# From /work (in the container or with arm-none-eabi-gcc + CMake on PATH):
#   make              # both
#   make flash_test
#   make loader
#   make clean
# From the host with Docker:
#   make docker-build / docker-build-loader / docker-shell

.PHONY: all flash_test loader clean clean-flash_test clean-loader \
        image docker-build docker-build-loader docker-shell docker-clean

TOOLCHAIN := $(CURDIR)/cmake/arm-none-eabi.cmake

FLASH_TEST_DIR := flash_test
FLASH_TEST_BUILD := $(FLASH_TEST_DIR)/build

LOADER_DIR := loader
LOADER_BUILD := $(LOADER_DIR)/build

all: flash_test loader

flash_test:
	cmake -S $(FLASH_TEST_DIR) -B $(FLASH_TEST_BUILD) \
		-DCMAKE_TOOLCHAIN_FILE=$(TOOLCHAIN)
	cmake --build $(FLASH_TEST_BUILD)

loader:
	cmake -S $(LOADER_DIR) -B $(LOADER_BUILD) \
		-DCMAKE_TOOLCHAIN_FILE=$(TOOLCHAIN)
	cmake --build $(LOADER_BUILD)

clean: clean-flash_test clean-loader

clean-flash_test:
	rm -rf $(FLASH_TEST_BUILD)

clean-loader:
	rm -rf $(LOADER_BUILD)

image:
	docker compose build

docker-build: image
	docker compose run --rm --user "$(shell id -u):$(shell id -g)" build

docker-build-loader: image
	docker compose run --rm --user "$(shell id -u):$(shell id -g)" build-loader

docker-shell: image
	docker compose run --rm --user "$(shell id -u):$(shell id -g)" shell

docker-clean: image
	docker compose run --rm --user "$(shell id -u):$(shell id -g)" build make clean

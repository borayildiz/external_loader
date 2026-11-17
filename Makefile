# CMake build for the flash test app.
# Inside the container (or with arm-none-eabi-gcc on PATH): make / make clean
# From the host with Docker: make docker-build

.PHONY: build clean docker-build docker-shell docker-clean image

BUILD_DIR := test/build
TOOLCHAIN := $(CURDIR)/cmake/arm-none-eabi.cmake

build:
	cmake -S test -B $(BUILD_DIR) -DCMAKE_TOOLCHAIN_FILE=$(TOOLCHAIN)
	cmake --build $(BUILD_DIR)

clean:
	rm -rf $(BUILD_DIR)

image:
	docker compose build

docker-build: image
	docker compose run --rm --user "$(shell id -u):$(shell id -g)" build

docker-shell: image
	docker compose run --rm --user "$(shell id -u):$(shell id -g)" shell

docker-clean: image
	docker compose run --rm --user "$(shell id -u):$(shell id -g)" build make clean

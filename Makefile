# Convenience targets that build inside Docker.

.PHONY: docker-build docker-shell docker-clean image

image:
	docker compose build

docker-build: image
	docker compose run --rm --user "$(shell id -u):$(shell id -g)" build

docker-shell: image
	docker compose run --rm --user "$(shell id -u):$(shell id -g)" shell

docker-clean:
	docker compose run --rm --user "$(shell id -u):$(shell id -g)" build make -C test clean

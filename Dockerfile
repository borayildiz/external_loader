# ARM embedded build environment for the IS25LP128F flash driver test app.
FROM ubuntu:24.04

ENV DEBIAN_FRONTEND=noninteractive

RUN apt-get update && apt-get install -y --no-install-recommends \
        gcc-arm-none-eabi \
        binutils-arm-none-eabi \
        libnewlib-arm-none-eabi \
        make \
        git \
        ca-certificates \
    && rm -rf /var/lib/apt/lists/*

# Avoid root-owned build artifacts when the host UID differs; override with
# docker compose --user "$(id -u):$(id -g)" if needed.
WORKDIR /work

CMD ["bash"]

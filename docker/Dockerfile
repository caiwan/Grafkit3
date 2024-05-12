ARG BASE_IMAGE=debian:bookworm-slim
FROM ${BASE_IMAGE} AS builder

USER root

# https://github.com/edowson/docker-nvidia-vulkan/tree/master

ENV DEBIAN_FRONTEND=noninteractive
RUN apt update \
    && apt upgrade -y \
    && apt install -y \
        software-properties-common \
        ca-certificates \
        apt-transport-https \
        gnupg \
        dirmngr \
        build-essential \
        git \
        curl \
        ninja-build \
        clang \
        pkg-config \
        python3 \
        python3-venv \
        python3-distutils \
        python3-jsonschema \
        python-is-python3 \
        bison \
        ocaml-core \
        pkg-config \
        sudo \
        xz-utils

RUN apt install -y \
        libxml2-dev \
        libx11-dev \
        libx11-xcb-dev \
        liblz4-dev \
        libzstd-dev \
        libxcb-dri3-dev \
        libxcb-keysyms1-dev \
        libpng-dev \
        libpciaccess0 \
        libxcb-present0 \
        libxcb-dri3-0 \
        libglm-dev cmake \
        libwayland-dev \
        libxrandr-dev \
        libxcb-randr0-dev \
        libxcb-ewmh-dev

RUN apt install -y \
        qtbase5-dev \
        qtchooser \
        qt5-qmake \
        qtbase5-dev-tools

RUN curl -L -o /tmp/cmake.tar.gz https://github.com/Kitware/CMake/releases/download/v3.28.2/cmake-3.28.2-linux-x86_64.tar.gz  \
    && tar -xvzf /tmp/cmake.tar.gz -C /usr/local --strip-components=1

ENV VULKAN_SDK_VERSION=1.3.275.0
ENV VULKAN_SDK_URL=https://sdk.lunarg.com/sdk/download/${VULKAN_SDK_VERSION}/linux/vulkansdk-linux-x86_64-${VULKAN_SDK_VERSION}.tar.xz
ENV VULKAN_SDK_DIR=/opt/vulkan/${VULKAN_SDK_VERSION}
RUN mkdir -p ${VULKAN_SDK_DIR} \
    && curl -L -o /tmp/vulkan-sdk.tar.gz ${VULKAN_SDK_URL}

RUN tar -xvf /tmp/vulkan-sdk.tar.gz -C /var/tmp
RUN /var/tmp/${VULKAN_SDK_VERSION}/vulkansdk --skip-deps --maxjobs
RUN cp -r /var/tmp/${VULKAN_SDK_VERSION}/x86_64/ ${VULKAN_SDK_DIR} \
    && cp /var/tmp/${VULKAN_SDK_VERSION}/setup-env.sh ${VULKAN_SDK_DIR}

RUN apt-get clean \
    && rm -rf /var/lib/apt/lists/* \
    && rm -rf /tmp/*

# TODO: Install nvidia drivers
ENV NVIDIA_VISIBLE_DEVICES=all
ENV NVIDIA_DRIVER_CAPABILITIES=graphics,utility,compute

ENTRYPOINT [ "bash" "-c" ]

# MARK: Develpper image
FROM builder AS dev

RUN apt update && \
    apt install -y \
        zsh \
        neovim \
        valgrind \
        cppcheck \
        lldb \
        llvm \
        gdb

RUN apt-get clean \
    && rm -rf /var/lib/apt/lists/* \
    && rm -rf /tmp/*

RUN useradd -m -u 1000 -s /bin/bash developer
RUN echo 'developer:developer' | chpasswd

USER developer

ENV CHSH='no'
ENV RUNZSH='no'
RUN sh -c "$(curl -fsSL https://raw.githubusercontent.com/ohmyzsh/ohmyzsh/master/tools/install.sh)"

USER root
RUN sed -i 's|/home/developer:/bin/bash|/home/developer:$(which zsh)|' /etc/passwd

USER developer
WORKDIR /home/developer
CMD [ "sleep infinity" ]

# MARK: App image
FROM builder AS app

# TOOD: Build app here

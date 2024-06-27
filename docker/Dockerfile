ARG BASE_IMAGE=ubuntu:20.04
FROM ${BASE_IMAGE} as builder

USER root

# https://github.com/edowson/docker-nvidia-vulkan/tree/master

ENV DEBIAN_FRONTEND=noninteractive
RUN apt update \
    && apt upgrade \
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
        libxcb-ewmh-dev \
        wayland-protocols \
        vulkan-utils

RUN apt install -y \
        qtbase5-dev \
        qtchooser \
        qt5-qmake \
        qtbase5-dev-tools

RUN sudo apt-key adv --keyserver hkp://keyserver.ubuntu.com:80 --recv-keys 3FA7E0328081BFF6A14DA29AA6A19B38D3D831EF \
    && echo "deb https://download.mono-project.com/repo/ubuntu stable-focal main" | sudo tee /etc/apt/sources.list.d/mono-official-stable.list \
    && apt update \
    && apt install -y mono-devel

RUN curl -L -o /tmp/cmake.tar.gz https://github.com/Kitware/CMake/releases/download/v3.28.2/cmake-3.28.2-linux-x86_64.tar.gz  \
    && tar -xvzf /tmp/cmake.tar.gz -C /usr/local --strip-components=1

RUN export VULKAN_VERSION=1.3.275.0 \
    && export VULKAN_SDK_VERSION=1.3.275.0 \
    && export VULKAN_SDK_URL=https://sdk.lunarg.com/sdk/download/${VULKAN_VERSION}/linux/vulkansdk-linux-x86_64-${VULKAN_SDK_VERSION}.tar.xz \
    && export VULKAN_SDK_DIR=/opt/vulkan/${VULKAN_SDK_VERSION} \
    && mkdir -p ${VULKAN_SDK_DIR} \
    && curl -L -o /tmp/vulkan-sdk.tar.gz ${VULKAN_SDK_URL} \
    && tar -xvf /tmp/vulkan-sdk.tar.gz -C /tmp \
    && /tmp/${VULKAN_SDK_VERSION}/vulkansdk --skip-deps -j 5 \
    && cp /tmp/${VULKAN_SDK_VERSION}/setup-env.sh /etc/profile.d/vulkan.sh

RUN apt-get clean \
    && rm -rf /var/lib/apt/lists/* \
    && rm -rf /tmp/*

# TODO: Install nvidia drivers
ENV NVIDIA_VISIBLE_DEVICES=all
ENV NVIDIA_DRIVER_CAPABILITIES=graphics,utility,compute


ENTRYPOINT [ "bash" "-c" ]

FROM builder as dev

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

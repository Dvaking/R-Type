#!/usr/bin/env bash

# Install SFML
. /etc/os-release
case "${ID}" in
ubuntu)
    export DEBIAN_FRONTEND=noninteractive
    # X11
    sudo apt-get update
    sudo apt-get install -y \
        git \
        libxrandr-dev \
        libxcursor-dev \
        libudev-dev \
        libfreetype-dev \
        libopenal-dev \
        libflac-dev \
        libvorbis-dev \
        libgl1-mesa-dev \
        libegl1-mesa-dev \
        libsfml-dev \
        coreutils
    ;;
fedora)
    # X11
    sudo dnf install -y \
        git \
        make \
        libX11-devel \
        libXrandr-devel \
        libXi-devel \
        libXcursor-devel \
        libXinerama-devel \
        libXrender-devel \
        libgudev-devel \
        glew-devel glm-devel freetype-devel \
        openal-soft-devel \
        flac-devel \
        libvorbis-devel \
        SFML-devel \
        coreutils
    ;;
nixos)
    # X11
    if [ "$(env | grep X11 -c)" -lt 3 ]; then
        nix develop --command bash -c "./compile.sh"
        exit 0
    fi
    ;;
arch)
    # X11
    sudo pacman -Syy libxrandr \
        libxcursor \
        libudev \
        freetype2 \
        openal \
        flac \
        libvorbis mesa
    ;;
*)
    echo "Unsupported OS"
    exit 1
    ;;
esac

FROM ubuntu:24.04

# Avoid interactive prompts during package installation
ENV DEBIAN_FRONTEND=noninteractive
ENV TZ=UTC
ENV LANG=en_US.UTF-8
ENV LANGUAGE=en_US:en
ENV LC_ALL=en_US.UTF-8

# Install dependencies and set up locale
RUN apt-get update && \
    apt-get install -y --no-install-recommends \
    android-sdk-libsparse-utils \
    autoconf \
    autoconf-archive \
    automake \
    bc \
    bison \
    build-essential \
    ca-certificates \
    cmake \
    cmake-data \
    cpio \
    curl \
    device-tree-compiler \
    flex \
    g++-multilib \
    gawk \
    git \
    kmod \
    lib32z1 \
    libbison-dev \
    libc6-dev \
    libgl1-mesa-dev \
    libmpc3 \
    libncurses-dev \
    libncurses6 \
    libssl-dev \
    libstdc++6 \
    libtool \
    locales \
    lz4 \
    lzop \
    m4 \
    make \
    mingw-w64 \
    mtd-utils \
    pkg-config \
    python3 \
    python3-pip \
    squashfs-tools \
    sudo \
    texinfo \
    tofrodos \
    u-boot-tools \
    vim \
    wget \
    git-lfs \
    avahi-daemon \
    libnss-mdns \
    zlib1g-dev \
    libpcre2-dev && \
    locale-gen en_US.UTF-8 && \
    apt-get clean && \
    rm -rf /var/lib/apt/lists/*

# Create a non-root user and copy workspace content
RUN useradd -m -s /bin/bash developer && \
    echo "developer ALL=(ALL) NOPASSWD:ALL" >> /etc/sudoers

# Set up environment variables for toolchain
ENV PATH="/opt/ivot/aarch64-ca53-linux-gnueabihf-10.4.0/bin:/opt/ivot/gcc-linaro-6.4.1-2018.05-x86_64_arm-eabi/bin:${PATH}"

# Switch to non-root user and set working directory
USER developer
WORKDIR /workspace

# Default command
CMD ["/bin/bash"]

FROM ubuntu:20.04

ARG UID 
ARG USER
ARG TZ_ARG=UTC

# Necessary for tzdata
ENV DEBIAN_FRONTEND=noninteractive
ENV TZ=${TZ_ARG}

# Add i386 support for Pin
RUN dpkg --add-architecture i386
RUN apt-get update && apt-get install -y \
    python \
    screen \
    tmux \
    binutils \
    libc6:i386 \
    libncurses5:i386 \
    libstdc++6:i386 \
 && rm -rf /var/lib/apt/lists/*

# For building Sniper
RUN apt-get update && apt-get install -y \
    automake \
    build-essential \
    cmake \
    curl \
    wget \
    libboost-dev \
    libsqlite3-dev \
    zlib1g-dev \
    libbz2-dev \
 && rm -rf /var/lib/apt/lists/*

# For building RISC-V Tools
RUN apt-get update && apt-get install -y \
    autoconf \
    automake \
    autotools-dev \
    bc \
    bison \
    curl \
    device-tree-compiler \
    flex \
    gawk \
    gperf \
    libexpat-dev \
    libgmp-dev \
    libmpc-dev \
    libmpfr-dev \
    libtool \
    libusb-1.0-0-dev \
    patchutils \
    pkg-config \
    texinfo \
    zlib1g-dev \
 && rm -rf /var/lib/apt/lists/*

# Helper utilities
RUN apt-get update && apt-get install -y \
    gdb \
    gfortran \
    git \
    g++-9 \
    vim \
    nano \
 && rm -rf /var/lib/apt/lists/*

# Upgrade OS
RUN apt-get upgrade \
 && rm -rf /var/lib/apt/lists/*

# Install sudo and SSH server
RUN apt-get update && apt-get install -y \
    sudo \
    openssh-server \
 && rm -rf /var/lib/apt/lists/*

# Create user and setup permissions on /etc/sudoers
RUN useradd -m -s /bin/bash -N -u $UID $USER && \
    # usermod -aG sudo $USER && \ 
    echo "${USER} ALL=(ALL) NOPASSWD: ALL" > /etc/sudoers && \
    chmod 0440 /etc/sudoers && \
    chmod g+w /etc/passwd
# Copy the id_ed25519 key to container using Docker secrets
RUN mkdir -p /home/$USER/.ssh && ln -s /run/secrets/user_ssh_key /home/$USER/.ssh/id_ed25519
RUN chown -R $USER:sudo /home/$USER/.ssh
RUN echo "Host *.trabe.io\n\tStrictHostKeyChecking no\n" >> /home/$USER/.ssh/config

# Allow SSH access
RUN mkdir /var/run/sshd
# Expose the SSH port
EXPOSE 22

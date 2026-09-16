ARG OS=ubuntu
ARG RELEASE=22.04
FROM $OS:$RELEASE
ENV DEBIAN_FRONTEND=noninteractive
RUN apt update && apt dist-upgrade -y && apt install -y \
    bash \
    build-essential \
    curl \
    file \
    gcc \
    git \
    libfftw3-dev \
    libfuse2 \
    libgl1-mesa-dev \
    libpulse-dev \
    libqt5serialport5-dev \
    libudev-dev \
    libusb-1.0-0-dev \
    lsb-release \
    patchelf \
    qtbase5-dev \
    qtchooser \
    qttools5-dev-tools \
    && rm -rf /var/lib/apt/lists/* \
    && rm -rf /etc/localtime

VOLUME ["/Lightpack", "/build.sh", "/etc/localtime"]

ENTRYPOINT /build.sh

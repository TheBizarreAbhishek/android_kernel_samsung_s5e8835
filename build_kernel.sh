#!/bin/bash

KERNEL_ROOT=$(pwd)

# Setup toolchain directories
mkdir -p "${KERNEL_ROOT}/build" "${HOME}/toolchains"

# Init clang-r450784e
if [ ! -d "${HOME}/toolchains/clang-r450784e" ]; then
    echo -e "\n[INFO] Downloading clang-r450784e..."
    mkdir -p "${HOME}/toolchains/clang-r450784e" && cd "${HOME}/toolchains/clang-r450784e"
    curl -LO "https://android.googlesource.com/platform//prebuilts/clang/host/linux-x86/+archive/722c840a8e4d58b5ebdab62ce78eacdafd301208/clang-r450784e.tar.gz"
    tar -xf clang-r450784e.tar.gz && rm clang-r450784e.tar.gz
    cd "${KERNEL_ROOT}"
fi

# Init arm gnu toolchain
if [ ! -d "${HOME}/toolchains/gcc" ]; then
    echo -e "\n[INFO] Downloading ARM GNU Toolchain..."
    mkdir -p "${HOME}/toolchains/gcc" && cd "${HOME}/toolchains/gcc"
    curl -LO "https://developer.arm.com/-/media/Files/downloads/gnu/14.2.rel1/binrel/arm-gnu-toolchain-14.2.rel1-x86_64-aarch64-none-linux-gnu.tar.xz"
    tar -xf arm-gnu-toolchain-14.2.rel1-x86_64-aarch64-none-linux-gnu.tar.xz
    cd "${KERNEL_ROOT}"
fi

# Export toolchain paths
export PATH="${HOME}/toolchains/clang-r450784e/bin:${PATH}"
export LD_LIBRARY_PATH="${HOME}/toolchains/clang-r450784e/lib64:${LD_LIBRARY_PATH}"
export BUILD_CROSS_COMPILE="${HOME}/toolchains/gcc/arm-gnu-toolchain-14.2.rel1-x86_64-aarch64-none-linux-gnu/bin/aarch64-none-linux-gnu-"
export BUILD_CC="${HOME}/toolchains/clang-r450784e/bin/clang"

# Export required Samsung Exynos variables
export ARCH=arm64
export TARGET_SOC=s5e8835
export PLATFORM_VERSION=13
export ANDROID_MAJOR_VERSION=t
export LLVM=1
export DTC_FLAGS="-@"

# Clean up
echo -e "\n[INFO] Running make clean & mrproper..."
make clean && make mrproper

# Load defconfig
echo -e "\n[INFO] Loading defconfig..."
make s5e8835-m35xxx_defconfig

# Start build
echo -e "\n[INFO] Starting kernel build..."
make -j$(nproc) Image dtbs

# Copy Image
cp "${KERNEL_ROOT}/arch/arm64/boot/Image" "${KERNEL_ROOT}/build/"

# Generate DTBO image
DTBO_DIR="${KERNEL_ROOT}/arch/arm64/boot/dts/samsung/m35x"
DTBO_FILES=$(find "$DTBO_DIR" -name "*.dtbo")

if [ -n "$DTBO_FILES" ]; then
    echo -e "\n[INFO] Creating DTBO image..."
    "${KERNEL_ROOT}/tools/mkdtimg" create "${KERNEL_ROOT}/build/dtbo.img" --page_size=4096 $DTBO_FILES
else
    echo -e "\n[INFO] No .dtbo files found, skipping DTBO creation."
fi

echo -e "\n[INFO] Kernel build completed successfully."

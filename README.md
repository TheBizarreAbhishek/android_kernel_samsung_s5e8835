# 🌀 Bizarre Kernel | Galaxy M35 & A35 (Exynos 1380)

[![Build Status](https://img.shields.io/github/actions/workflow/status/TheBizarreAbhishek/android_kernel_samsung_s5e8835/build-kernels.yml?branch=android13-5.15&style=for-the-badge&logo=github)](https://github.com/TheBizarreAbhishek/android_kernel_samsung_s5e8835/actions)
[![Kernel Version](https://img.shields.io/badge/GKI_Kernel-5.15.216-blue?style=for-the-badge&logo=linux)](https://android.googlesource.com/kernel/common/)
[![Architecture](https://img.shields.io/badge/Arch-ARM64--v8a-orange?style=for-the-badge&logo=arm)](https://developer.arm.com/)
[![Android Version](https://img.shields.io/badge/Android-13%20--%2016-green?style=for-the-badge&logo=android)](https://source.android.com/)

A clean, high-performance, and battery-optimized custom kernel designed for Samsung Exynos 1380 (`s5e8835`) devices. Built with modern memory management, root stealth, and low-latency gaming in mind.

---

### 📱 Supported Devices
* **Samsung Galaxy M35 5G** (`SM-M356B`, `SM-M356E`)
* **Samsung Galaxy A35 5G** (`SM-A356B`, `SM-A356E`, `SM-A356U`, `SM-A3560`)

---

## ✨ Features Overview

### ⚡ Performance & Battery
* **Default Schedutil Governor:** Energy-Aware Scheduling on all CPU clusters for buttery-smooth 120Hz responsiveness.
* **Power-Efficient Workqueues:** Automatically routes background kernel tasks to energy-saving LITTLE cores to maximize Screen-On-Time (SOT).
* **Stripped Samsung Debug Overhead:** Disabled Samsung `SEC_DEBUG`, ACPM logging, and IRQ traces to eliminate boot and UI micro-stutters.
* **Multiple Scaling Governors:** Choice of `schedutil` (default), `ondemand`, `conservative`, `powersave`, and `performance`.

### 🧠 Memory Management
* **Dual-Stage ZRAM (LZ4 + ZSTD):** Fast `lz4` for zero-latency foreground use + automatic `zstd` recompression on screen-off (achieves 4.3x+ compression, freeing ~1.8GB RAM).
* **Multi-Gen LRU (MGLRU):** Enabled by default to keep frequently used apps alive and eliminate reload delays.
* **Kernel Samepage Merging (KSM):** Built-in page deduplication engine (`CONFIG_KSM=y`) that merges duplicate RAM pages across apps to save extra memory.
* **1-Flash TWRP Compatibility:** Built-in monolithic ZRAM architecture that works out-of-the-box even with broken `/data` decryption in recovery.

### 🌐 Networking & Connectivity
* **Google BBRv3 TCP:** Default congestion control for ultra-low gaming ping, zero bufferbloat, and maximum 5G/Wi-Fi speeds.
* **Carrier Hotspot & Tethering Bypass:** Built-in TTL & Hop Limit targets (`CONFIG_NETFILTER_XT_TARGET_HL=y`, `TTL=y`) to bypass carrier tethering restrictions.
* **In-Kernel WireGuard:** Native, battery-efficient WireGuard VPN engine compiled directly into the kernel.

### 🛡️ Root, Stealth & Security
* **KernelSU / KernelSU-Next:** Next-generation kernel-level rooting support.
* **SUSFS Integration:** Complete root & mount stealth with path spoofing, mount cloaking, and open redirect protection.
* **Knox Security Stripped:** Completely removed Samsung Knox subsystems (`DEFEX`, `PROCA`, `FIVE`, `DSMS`, `KPERFMON`) to allow unrestricted root operations.
* **Hidden Configuration:** Disabled `/proc/config.gz` export to prevent anti-root detectors from sniffing kernel flags.
* **SELinux Flexibility:** Available in both Enforcing (default) and Permissive builds.

### 🎮 Gaming, Virtualization & Containers
* **NTSYNC Driver:** High-performance Windows NT synchronization primitives for smoother Wine / Proton PC emulation and gaming.
* **Container Support:** Full namespace isolation (`NET`, `PID`, `IPC`, `UTS`, `USER`) for Docker and Droidspace with kABI-safe System V IPC.

---

## 📦 Build Variants

Pre-built AnyKernel3 flashable zips are automatically compiled on GitHub Actions:

| Variant | Root Solution | SUSFS Stealth | SELinux Mode | Recommended For |
| :--- | :---: | :---: | :---: | :--- |
| **`ksu-susfs`** | KernelSU | ✔️ Included | Enforcing | Daily driving with full banking app stealth *(Recommended)* |
| **`ksunext-susfs`** | KernelSU-Next | ✔️ Included | Enforcing | Users wanting the latest KernelSU-Next features with stealth |
| **`ksu`** | KernelSU | ❌ None | Enforcing | Standard KernelSU root without SUSFS patches |
| **`ksunext`** | KernelSU-Next | ❌ None | Enforcing | Standard KernelSU-Next root |
| **`permissive`** | None | ❌ None | Permissive | Testing custom ROMs or GSI ports requiring SELinux Permissive |
| **`clean`** | None | ❌ None | Enforcing | 100% Stock unrooted setup with kernel performance tweaks |

---

## 📲 Installation

1. Download your preferred variant ZIP from the [GitHub Releases](https://github.com/TheBizarreAbhishek/android_kernel_samsung_s5e8835/actions) tab.
2. Reboot your device into custom recovery (**TWRP / OrangeFox**).
3. Flash the `.zip` file (No need to wipe `/data` or cache).
4. Reboot system and enjoy!

---

## 🛠️ Build Specifications
* **Target Architecture:** `ARM64-v8a`
* **SoC:** Samsung Exynos 1380 (`s5e8835`)
* **OS Compatibility:** One UI 6.x / 7.x / 8.x (Android 14 – 16)
* **Compiler:** Google LLVM Clang Toolchain `r450784d`
* **LTO:** Full Link-Time Optimization (`CONFIG_LTO_CLANG=y`)

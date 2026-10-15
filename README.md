# 🌀 Bizarre Kernel | Galaxy M35 & A35 (Exynos 1380)

[![Build Status](https://img.shields.io/github/actions/workflow/status/TheBizarreAbhishek/android_kernel_samsung_s5e8835/build-kernels.yml?branch=android13-5.15&style=for-the-badge&logo=github)](https://github.com/TheBizarreAbhishek/android_kernel_samsung_s5e8835/actions)
[![Kernel Version](https://img.shields.io/badge/GKI_Kernel-5.15.216-blue?style=for-the-badge&logo=linux)](https://android.googlesource.com/kernel/common/)
[![Android Version](https://img.shields.io/badge/Android-16-green?style=for-the-badge&logo=android)](https://source.android.com/)

A highly optimized, customized GKI-compliant kernel for Samsung Exynos 1380 (s5e8835) devices. Designed for power users, containerization enthusiasts, and security researchers.

### 📱 Supported Devices (Exynos 1380 / s5e8835)
* **Samsung Galaxy M35 5G** (SM-M356B, SM-M356E)
* **Samsung Galaxy A35 5G** (SM-A356B, SM-A356E, SM-A356U, SM-A3560)

---

## 🚀 Key Features

### 🛡️ Rooting & Advanced Stealth
* **KernelSU / KSU-Next:** Integrated dynamically at build time for next-generation kernel-level rooting.
* **SUSFS integration:** Fully hidden root status with advanced path/mount spoofing (`SUS_PATH`, `SUS_MOUNT`, `SUS_KSTAT`, `Spoofer`, `Open Redirect`).
* **SELinux Controls:** Available in both Enforcing (default) and Permissive builds.

### 🐳 Container & Virtualization (Docker / Droidspace)
* **Isolated Namespaces:** Full isolation support (`NET_NS`, `PID_NS`, `IPC_NS`, `UTS_NS`, `USER_NS`).
* **kABI Preservation:** Custom GKI System V IPC patch ensures total stability, preventing Kernel ABI mismatch bootloops while allowing Docker and system containers to function.

### ⚡ Performance, Memory & Networking
* **ZRAM Multi-Compressor (Two-Stage Recompression):** Dual-compressor engine powered by fast real-time `lz4` for zero foreground latency during gaming and scrolling, paired with secondary `zstd` recompression for idle and huge pages during screen-off quiescent sleep. Delivers 3.1x+ memory compression ratio with zero lag.
* **Multi-Gen LRU (MGLRU):** Enabled by default (`CONFIG_LRU_GEN_ENABLED=y`) with active page generation tracking for optimal working set estimation and instant app switching.
* **Schedutil CPU Governor:** Removed proprietary Samsung `energy_aware` overrides, ensuring upstream Energy-Aware Scheduling (`schedutil`) is default on all CPU clusters for buttery-smooth responsiveness.
* **BBR TCP Default:** Enabled BBR congestion control as the default TCP algorithm for ultra-low latency, reduced packet loss, and maximum throughput over Wi-Fi and 5G cellular networks.
* **1-Flash Plug-and-Play (Broken TWRP Compatible):** Built-in monolithic ZRAM architecture with kernel-level DLKM intercept (`kernel/module.c`) allows flashing via AnyKernel3 without requiring broken `/data` decryption in TWRP.
* **NTSYNC:** Backported Windows NT synchronization primitives driver helper for high-performance wine/proton gaming.
* **Reduced Overhead:** Disabled Samsung `SEC_DEBUG`, verbose ACPM power management logging, spurious IRQs logging, and block IO stats, reducing CPU overhead and eliminating boot/UI lag.
* **Governor Support:** Enabled `schedutil` (default), `ondemand`, `conservative`, `powersave`, and `performance` CPU scaling governors.

### 🛡️ Samsung Security Bypasses & Stealth
* **Knox Disabled:** Completely stripped proprietary Samsung Knox security subsystems (`DEFEX`, `PROCA`, `FIVE`, `DSMS`, `KPERFMON`) to allow root operations and arbitrary execution.
* **Hidden Configuration:** Disabled `/proc/config.gz` export to defeat root detectors checking kernel configs.

---

## 📦 Build Matrix Variants

We build multiple kernel configurations automatically on GitHub Actions:

| Variant Name | Root Solution | SUSFS Stealth | SELinux Mode |
| :--- | :---: | :---: | :---: |
| `clean` | None | ❌ | Enforcing |
| `permissive` | None | ❌ | Permissive |
| `ksu` | KernelSU | ❌ | Enforcing |
| `ksunext` | KernelSU Next | ❌ | Enforcing |
| `ksu-susfs` | KernelSU | ✔️ | Enforcing |
| `ksunext-susfs` | KernelSU Next | ✔️ | Enforcing |


---

## 🛠️ Toolchain & Specifications

* **Target Architecture:** `ARM64-v8a`
* **SoC Platform:** Samsung Exynos 1380 (s5e8835)
* **Host OS Support:** One UI 8.0 / 8.5 (Android 16)
* **Compiler:** Google LLVM Clang Toolchain `r450784d`
* **LTO Optimizations:** Link-Time Optimization (`CONFIG_LTO_CLANG=y`) enabled for maximum efficiency.

---

## 📲 Installation Instructions

1. **Kernel Installation:**
   * Download the correct variant `.zip` file (e.g. `ksu-susfs`) for your setup from the [Releases](https://github.com/TheBizarreAbhishek/android_kernel_samsung_s5e8835/releases) tab.
   * Flash the ZIP using a custom recovery (**TWRP / OrangeFox**) or a dedicated kernel flasher app (like **Franco Kernel Manager / EX Kernel Manager**).
   * Reboot your device.

---

## 🗺️ Roadmap / TODO

- [ ] **NetHunter Drivers Module** — Distribute high-performance wireless injection drivers (`ath9k_htc`, `rtl8187`, `rt2800usb`) and BadUSB HID injection support as a separate flashable KernelSU/Magisk module (`BizarreKernel-NetHunter-Drivers.zip`). Not included in current releases.

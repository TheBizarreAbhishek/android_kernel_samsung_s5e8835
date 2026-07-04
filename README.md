# 🌀 Bizarre Kernel | Galaxy M35 & A35 (Exynos 1380)

[![Build Status](https://img.shields.io/github/actions/workflow/status/TheBizarreAbhishek/android_kernel_samsung_s5e8835/build-kernels.yml?branch=android13-5.15&style=for-the-badge&logo=github)](https://github.com/TheBizarreAbhishek/android_kernel_samsung_s5e8835/actions)
[![Kernel Version](https://img.shields.io/badge/GKI_Kernel-5.15.208-blue?style=for-the-badge&logo=linux)](https://android.googlesource.com/kernel/common/)
[![Android Version](https://img.shields.io/badge/Android-16-green?style=for-the-badge&logo=android)](https://source.android.com/)

A highly optimized, customized GKI-compliant kernel for Samsung Galaxy M35 & A35 devices powered by the **Exynos 1380 (s5e8835)** SoC. Designed for power users, containerization enthusiasts, and security researchers.

---

## 🚀 Key Features

### 🛡️ Rooting & Advanced Stealth
* **KernelSU / KSU-Next:** Integrated dynamically at build time for next-generation kernel-level rooting.
* **SUSFS integration:** Fully hidden root status with advanced path/mount spoofing (`SUS_PATH`, `SUS_MOUNT`, `SUS_KSTAT`, `Spoofer`, `Open Redirect`).
* **SELinux Controls:** Available in both Enforcing (default) and Permissive builds.

### 🐳 Container & Virtualization (Docker / Droidspace)
* **Isolated Namespaces:** Full isolation support (`NET_NS`, `PID_NS`, `IPC_NS`, `UTS_NS`, `USER_NS`).
* **kABI Preservation:** Custom GKI System V IPC patch ensures total stability, preventing Kernel ABI mismatch bootloops while allowing Docker and system containers to function.

### 📡 NetHunter & External Wi-Fi Support
* **Dynamic Modprobe Modules:** High-performance wireless injection adapters compiled as loadable modules (`.ko`) to avoid memory linker overflows:
  * **Atheros:** `ath9k_htc` (AR9271), `ath9k`
  * **Realtek:** `rtl8187`
  * **Ralink:** `rt2800usb` (RT3070/RT5370)
* **Systemless Dynamic Loading:** AnyKernel3 automatically packages modules into Magisk systemless overlays under `/vendor/lib/modules/` at flash time.
* **USB HID Injection:** Full BadUSB keyboard/mouse injection support.
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
| `resukisu-susfs` | ReSukiSU | ✔️ | Enforcing |
| `sukisu-ultra-susfs`| SukiSU Ultra | ✔️ | Enforcing |

---

## 🛠️ Toolchain & Specifications

* **Target Architecture:** `ARM64-v8a`
* **SoC Platform:** Samsung Exynos 1380 (s5e8835)
* **Host OS Support:** One UI 8.0 / 8.5 (Android 16)
* **Compiler:** Google LLVM Clang Toolchain `r450784d`
* **LTO Optimizations:** Link-Time Optimization (`CONFIG_LTO_CLANG=y`) enabled for maximum efficiency.

---

## 📲 Installation Instructions

1. Download the correct variant `.zip` file for your setup from the [Releases](https://github.com/TheBizarreAbhishek/android_kernel_samsung_s5e8835/releases) tab.
2. Flash the ZIP using a custom recovery (**TWRP / OrangeFox**) or a dedicated kernel flasher app (like **Franco Kernel Manager / EX Kernel Manager**).
3. Reboot and enjoy!

# Bizarre Kernel — Samsung Exynos 1380 (`s5e8835`)

[![Build Status](https://img.shields.io/github/actions/workflow/status/TheBizarreAbhishek/android_kernel_samsung_s5e8835/build-kernels.yml?branch=android13-5.15&style=for-the-badge&logo=github)](https://github.com/TheBizarreAbhishek/android_kernel_samsung_s5e8835/actions)
[![Kernel](https://img.shields.io/badge/Linux-5.15.220%20GKI-2563eb?style=for-the-badge&logo=linux&logoColor=white)](https://android.googlesource.com/kernel/common/)
[![Architecture](https://img.shields.io/badge/Arch-ARM64-ea580c?style=for-the-badge&logo=arm&logoColor=white)](https://developer.arm.com/)
[![Platform](https://img.shields.io/badge/One%20UI-6.x%20--%209.x-16a34a?style=for-the-badge&logo=android&logoColor=white)](https://source.android.com/)
[![SoC](https://img.shields.io/badge/SoC-Exynos%201380%20(s5e8835)-6d28d9?style=for-the-badge)](https://semiconductor.samsung.com/processor/mobile-processor/exynos-1380/)

**Bizarre Kernel** is a high-performance, all-round custom kernel built for **all Samsung Exynos 1380 (`s5e8835`)** smartphones and tablets. Designed primarily for **speed, gaming performance, and smooth 120Hz responsiveness**, it pairs safe CPU/GPU overclocking and undervolting with smart memory management and clean thermal headroom so your device stays fast without unnecessary heat or drain.

---

## Performance & Clocks at a Glance

| Domain | Stock Max | Bizarre Kernel | Benefit |
| :--- | :---: | :---: | :--- |
| **Big CPU (`Cortex-A78`)** | `2.40 GHz` (`2400 MHz`) | **`2.70 GHz`** (`2704 MHz`) | Faster app launches, gaming FPS, and heavy multitasking |
| **Little CPU (`Cortex-A55`)** | `2.00 GHz` (`2002 MHz`) | **`2.21 GHz`** (`2210 MHz`) | Snappier UI scrolling and background responsiveness |
| **GPU (`Mali-G68 MP5`)** | `949 MHz` | **`1053 MHz`** | Higher graphics performance and steadier frame rates |
| **DSU Interconnect** | `1196 MHz` (`1.20 GHz`) | **`1898 MHz`** (`1.90 GHz`) | Lower cache/cluster latency under load |
| **Voltage Tuning** | `0%` (Stock) | **`-2%` Undervolt** | Reduced power draw and heat across CPU and GPU |
| **Thermal Limits** | `75°C` / `85°C` CPU (`88°C` GPU) | **`80°C` / `90°C` CPU (`93°C` GPU)** | `+5°C` headroom for longer sustained peak performance |

---

## Key Features

### Performance, Overclocking & Gaming
* **Safe CPU & GPU Overclocking:** Pushes **Big (`A78`)** from `2.40 GHz` to **`2.70 GHz`**, **Little (`A55`)** from `2.00 GHz` to **`2.21 GHz`**, **DSU** from `1196 MHz` to **`1898 MHz`**, and **Mali-G68 GPU** from `949 MHz` to **`1053 MHz`** for noticeably faster daily speed and gaming stability.
* **Balanced `-2%` Undervolting:** Applies a stable `-2%` voltage reduction across both CPU clusters and the GPU (vs. stock `0%` margin) to offset extra heat and improve performance-per-watt.
* **Extended Thermal Headroom (`+5°C`):** Raises thermal thresholds cleanly from stock (`75°C` switch-on, `85°C` CPU / `88°C` GPU) to **`80°C` switch-on, `90°C` CPU / `93°C` GPU** so clocks stay higher for longer during gaming and heavy workloads.
* **120Hz UI & Touch Responsiveness:** Tuned `schedutil` governor response times and GPU touch boost (`455 MHz`) for fluid 120Hz animations and scrolling.
* **Faster App Loading & Storage I/O:** `512 KB` storage read-ahead, tuned `F2FS` latency, and optimized ARM64 memory routines (`clear_page` / `memcmp`) speed up cold app opens.
* **PC Emulation Ready (`NTSYNC`):** Native `NTSYNC` driver support for higher FPS and smoother performance in Windows emulators like Winlator, Mobox, and Wine.
* **Low-Latency Networking (`BBRv3` + `FQ`):** Google BBRv3 congestion control with Fair Queuing reduces ping spikes in online games and stabilizes Wi-Fi/5G speeds.

### Memory & Multitasking
* **Dual-Stage ZRAM (`LZ4` + `ZSTD`):** Uses fast `LZ4` while the screen is on for zero-lag responsiveness, and compresses idle memory deeper with `ZSTD` when the screen turns off to free extra RAM.
* **Fewer App Reloads (`MGLRU` + `KSM`):** Multi-Gen LRU memory management and duplicate page merging keep more apps alive in the background during heavy multitasking.
* **Automatic Memory Defragmentation:** Compacts fragmented memory blocks during idle screen-off states to maintain long-term smoothness across multi-day uptime.

### Daily Balance & Efficiency
* **Smart Core Scheduling:** Steers low-priority background work to the efficiency Little cores (`A55`), keeping the Big cores (`A78`) ready for foreground apps and games.
* **Unthrottled Screen-On Charging:** Keeps full fast-charging speeds active even while using the phone with the screen turned on.
* **Clean Kernel Overhead:** Strips Samsung diagnostic log spam and trims unnecessary wakelocks/wakeup overhead for cooler idle temperatures and solid overall battery life.

### Root, Privacy & Freedom
* **KernelSU & KernelSU-Next Support:** Built-in kernel-level root management with granular per-app control.
* **SUSFS Stealth Integration:** Hides root, mounts, and modifications cleanly for banking apps, UPI payments, and work profiles.
* **Knox Restrictions Removed:** Disables restrictive Samsung security modules (`DEFEX`, `PROCA`, `FIVE`) for seamless root and module compatibility.
* **Built-In WireGuard & Hotspot Freedom:** Native in-kernel WireGuard VPN support and carrier tethering/hotspot restriction bypass.

---

## Compatibility

* **Supported Platform:** All Samsung devices powered by the **Exynos 1380** SoC (Codename: **`s5e8835`**)
* **OS / ROM Support:** **One UI 6.x – 9.x** (`Android 14 – 17`), as well as GSIs and custom ROMs

---

## Release Variants

Pre-built AnyKernel3 flashable `.zip` packages are published on the [Releases](https://github.com/TheBizarreAbhishek/android_kernel_samsung_s5e8835/releases) page:

| Variant | Root Engine | SUSFS Stealth | Best Suited For |
| :--- | :---: | :---: | :--- |
| **`ksu-susfs`** | KernelSU | Yes | Daily use with root + banking/UPI app hiding *(Recommended)* |
| **`ksunext-susfs`** | KernelSU-Next | Yes | KernelSU-Next users wanting full root hiding support |
| **`ksu`** | KernelSU | No | Standard KernelSU without SUSFS |
| **`ksunext`** | KernelSU-Next | No | Standard KernelSU-Next without SUSFS |
| **`clean`** | None | No | Unrooted users wanting all performance, OC/UV, and memory upgrades |
| **`permissive`** | None | No | Testing GSIs or ROM ports that require SELinux Permissive |

---

## Installation

1. Download your preferred `.zip` variant from [Releases](https://github.com/TheBizarreAbhishek/android_kernel_samsung_s5e8835/releases).
2. Flash the `.zip` using **TWRP / OrangeFox Recovery** or a root flasher app (**Kernel Flasher** / **Franco Kernel Manager**).
   *No `/data` wipe is required.*
3. Reboot and enjoy.

---

## Build Specifications

* **Platform:** Samsung Exynos 1380 (`s5e8835`)
* **Kernel Base:** Linux Android Common Kernel `5.15.220` (GKI)
* **Architecture:** ARM64 (`arm64-v8a`)
* **Compiler:** Google Clang LLVM `r450784d` (ThinLTO + CFI)
* **Packaging:** AnyKernel3

---

## Credits

* **Linux Kernel & AOSP**
* **Samsung Open Source Release Center (OSRC)**
* **osm0sis** — AnyKernel3
* **tiann** — KernelSU
* **rifsxd & KernelSU-Next Team** — KernelSU-Next
* **simonpunk** — SUSFS
* **Linux MM / ZRAM Contributors**

---

<p align="center">
  <b>Crafted with ❤️ by <a href="https://github.com/TheBizarreAbhishek">Abhishek Babu (@TheBizarreAbhishek)</a></b>
</p>

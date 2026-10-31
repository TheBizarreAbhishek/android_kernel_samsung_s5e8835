# 🌀 Bizarre Kernel | Samsung Exynos 1380 (`s5e8835`)

[![Build Status](https://img.shields.io/github/actions/workflow/status/TheBizarreAbhishek/android_kernel_samsung_s5e8835/build-kernels.yml?branch=android13-5.15&style=for-the-badge&logo=github)](https://github.com/TheBizarreAbhishek/android_kernel_samsung_s5e8835/actions)
[![Kernel Version](https://img.shields.io/badge/GKI_Kernel-5.15.216-blue?style=for-the-badge&logo=linux)](https://android.googlesource.com/kernel/common/)
[![Architecture](https://img.shields.io/badge/Arch-ARM64--v8a-orange?style=for-the-badge&logo=arm)](https://developer.arm.com/)
[![Android Version](https://img.shields.io/badge/Android-13%20--%2016-green?style=for-the-badge&logo=android)](https://source.android.com/)
[![Universal Exynos 1380](https://img.shields.io/badge/SoC-Exynos%201380%20(Universal)-purple?style=for-the-badge)](https://semiconductor.samsung.com/processor/mobile-processor/exynos-1380/)

A clean, high-performance, and battery-optimized custom kernel crafted for **all Samsung Exynos 1380 (`s5e8835`)** smartphones and tablets. Built to deliver cooler temperatures, longer battery life, lag-free 120Hz scrolling, and seamless root stealth for banking apps.

---

## 📱 Supported Devices

> **Universal Compatibility:** This kernel works across **all Samsung devices powered by the Exynos 1380 platform**.

| Device Model | Codename | Model Numbers |
| :--- | :--- | :--- |
| **Galaxy M35 5G** | `m35xxx` | `SM-M356B`, `SM-M356E` |
| **Galaxy A35 5G** | `a35xxx` | `SM-A356B`, `SM-A356E`, `SM-A356U`, `SM-A3560` |
| **Galaxy A54 5G** | `a54xxx` | `SM-A546B`, `SM-A546E`, `SM-A546U`, `SM-A5460`, `SM-A546V` |
| **Galaxy F54 5G** | `f54xxx` | `SM-E546B` |
| **Galaxy Tab S9 FE** | `gts9fe` | `SM-X510` (Wi-Fi), `SM-X516B` / `SM-X516C` / `SM-X518U` (5G) |
| **Galaxy Tab S9 FE+** | `gts9fep` | `SM-X610` (Wi-Fi), `SM-X616B` / `SM-X616C` (5G) |

*Compatible with **One UI 6.x**, **One UI 7.x**, and upcoming Android 15/16 GSI and custom ROMs.*

---

## 🚀 Key Features & Benefits

### 🔋 Battery Life & Efficiency
* **Background Task Little-Core Restriction:** Automatically keeps background apps, sync tasks, and system services pinned strictly to energy-saving Little cores (A55). Prevents big performance cores from waking up when your phone is in your pocket or idle.
* **Screen-On Fast Charging (Bypass Throttling):** Keeps full 25W fast charging active even when your screen is turned on and you are actively using your phone (removes Samsung's default screen-on charging slowdown).
* **Mali GPU Touch Boost Tuning:** Optimizes GPU touch frequency to significantly save battery during web and social media scrolling without sacrificing 120Hz smoothness.
* **Power-Efficient Workqueues:** Groups kernel background maintenance tasks onto energy-efficient cores to maximize Screen-On-Time (SOT).
* **Stripped Samsung Background Logging:** Eliminates continuous Samsung debug overhead, log spam, and diagnostic tracing, reducing idle heat and battery drain.

---

### ⚡ Performance & Gaming
* **Buttery-Smooth 120Hz Responsiveness:** Tuned CPU governor responsiveness for instant touch reaction and zero micro-stutters during heavy gaming or rapid multitasking.
* **Google BBRv3 + Fair Queuing (FQ):** Next-generation network congestion control that slashes latency, minimizes ping fluctuations in competitive games (BGMI, COD, Free Fire), and boosts Wi-Fi and 5G download stability.
* **Faster App Cold Starts:** Expanded storage read-ahead buffer so heavy games and large applications open noticeably quicker from storage.
* **NTSYNC Gaming Driver:** Fast Windows synchronization primitives for significantly higher FPS and smoother PC game emulation via Winlator, Mobox, and Wine.

---

### 🧠 Smart Memory Management (Multitasking)
* **Dual-Stage Smart ZRAM Compression:** Blazing-fast compression while using your phone, combined with deep screen-off recompression that frees up **1.5 GB to 2.0 GB of extra RAM**.
* **Zero App Reloads (MGLRU):** Modern Multi-Gen LRU memory engine keeps your favorite and recent apps alive in the background without aggressive kills or annoying reloads.
* **Idle Memory Defragmentation:** Automatically cleans up and organizes compressed memory when your phone screen turns off, preventing memory fragmentation over days of continuous uptime.
* **Duplicate RAM Page Merging:** Deduplicates identical memory pages across apps to save extra memory for heavy workflows.

---

### 🛡️ Root, Stealth & Freedom
* **Next-Gen Kernel Root (KernelSU & KernelSU-Next):** Modern kernel-level root access with complete per-app permission controls.
* **SUSFS Root Stealth:** Advanced cloaking technology that hides root, patched partitions, and mounts from strict banking apps, UPI apps (Google Pay, PhonePe, Paytm), and work profiles.
* **Knox Security Hooks Removed:** Stripped restrictive Samsung Knox security subsystems (`DEFEX`, `PROCA`, `FIVE`) so root and root modules work seamlessly.
* **Carrier Hotspot & Tethering Freedom:** Bypasses carrier hotspot restrictions and data throttling so you can share your mobile data without limits.
* **In-Kernel WireGuard VPN:** Native VPN engine built directly into the kernel for fast, battery-friendly encrypted networking.

---

## 📦 Download Variants

Pre-built AnyKernel3 flashable `.zip` packages are automatically generated on every release:

| Package Variant | Root Method | SUSFS Stealth | Recommended For |
| :--- | :---: | :---: | :--- |
| **`ksu-susfs`** | KernelSU | 🛡️ Yes | **Daily driving with banking apps, UPI, and full root stealth** *(Recommended)* |
| **`ksunext-susfs`** | KernelSU-Next | 🛡️ Yes | Users who want the latest KernelSU-Next cutting-edge features with stealth |
| **`ksu`** | KernelSU | ❌ No | Standard KernelSU root without extra stealth patches |
| **`ksunext`** | KernelSU-Next | ❌ No | Standard KernelSU-Next root without stealth |
| **`clean`** | None | ❌ No | **100% Stock unrooted setup** with all performance, battery, and memory enhancements |
| **`permissive`** | None | ❌ No | Testing custom GSI builds or ports requiring SELinux Permissive |

---

## 📲 Installation Guide

### Via Custom Recovery (TWRP / OrangeFox)
1. Download your desired variant `.zip` file from the [Releases](https://github.com/TheBizarreAbhishek/android_kernel_samsung_s5e8835/releases) page.
2. Boot your phone into **TWRP / OrangeFox Recovery**.
3. Tap **Install**, select the downloaded kernel `.zip` file, and swipe to confirm.
   *(No need to wipe `/data` or cache — all your personal data and settings remain untouched).*
4. Tap **Reboot System**.

### Via Kernel Flasher App (Rooted Users)
1. Download the kernel `.zip` file on your device.
2. Open **Kernel Flasher** or **Franco Kernel Manager (FKM)**.
3. Select **Flash Kernel / Flash ZIP**, choose the downloaded `.zip`, and tap flash.
4. Reboot your device.

---

## ⚙️ Technical Specifications

* **Platform:** Samsung Exynos 1380 (`s5e8835`)
* **Base Kernel:** Linux Android Common Kernel `5.15.216` (GKI)
* **Architecture:** 64-bit ARM (`arm64-v8a`)
* **Toolchain:** Google Clang LLVM `r450784d` with Full Link-Time Optimization (LTO)
* **Installation Engine:** AnyKernel3 by osm0sis (bizarre edition)

---

## 🤝 Credits & Acknowledgements
* **Linux Kernel & Android Open Source Project (AOSP)**
* **Samsung Open Source Release Center (OSRC)**
* **osm0sis** for AnyKernel3
* **tiann** for KernelSU
* **rifsxd** & **KernelSU-Next team** for KernelSU-Next
* **simonpunk** for SUSFS
* **The Linux MM / ZRAM community** for multi-compression backports

---

<p align="center">
  <b>Crafted with ❤️ by Abhishek Babu (<a href="https://github.com/TheBizarreAbhishek">@TheBizarreAbhishek</a>)</b>
</p>

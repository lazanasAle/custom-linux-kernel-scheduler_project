# Linux Scheduling Class Implementation - Kernel 6.14.8

This repository contains the implementation of my kernel scheduling class algorithm  designed to be integrated with the **Linux 6.14.8 kernel** source tree.

## ⚙️ Requirements

- Linux kernel source tarball version **6.14.8**
- A working Linux environment with `gcc 14.1.0`, `make`, and required development tools
- Familiarity with kernel compilation and configuration

## 📦 Installation Instructions

1. **Download and Extract Kernel Source**

   Download the official Linux 6.14.8 kernel tarball and extract it:

   ```bash
   wget https://cdn.kernel.org/pub/linux/kernel/v6.x/linux-6.14.8.tar.xz
   tar -xf linux-6.14.8.tar.xz
   cd linux-6.14.8
   ```
   
2. **Integrate new Kernel Code**

Clone this repository, and copy the contents into the corresponding paths within the extracted kernel source. The directory structure in this repository matches the kernel tree, so each file must be placed in its matching destination (files already in the kernel will be overwritten with the modified ones of this repo).

```bash
cp -r custom-linux-kernel-scheduler_project/customized-kernel/* linux-6.14.8/
```

3. Configure and Build the Kernels

Run the following commands to configure and build the kernel with the new scheduler:

```bash
make menuconfig
make -j$(nproc) bzImage
```
A prebuilt `bzImage` is also included in this repository under:
`arch/x86/boot/bzImage`

you can use this `bzImage` in an emulator with qemu providing the equivalent initramfs.

--- 

## BPF_Uproach Folder

This folder contains:

Kernel modifications including additional system calls

A BPF program implementing the scheduling logic

Installation and usage steps are the same as the custom kernel installation. You can insert the BPF scheduler as a kernel module and test its functionality.

---

The testbenches folder includes user-space C programs designed to test:

The custom scheduler’s behavior

Integration of additional system calls


(Currently only the syscalls are fully Implemented this is just a checklist)

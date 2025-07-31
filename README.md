# Kernel 6.14.8: Implementation of a Linux Scheduling Class

The implementation of my kernel scheduling technique, which is designed to be merged with the **Linux 6.14.8 kernel** source tree, is contained in this repository.

## ⚙️ Requirements

- **6.14.8** Linux kernel source tarball version.
- A functional Linux environment with `gcc 14.1.0`, `make`, and necessary development tools (because linux 6.14.8 was made for `gcc 14`, more recent versions like `gcc 15` may display compile errors).
- Knowledge of kernel configuration and compilation

## Installation Guidelines

1. Extract the kernel source after downloading it.

   Get and extract the official Linux 6.14.8 kernel tarball:

   ```bash
   wget https://cdn.kernel.org/pub/linux/kernel/v6.x/linux-6.14.8.tar.xz
   tar -xf linux-6.14.8.tar.xz
   ```
   
2. **Integrate new Kernel Code**

Make a clone of this repository and move the contents of the `customized_kernel` folder into the corresponding locations in the kernel source that was extracted. Each file must be placed in its equivalent location because the directory structure of this folder corresponds to the kernel tree; files that are already in the kernel will be replaced by the updated files from this repository.


```bash
cp -r custom-linux-kernel-scheduler_project/customized-kernel/* linux-6.14.8/
```

3. Configure and Build the Kernels

Run the following commands to configure and build the kernel with the new scheduler:

```bash
cd linux-6.14.8
make menuconfig
make -j$(nproc) bzImage
```
A prebuilt `bzImage` is included in this repository under:
`arch/x86/boot/bzImage`

you can use this `bzImage` in an emulator with qemu providing the equivalent initramfs, or boot a linux vm with the bzImage as the -kernel option


---

The `testbenches` folder includes user-space C programs designed to test:

The custom scheduler’s behavior

Integration of additional system calls

---

The scheduler has been tested and is fully functional with **6.14.11** Linux kerneln version as well.

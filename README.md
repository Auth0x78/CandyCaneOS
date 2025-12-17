# CandyCane OS

A simple hobby kernel written in C. The goal is to explore x86 architecture, VGA graphics mode, and eventually develop a custom game.

---

## 🛠️ Build and Run

Ensure you have `gcc`, `nasm`, `ld`, `grub-mkrescue`, and `xorriso` installed.
To install the above packages you can run the following commands:
## 🛠️ Prerequisites (Linux)

Install the required toolchain using your distribution's package manager:

**Ubuntu / Debian / Mint / Kali:**
```bash
sudo apt update
sudo apt install build-essential nasm grub-common grub-pc-bin xorriso qemu-system-x86

```

**Arch Linux:**

```bash
sudo pacman -S base-devel nasm grub libisoburn qemu-full

```

**Fedora:**

```bash
sudo dnf groupinstall "Development Tools"
sudo dnf install nasm grub2-tools-extra libisoburn qemu-system-x86

```

**Verify Installation:**

```bash
gcc --version && nasm -v && ld -v && grub-mkrescue --version && xorriso --version

```

1. **Build the ISO:**
```bash
make

```


2. **Run in QEMU:**
```bash
make run

```


3. **Clean build files:**
```bash
make clean

```

---

## ❤️ Inspiration

The development of this kernel is inspired by [CinemintOS](https://github.com/EHowardHill/CinemintOS).

---
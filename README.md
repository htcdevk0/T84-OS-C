# T84 OS

![Kernel](https://img.shields.io/badge/kernel-bare--metal-darkblue)
![Language](https://img.shields.io/badge/language-C-blue)
![Architecture](https://img.shields.io/badge/arch-x86--32-lightgrey)
![Inspired](https://img.shields.io/badge/inspired-1984-orange)
![Status](https://img.shields.io/badge/status-in--development-yellow)

**T84 OS** is a small experimental operating system kernel inspired by operating systems from **1984**, mixing old-school design with a slightly more modern mindset.

This project focuses on **simplicity, low-level control, and learning**, rather than performance or full POSIX compliance.

---

## ✨ Philosophy

- Inspired by early personal computer operating systems (1980–1985)
- Minimal, understandable, and hackable
- No unnecessary abstractions
- Built to *learn how operating systems really work*

> If you can understand it, you can control it.

---

## 🧠 Features

- Bare-metal kernel written in **C and Assembly**
- Text-mode terminal interface
- Command-based shell
- Custom internal commands
- Runtime features (no pre-generated build-time behavior)
- Lightweight and educational by design

Current implemented concepts:
- Command parser
- Terminal output system
- Simple shell commands

---

## 🏗️ Architecture

- **CPU**: x86 (32-bit)
- **Bootloader**: GRUB (Multiboot)
- **Kernel type**: Monolithic (very small)
- **Mode**: Protected Mode
- **Graphics**: Text-mode (VGA)

Folder structure (simplified):


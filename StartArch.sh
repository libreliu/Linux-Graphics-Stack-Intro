#!/bin/bash

QEMU_BASE_PARAMS=

# qemu-system-x86_64 \
#   -m 16384 \
#   -cpu host \
#   -smp 10 \
#   -enable-kvm \
#   -drive file=archlinux.qcow2,format=qcow2 \
#   -nic user \
#   -boot d \
#   -display default,show-cursor=on \
#   -device ramfb \
#   -vga none \
#   -drive if=pflash,format=raw,readonly,file=/usr/share/edk2/x64/OVMF_CODE.4m.fd \
#   -usb -device usb-tablet

# qemu-system-x86_64 \
#   -m 16384 \
#   -cpu host \
#   -smp 10 \
#   -enable-kvm \
#   -drive file=archlinux.qcow2,format=qcow2 \
#   -nic user \
#   -boot d \
#   -display default,show-cursor=on \
#   -vga std \
#   -drive if=pflash,format=raw,readonly,file=/usr/share/edk2/x64/OVMF_CODE.4m.fd \
#   -usb -device usb-tablet

# qemu-system-x86_64 \
#   -m 16384 \
#   -cpu host \
#   -smp 10 \
#   -enable-kvm \
#   -drive file=archlinux.qcow2,format=qcow2 \
#   -nic user \
#   -boot d \
#   -display default,show-cursor=on \
#   -device virtio-gpu-pci \
#   -vga none \
#   -drive if=pflash,format=raw,readonly,file=/usr/share/edk2/x64/OVMF_CODE.4m.fd \
#   -usb -device usb-tablet

# qemu-system-x86_64 \
#   -m 16384 \
#   -cpu host \
#   -smp 10 \
#   -enable-kvm \
#   -drive file=archlinux.qcow2,format=qcow2 \
#   -nic user \
#   -boot d \
#   -display default,show-cursor=on \
#   -vga cirrus \
#   -drive if=pflash,format=raw,readonly,file=/usr/share/edk2/x64/OVMF_CODE.4m.fd \
#   -usb -device usb-tablet

  # -device virtio-gpu-pci \
  # -device ramfb \
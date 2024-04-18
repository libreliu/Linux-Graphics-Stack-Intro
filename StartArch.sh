#!/bin/bash

# Usage of script: ./script_name.sh [gpu_option]
# gpu_option can be 'ramfb', 'std', 'virtio', 'cirrus', or none for default generic VESA

gpu_option="${1}"

QEMU_BASE_PARAMS="\
  -m 16384 \
  -cpu host \
  -smp 10 \
  -enable-kvm \
  -drive file=archlinux.qcow2,format=qcow2 \
  -nic user \
  -boot d \
  -display default,show-cursor=on \
  -drive if=pflash,format=raw,readonly,file=/usr/share/edk2/x64/OVMF_CODE.4m.fd \
  -usb -device usb-tablet"

case $gpu_option in
  "ramfb")
    GPU_CONFIG="-device ramfb -vga none"
    ;;
  "std")
    GPU_CONFIG="-vga std"
    ;;
  "virtio")
    GPU_CONFIG="-device virtio-gpu-pci -vga none"
    ;;
  "cirrus")
    GPU_CONFIG="-vga cirrus"
    ;;
  "qxl-vga")
    GPU_CONFIG="-vga qxl"
    ;;
  *)
    GPU_CONFIG="-vga std" # default to std vga
    ;;
esac

echo "Using GPU_CONFIG ${GPU_CONFIG}"

# Start QEMU with chosen GPU configuration
qemu-system-x86_64 $QEMU_BASE_PARAMS $GPU_CONFIG

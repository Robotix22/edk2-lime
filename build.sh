#!/bin/bash
# based on the instructions from edk2-platform
set -e
. build_common.sh

rm -rf Build/limePkg/DEBUG_GCC5/FV/Ffs/7E374E25-8E01-4FEE-87F2-390C23C606CDFVMAIN
# not actually GCC5; it's GCC7 on Ubuntu 18.04.
GCC5_AARCH64_PREFIX=aarch64-linux-gnu- build -s -n 0 -a AARCH64 -t GCC5 -p limePkg/lime.dsc -b DEBUG
gzip -c < Build/limePkg/DEBUG_GCC5/FV/LIMEPKG_UEFI.fd >uefi_image
cat lime.dtb >>uefi_image
mkbootimg --header_version 1 --pagesize 4096 --id --os_version 12.0 --os_patch_level 2022-01-01 --kernel uefi_image --ramdisk ramdisk -o boot-lime.img

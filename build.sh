#!/bin/bash
for i in "${EDK2}" ./edk2 ../edk2
do	if [ -n "${i}" ]&&[ -f "${i}/edksetup.sh" ]
	then	_EDK2="$(realpath "${i}")"
		break
	fi
done
for i in "${EDK2_PLATFORMS}" ./edk2-platforms ../edk2-platforms
do	if [ -n "${i}" ]&&[ -d "${i}/Platform" ]
	then	_EDK2_PLATFORMS="$(realpath "${i}")"
		break
	fi
done
for i in "${SIMPLE_INIT}" sdm845Pkg/Library/SimpleInit ./simple-init ../simple-init
do	if [ -n "${i}" ]&&[ -f "${i}/SimpleInit.inc" ]
	then	_SIMPLE_INIT="$(realpath "${i}")"
		break
	fi
done
export CROSS_COMPILE="${CROSS_COMPILE:-aarch64-linux-gnu-}"
export GCC5_AARCH64_PREFIX="${CROSS_COMPILE}"
export CLANG38_AARCH64_PREFIX="${CROSS_COMPILE}"
export PACKAGES_PATH="$_EDK2:$_EDK2_PLATFORMS:$_SIMPLE_INIT:$PWD"
export WORKSPACE="${PWD}"
set -e
mkdir -p "${_SIMPLE_INIT}/build" "${_SIMPLE_INIT}/root/usr/share/locale"
for i in "${_SIMPLE_INIT}/po/"*.po
do	[ -f "${i}" ]||continue
	_name="$(basename "$i" .po)"
	_path="${_SIMPLE_INIT}/root/usr/share/locale/${_name}/LC_MESSAGES"
	mkdir -p "${_path}"
	msgfmt -o "${_path}/simple-init.mo" "${i}"
done
bash "${_SIMPLE_INIT}/scripts/gen-rootfs-source.sh" \
	"${_SIMPLE_INIT}" \
	"${_SIMPLE_INIT}/build"

source "${_EDK2}/edksetup.sh"
set -e
make -C "${_EDK2}/BaseTools"||exit "$?"
build -s -n 0 -a AARCH64 -t "GCC5" -p "limePkg/lime.dsc" -b "DEBUG" 
gzip -c < "Build/limePkg/DEBUG_GCC5/FV/LIMEPKG_UEFI.fd" > "uefi-lime.img.gz" 
cat "uefi-lime.img.gz" "lime.dtb" > "uefi-lime.img.gz-dtb" 
mkbootimg --kernel "uefi-lime.img.gz-dtb" --ramdisk ramdisk --kernel_offset 0x00000000 --ramdisk_offset 0x00000000 --tags_offset 0x00000000 --os_version "12.0.0" --os_patch_level "$(date '+%Y-%m')" --header_version 1 -o "boot-lime.img" 
echo "Build Done: boot-lime.img"

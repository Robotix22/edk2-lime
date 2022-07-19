# EDK2 UEFI Firmware For Xiaomi Redmi 9T

## Resources

[Discord Server](https://discord.gg/Gb4KAqAQdm)

## Status

Can Boot into UEFI Shell

## Working

Display

## Not Working

...

## Building

You need to clone these repositories 

```bash
git clone https://github.com/Robotix22/edk2-lime.git
git clone https://github.com/tianocore/edk2.git --recursive
git clone https://github.com/tianocore/edk2-platforms.git
```
You should have all three directories side by side.

Now Install needed Packages

For Debian and Ubuntu:

```bash
sudo apt install build-essential uuid-dev iasl git nasm gcc-aarch64-linux-gnu mkbootimg python3-distutils gettext
```

Now you can build the UEFI Image by doing

```bash
cd edk2-lime
./firstrun.sh
./build.sh
```

And finally you can now boot the Image or flash the image to `boot` or `recovery`

```bash
fastboot boot boot-lime.img
fastboot flash boot boot-lime.img
fastboot flash recovery boot-lime.img
```

## Credits

This is based on `edk2-sdm845` from `edk2-porting`

Thanks to `Xhelowrk` for Testing

SimpleFbDxe is based on `NintendoSwitchPkg` from `imbushuo`

# SDK Knowledge

### Author Manas Chachra (H582143)

- **BSP folder**: Use `make linux_config` to change Linux kernel settings and add/remove modules to the kernel.
- **root-fs folder**: Contains the `rootfs` folder, which is a copy of the filesystem stored inside the device. You can use this as a reference for what the device will actually contain once it boots.
- **code folder**: Add third-party binaries, etc., if you want to cross-compile to `applications/external` folder. Similarly, add libraries to the `lib/external` folder.
- **configs folder**: Contains configuration files.
- **cfg_gen folder**: Add applications to build at compile time to `nvt-info.dtsi`.
- **nvt-top**: Use this tool to quickly enable or disable hardware modules.

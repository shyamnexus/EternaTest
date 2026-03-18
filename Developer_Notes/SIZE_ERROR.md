## Partition Size Error: "Partition[7] Size is too smaller than that you wanna update."

### Author Manas Chachra (H582143)


**Example error:**
```
Partition[7] Size is too smaller than that you wanna update.(0x015CD5B0 > 0x01400000)
nvtpack:-5
failed sanity.
Update fail
```

### What it means
- The size of the partition (e.g., rootfs) defined in `nvt-storage-partition.dtsi` is smaller than the actual image you are trying to write to it.
- Partition[7] typically refers to the `rootfs` partition.

### How to fix

1. **Check the error message:**  
   Note the required size (e.g., `0x015CD5B0`) and the current partition size (e.g., `0x01400000`).

2. **Update partition size:**
   - Go to `configs/cfg_gen/nvt-storage-partition.dtsi`.
   - Increase the size of the `rootfs` partition (or the partition indicated by the error) so it is larger than the image you are trying to write.

3. **Adjust LEB count if needed:**
   - If you get a LEB count error in the build process, also update the `ROOTFS_UBI_MAX_LEB_COUNT` or similar values in `mtd_cfg.txt` to match the new partition size.

4. **Rebuild the firmware.**

**Tip:**  
Always ensure the partition size in the `.dtsi` file is larger than the image size generated at the end of the build process.

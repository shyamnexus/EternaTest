INCLUDE $LDS_EXTERN
/* Linker script for malta
 *
 * Version: Sourcery G++ Lite 4.3-221
 * Support: https://support.codesourcery.com/GNUToolchain/
 *
 * Copyright (c) 2007, 2008, 2009 CodeSourcery, Inc.
 *
 * The authors hereby grant permission to use, copy, modify, distribute,
 * and license this software and its documentation for any purpose, provided
 * that existing copyright notices are retained in all copies and that this
 * notice is included verbatim in any distributions.  No written agreement,
 * license, or royalty fee is required for any of the authorized uses.
 * Modifications to this software may be copyrighted by their authors
 * and need not follow the licensing terms described here, provided that
 * the new terms are clearly indicated on the first page of each file where
 * they apply.
 * */

/* _RTOS_HEAP_SIZE: for FreeRTOS's heap-method6, only be used on FreeRTOS-Kernel */
_RTOS_HEAP_SIZE = DEFINED(_RTOS_HEAP_SIZE) ? _RTOS_HEAP_SIZE : 0x100000;

/* Following Stacks are used for each CPU modes on boot.S */

#if defined(__aarch64__)
_EL1_STACK_SIZE = 2048;
_EL2_STACK_SIZE = 2048;
_EL3_STACK_SIZE = 2048;
#else
_ABORT_STACK_SIZE = 1024;
_SUPERVISOR_STACK_SIZE = 2048;
_IRQ_STACK_SIZE = 1024;
_FIQ_STACK_SIZE = 1024;
_UNDEF_STACK_SIZE = 1024;
_SYS_STACK_SIZE = 0xA000;
#endif

OUTPUT_ARCH($ARCH)
ENTRY(_nvtboot)
SECTIONS
{
    .START_UP $START_UP: {
        _section_01_addr = ABSOLUTE(.);
        KEEP (*(.nvtboot))
    }
	.BIN_INFO $BIN_INFO: {
        KEEP(./startup/bin_info.o(.data*));
    }
    .CODE_INFO $CODE_INFO: {
        KEEP(./startup/code_info.o(.data*));
        . = ALIGN(0x8);
        KEEP (*(.boot))
        . = ALIGN(0x800);
        KEEP (*(.vectors))
    }

    _stext = .;

	.ctors (ALIGN (0x8)) : {
        __CTOR_LIST__ = .;
        KEEP (*(SORT(.ctors.*)))
        KEEP (*(.ctors))
        KEEP (*(SORT(.init_array.*)))
        KEEP (*(.init_array))
        __CTOR_END__ = .;
    }

    .dtors (ALIGN (0x8)) : {
        __DTOR_LIST__ = .;
        KEEP (*(SORT(.dtors.*)))
        KEEP (*(.dtors))
        KEEP (*(SORT(.fini_array.*)))
        KEEP (*(.fini_array))
        __DTOR_END__ = .;
    }

    .init : {
        KEEP (*(.init))
    }

    .fini : {
        KEEP (*(.fini))
    }

    .fixup : {
        __fixup_start = .;
        *(.fixup)
        __fixup_end = .;
    }

    .gcc_except_table : {
        *(.gcc_except_table)
    }

    .ARM.exidx : {
        __exidx_start = .;
        *(.ARM.exidx*)
        *(.gnu.linkonce.armexidix.*.*)
        __exidx_end = .;
    }

    .ARM.attributes : {
        __ARM.attributes_start = .;
        *(.ARM.attributes)
        __ARM.attributes_end = .;
    }

    .devtab (ALIGN (0x8)) : {
        __MAINFUNCTAB_BEGIN__ = .;
        . = .; KEEP(*( SORT (.cmdsys.table.*))) ;
        __MAINFUNCTAB_END__ = .;
        __EXAMFUNCTAB_BEGIN__ = .;
        . = .; KEEP(*( SORT (.examsys.table.*))) ;
        __EXAMFUNCTAB_END__ = .;
    }

#if defined(__aarch64__)
    .mmu_tbl0 (ALIGN(4096)) : {
        __mmu_tbl0_start = .;
        *(.mmu_tbl0)
        __mmu_tbl0_end = .;
    }

    .mmu_tbl1 (ALIGN(4096)) : {
        __mmu_tbl1_start = .;
        *(.mmu_tbl1)
        __mmu_tbl1_end = .;
    }

    .mmu_tbl2 (ALIGN(4096)) : {
        __mmu_tbl2_start = .;
        *(.mmu_tbl2)
        __mmu_tbl2_end = .;
    }
#endif

    .SECTION_01 (ALIGN (0x8)): {
        *(.boot)
        *(.gcc_except_table)
        *(.newlib)
        *(.jcr)
        *(.got)
        *(.got.plt)
        /* app */
        ./startup/startup.o (.text* .data* .data1* .rodata* .rodata1* .reginfo* .init* .exit*)
        ./rtos-main.o (.text* .data* .data1* .rodata* .rodata1* .reginfo* .init* .exit*)
        ./system/sys_fdt.o (.text* .data* .data1* .rodata* .rodata1* .reginfo* .init* .exit*)
        ./system/sys_fwload.o (.text* .data* .data1* .rodata* .rodata1* .reginfo* .init* .exit*)
        ./system/sys_fastboot.o (.text* .data* .data1* .rodata* .rodata1* .reginfo* .init* .exit*)
        ./system/sys_storage_partition.o (.text* .data* .data1* .rodata* .rodata1* .reginfo* .init* .exit*)
        ./flow/flow_preview.o (.text* .data* .data1* .rodata* .rodata1* .reginfo* .init* .exit*)
        ./flow/flow_encode.o (.text* .data* .data1* .rodata* .rodata1* .reginfo* .init* .exit*)
        ./flow/fast_panel.o (.text* .data* .data1* .rodata* .rodata1* .reginfo* .init* .exit*)
        ./flow/fast_ai.o (.text* .data* .data1* .rodata* .rodata1* .reginfo* .init* .exit*)
        ./flow/fast_audio.o (.text* .data* .data1* .rodata* .rodata1* .reginfo* .init* .exit*)
        ./flow/fast_audio_sound.o (.text* .data* .data1* .rodata* .rodata1* .reginfo* .init* .exit*)
        ./flow/fast_venc.o (.text* .data* .data1* .rodata* .rodata1* .reginfo* .init* .exit*)
        ./flow/fast_sensor.o (.text* .data* .data1* .rodata* .rodata1* .reginfo* .init* .exit*)
        ./flow/flow_boot_linux.o (.text* .data* .data1* .rodata* .rodata1* .reginfo* .init* .exit*)
        ./flow/bridge_mem.o (.text* .data* .data1* .rodata* .rodata1* .reginfo* .init* .exit*)
        /* lib */
        libfwsrv.a (.text* .data* .data1* .rodata* .rodata1* .reginfo* .init* .exit*)
        libutility.a (.text* .data* .data1* .rodata* .rodata1* .reginfo* .init* .exit*)
        libzlib.a (.text* .data* .data1* .rodata* .rodata1* .reginfo* .init* .exit*)
        libDbgUt.a (.text* .data* .data1* .rodata* .rodata1* .reginfo* .init* .exit*)
        /* driver */
        libdrv_portable.a (.text* .data* .data1* .rodata* .rodata1* .reginfo* .init* .exit*)
        libdrv_drtc.a (.text* .data* .data1* .rodata* .rodata1* .reginfo* .init* .exit*)
        libdrv_rtc.a (.text* .data* .data1* .rodata* .rodata1* .reginfo* .init* .exit*)
        libdrv_serial.a (.text* .data* .data1* .rodata* .rodata1* .reginfo* .init* .exit*)
        libdrv_storage.a (.text* .data* .data1* .rodata* .rodata1* .reginfo* .init* .exit*)
        /* kdrv */
        libkdrv_usb2dev.a (.text* .data* .data1* .rodata* .rodata1* .reginfo* .init* .exit*)
        libkdrv_comm.a (.text* .data* .data1* .rodata* .rodata1* .reginfo* .init* .exit* .init* .exit*)
        libkdrv_hrtimer.a (.text* .data* .data1* .rodata* .rodata1* .reginfo* .init* .exit* .init* .exit*)
        libkdrv_cpu.a (.text* .data* .data1* .rodata* .rodata1* .reginfo* .init* .exit*)
        libkdrv_rtosfdt.a (.text* .data* .data1* .rodata* .rodata1* .reginfo* .init* .exit*)
        libkdrv_top.a (.text* .data* .data1* .rodata* .rodata1* .reginfo* .init* .exit*)
        libkdrv_pll.a (.text* .data* .data1* .rodata* .rodata1* .reginfo* .init* .exit*)
        libkdrv_gpio.a (.text* .data* .data1* .rodata* .rodata1* .reginfo* .init* .exit*)
        libkdrv_adc.a (.text* .data* .data1* .rodata* .rodata1* .reginfo* .init* .exit*)
        libkdrv_interrupt.a (.text* .data* .data1* .rodata* .rodata1* .reginfo* .init* .exit*)
        libker_pkg.a (.text* .data* .data1* .rodata* .rodata1* .reginfo* .init* .exit*)
        /* os */
        ./startup/bin_info.o(.got*)
        libkwrap.a (.text.* .rodata.* .text* .data* .data1* .rodata* .rodata1* .reginfo* .init* .exit*)
        libvos.a (.text.* .rodata.* .text* .data* .data1* .rodata* .rodata1* .reginfo* .init* .exit*)
        /* freertos.a must be located after kwrap.a for linking text.xEventGroup* */
        libfreertos-posix.a (.text* .data* .data1* .rodata* .rodata1* .reginfo* .init* .exit*)
        libfreertos.a (.text* .data* .data1* .rodata* .rodata1* .reginfo* .init* .exit*)
        /* std-c */
        libc.a (.text* .data* .data1* .rodata* .rodata1* .reginfo* .init* .exit*)
        libgcc.a (.text* .data* .data1* .rodata* .rodata1* .reginfo* .init* .exit*)
        libstdc++.a (.text* .data* .data1* .rodata* .rodata1* .reginfo* .init* .exit*)
    }

	.SECTION_02 (ALIGN (0x8)): {
        _section_01_size = ABSOLUTE(.) - _section_01_addr;
        _section_02_addr = ABSOLUTE(.);
        /* fdt */
        _fdt_sensor = ABSOLUTE(.);
        KEEP(./dts/sensor.o (.sensor*));
        /* ext_drivers */
        $SENSOR1.a(.text* .data* .data1* .rodata* .rodata1* .reginfo* .init* .exit*)
        /* kflow */
        libkflow_common.a (.text* .data* .data1* .rodata* .rodata1* .reginfo* .init* .exit*)
        libkflow_videocapture.a (.text* .data* .data1* .rodata* .rodata1* .reginfo* .init* .exit*)
        /* kdrv */
        libkdrv_videocapture.a (.text* .data* .data1* .rodata* .rodata1* .reginfo* .init* .exit*)
        libvendor_media.a (.text* .data* .data1* .rodata* .rodata1* .reginfo* .init* .exit*)
        libkdrv_builtin$LIB_SUFFIX.a (.text* .data* .data1* .rodata* .rodata1* .reginfo* .init* .exit*)
        /* IO */
        libkdrv_i2c.a (.text* .data* .data1* .rodata* .rodata1* .reginfo* .init* .exit*)
        libnvt_spi.a (.text* .data* .data1* .rodata* .rodata1* .reginfo* .init* .exit*)
        libkdrv_gpio.a (.text* .data* .data1* .rodata* .rodata1* .reginfo* .init* .exit*)
    }

	.SECTION_03 (ALIGN (0x8)): {
        _section_02_size = ABSOLUTE(.) - _section_02_addr;
        _section_03_addr = ABSOLUTE(.);
        /* app */
        ./system/sys_nvtmpp.o (.text* .data* .data1* .rodata* .rodata1* .reginfo* .init* .exit*)
        ./system/sys_mempool.o (.text* .data* .data1* .rodata* .rodata1* .reginfo* .init* .exit*)
#if (_PACKAGE_VIDEO_)
        /* hdal */
        libhdal.a(.text* .data* .data1* .rodata* .rodata1* .reginfo* .init* .exit*)
        /* kflow */
        libkflow_videoprocess$LIB_SUFFIX.a (.text* .data* .data1* .rodata* .rodata1* .reginfo* .init* .exit*)
        libnvt_videosprite.a (.text* .data* .data1* .rodata* .rodata1* .reginfo* .init* .exit*)
        libnvt_gfx.a (.text* .data* .data1* .rodata* .rodata1* .reginfo* .init* .exit*)
        /* kdrv */
        libkdrv_videocapture.a (.text* .data* .data1* .rodata* .rodata1* .reginfo* .init* .exit*)
        libkdrv_videoprocess$LIB_SUFFIX.a (.text* .data* .data1* .rodata* .rodata1* .reginfo* .init* .exit*)
        libkdrv_gfx2d.a (.text* .data* .data1* .rodata* .rodata1* .reginfo* .init* .exit*)
        /* ISP */
        libnvt_ae.a (.text* .data* .data1* .rodata* .rodata1* .reginfo* .init* .exit*)
        libnvt_awb.a (.text* .data* .data1* .rodata* .rodata1* .reginfo* .init* .exit*)
        libnvt_iq.a (.text* .data* .data1* .rodata* .rodata1* .reginfo* .init* .exit*)
        libnvt_isp.a (.text* .data* .data1* .rodata* .rodata1* .reginfo* .init* .exit*)
#endif
    }

	.SECTION_04 (ALIGN (0x8)): {
        _section_03_size = ABSOLUTE(.) - _section_03_addr;
        _section_04_addr = ABSOLUTE(.);
        /* display */
#if (_PACKAGE_DISPLAY_)
        $LCD1.a(.text* .data* .data1* .rodata* .rodata1* .reginfo* .init* .exit*)
        libnvt_dispdev_panel.a(.text* .data* .data1* .rodata* .rodata1* .reginfo* .init* .exit*)
        libkflow_videoout.a(.text* .data* .data1* .rodata* .rodata1* .reginfo* .init* .exit*)
        libkdrv_videoout.a(.text* .data* .data1* .rodata* .rodata1* .reginfo* .init* .exit*)
        libnvt_sif.a(.text* .data* .data1* .rodata* .rodata1* .reginfo* .init* .exit*)
        libnvt_ide.a(.text* .data* .data1* .rodata* .rodata1* .reginfo* .init* .exit*)
#endif
        /* storage */
        ./system/sys_card.o (.text* .data* .data1* .rodata* .rodata1* .reginfo* .init* .exit*)
        ./system/sys_filesys.o (.text* .data* .data1* .rodata* .rodata1* .reginfo* .init* .exit*)
#if (_PACKAGE_FILESYS_)
        libFileSys.a(.text* .data* .data1* .rodata* .rodata1* .reginfo* .init* .exit*)
        libFsUitron.a(.text* .data* .data1* .rodata* .rodata1* .reginfo* .init* .exit*)
#endif
        libdrv_storage.a(.text* .data* .data1* .rodata* .rodata1* .reginfo* .init* .exit*)
	libnvt_gfx.a (.text* .data* .data1* .rodata* .rodata1* .reginfo* .init* .exit*)
	libkdrv_gfx2d.a (.text* .data* .data1* .rodata* .rodata1* .reginfo* .init* .exit*)
	./flow/video_frm_416_128_1_argb4444.o (.text* .data* .data1* .rodata* .rodata1* .reginfo* .init* .exit*)
    }

    .SECTION_05 (ALIGN (0x8)):
    {
        _section_04_size = ABSOLUTE(.) - _section_04_addr;
        _section_05_addr = ABSOLUTE(.);
#if (_PACKAGE_VIDEO_)
        /* video codec */
        libkflow_videoenc.a(.text* .data* .data1* .rodata* .rodata1* .reginfo* .init* .exit*)
        libkdrv_h26x.a(.text* .data* .data1* .rodata* .rodata1* .reginfo* .init* .exit*)
        libkdrv_jpg.a(.text* .data* .data1* .rodata* .rodata1* .reginfo* .init* .exit*)
        libnvt_tse.a (.text* .data* .data1* .rodata* .rodata1* .reginfo* .init* .exit*)
#endif
        /* audio */
#if (_PACKAGE_AUDIO_)
        libkflow_audiocap.a(.text* .data* .data1* .rodata* .rodata1* .reginfo* .init* .exit*)
        libkflow_audioenc.a(.text* .data* .data1* .rodata* .rodata1* .reginfo* .init* .exit*)
        libnvt_audio.a(.text* .data* .data1* .rodata* .rodata1* .reginfo* .init* .exit*)
        libnvt_kdrv_audio.a(.text* .data* .data1* .rodata* .rodata1* .reginfo* .init* .exit*)
#endif
    }

	.SECTION_06 (ALIGN (0x8)): {
        _section_05_size = ABSOLUTE(.) - _section_05_addr;
        _section_06_addr = ABSOLUTE(.);
    }

    .SECTION_07 (ALIGN (0x8)):
    {
        _section_06_size = ABSOLUTE(.) - _section_06_addr;
        _section_07_addr = ABSOLUTE(.);
    }

    .SECTION_08 (ALIGN (0x8)):
    {
        _section_07_size = ABSOLUTE(.) - _section_07_addr;
        _section_08_addr = ABSOLUTE(.);
    }

    .SECTION_09 (ALIGN (0x8)):
    {
        _section_08_size = ABSOLUTE(.) - _section_08_addr;
        _section_09_addr = ABSOLUTE(.);
    }

    .SECTION_10 (ALIGN (0x8)):
    {
    	_section_09_size = ABSOLUTE(.) - _section_09_addr;
        _section_10_addr = ABSOLUTE(.);
		KEEP (*(.nvtversioninfo.begin*))
		_section_verinfostart = ABSOLUTE(.);
        KEEP(*(.version.info.*))
		_section_verinfoend = ABSOLUTE(.);
        KEEP (*(.nvtversioninfo.end*))
        *(.text*)
        *(.gnu.linkonce.t.*)
        *(.gnu_warning)
        *(.glue_7)
        *(.glue_7t)
        *(.vfp11_veneer)
        *(.ARM.extab.*)
        *(.gnu.linkonce.armextab.*)

        __rodata_start = .;
        *(.rodata)
        *(.rodata.*)
        *(.gnu.linkonce.r.*)
        __rodata_end = .;

        __rodata1_start = .;
        *(.rodata1)
        *(.rodata1.*)
        __rodata1_end = .;

        __sdata2_start = .;
        *(.sdata2)
        *(.sdata2.*)
        *(.gnu.linkonce.s2.*)
        __sdata2_end = .;

        __data_start = .;
        _load_general_var_base = .;
    	_image_general_var_base = .;
        *(.data)
        *(.data.*)
        _image_general_var_limit = .;
        __data_end = .;

        __data1_start = .;
        *(.data1)
        *(.data1.*)
        *(.gnu.linkonce.d.*)
        __data1_end = .;

        __sdata_start = .;
        *(.sdata)
        *(.sdata.*)
        *(.gnu.linkonce.s.*)
        __sdata_end = .;
    }

    _section_10_size = ABSOLUTE(.) - _section_10_addr;
	_etext = ABSOLUTE(.);

    . = ALIGN(8);
	/* collect zero initialized global data (mark "B" in sym file) */
    /* collect static local data (mark "b" in sym file) */
    _section_zi_addr = ABSOLUTE(.);
    _image_general_zi_zi_base = ABSOLUTE(.);

    .bss (NOLOAD) : {
        __bss_start = .;
        *(.bss)
        *(.bss.*)
        *(.gnu.linkonce.b.*)
        *(COMMON)
        __bss_end = .;
    }

	/* stack area start for RTOS tasks */
    _image_general_zi_zi_limit = ABSOLUTE(.);

#if !defined(__aarch64__)
    .ttb (NOLOAD) : {
        . = ALIGN(0x4000);
        _ttb = .;
        . = . + 0x4000;
    }
#endif

    /* Generate Stack and Heap definitions */
#if defined(__aarch64__)
    .stack (NOLOAD) : {
        _stack_end = .;
        . = ALIGN(64);
        _el3_stack_end = .;
        . += _EL3_STACK_SIZE;
        __el3_stack = .;
        _el2_stack_end = .;
        . += _EL2_STACK_SIZE;
        . = ALIGN(64);
        __el2_stack = .;
        _el1_stack_end = .;
        . += _EL1_STACK_SIZE;
        . = ALIGN(64);
        __el1_stack = .;
        . = ALIGN(64);
        _rtos_heap_start = .;
        . += _RTOS_HEAP_SIZE;
        _rtos_heap_end = .;
         _stack = .;
    }
#else
    .stack (NOLOAD) : {
        _stack_end = .;
        . = ALIGN(16);
        _sys_stack_end = .;
        . += _SYS_STACK_SIZE;
        . = ALIGN(16);
        __sys_stack = .;
        . = ALIGN(16);
        _irq_stack_end = .;
        . += _IRQ_STACK_SIZE;
        . = ALIGN(16);
        __irq_stack = .;
        _supervisor_stack_end = .;
        . += _SUPERVISOR_STACK_SIZE;
        . = ALIGN(16);
        __supervisor_stack = .;
        _abort_stack_end = .;
        . += _ABORT_STACK_SIZE;
        . = ALIGN(16);
        __abort_stack = .;
        _fiq_stack_end = .;
        . += _FIQ_STACK_SIZE;
        . = ALIGN(16);
        __fiq_stack = .;
        _undef_stack_end = .;
        . += _UNDEF_STACK_SIZE;
        . = ALIGN(16);
        __undef_stack = .;
        . = ALIGN(16);
        _rtos_heap_start = .;
        . += _RTOS_HEAP_SIZE;
        _rtos_heap_end = .;
        _stack = .;
    }
#endif
    .libc_heap (NOLOAD) : {
        . = ALIGN(64); /* 64 bytes alignment for cache issue */
        _libc_heap_start = .; /* LibC's heap is from here to end of rtos memory */
    }

    end = .;
}

/* libkdrv_videocapture.a is for sensor_register_rate out of section range */
GROUP(libgcc.a libc.a libdrv_portable.a libfreertos.a libkwrap.a libkdrv_comm.a libnvt_sif.a libkdrv_top.a libkdrv_videocapture.a libkflow_common.a libker_pkg.a)

EXTERN("_init", "usage_cmd_showhelp")

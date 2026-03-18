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

/* _RTOS_HEAP_SIZE: for FreeRTOS's heap-method5, only be used on FreeRTOS-Kernel */
_RTOS_HEAP_SIZE = DEFINED(_RTOS_HEAP_SIZE) ? _RTOS_HEAP_SIZE : 0xA0000;

/* Following Stacks are used for each CPU modes on boot.S */
_STACK_SIZE = DEFINED(_STACK_SIZE) ? _STACK_SIZE : 0xA000;
_ABORT_STACK_SIZE = DEFINED(_ABORT_STACK_SIZE) ? _ABORT_STACK_SIZE : 1024;
_SUPERVISOR_STACK_SIZE = DEFINED(_SUPERVISOR_STACK_SIZE) ? _SUPERVISOR_STACK_SIZE : 2048;
_IRQ_STACK_SIZE = DEFINED(_IRQ_STACK_SIZE) ? _IRQ_STACK_SIZE : 1024;
_FIQ_STACK_SIZE = DEFINED(_FIQ_STACK_SIZE) ? _FIQ_STACK_SIZE : 1024;
_UNDEF_STACK_SIZE = DEFINED(_UNDEF_STACK_SIZE) ? _UNDEF_STACK_SIZE : 1024;

OUTPUT_ARCH(arm)
ENTRY(_vector_table)
SECTIONS
{
    .START_UP $START_UP: {
        _section_01_addr = ABSOLUTE(.);
        KEEP (*(.vectors))
    }
	.BIN_INFO $BIN_INFO: {
        KEEP(./startup/bin_info.o(.data*));
    }
    .CODE_INFO $CODE_INFO: {
		KEEP(./startup/code_info.o(.data*));
		_load_core2_entry_program_start_base = .;
		KEEP(./startup/smp_entry.o(.smp*));
		KEEP(./startup/smp_entry.o(.data*));
		_load_core2_entry_program_end_base = .;
    }

	.DOCHG_DMA_CLK 0xF07C0000:
    AT (LOADADDR(.CODE_INFO) + SIZEOF(.CODE_INFO))
    {
		_ram_change_dma_clock_start = LOADADDR(.CODE_INFO) + SIZEOF(.CODE_INFO);
		_load_change_dma_clock_start = ABSOLUTE(.);
		KEEP(./startup/clock_api.o(.vtrs* .text* .data*));
		KEEP(./startup/power_down_isr.o(.text* .data*));
		_load_change_dma_clock_base = ABSOLUTE(.);
		*(.chgdma_data*) /* collect variables used in change DMA flow */
    }

    .DUMMY_ALIGN LOADADDR(.CODE_INFO) + SIZEOF(.CODE_INFO) + SIZEOF(.DOCHG_DMA_CLK):
    AT (LOADADDR(.CODE_INFO) + SIZEOF(.CODE_INFO) + SIZEOF(.DOCHG_DMA_CLK)) {
        . = . + 1;
        . = ALIGN(64);
    }

    _stext = .;

	.ctors LOADADDR(.CODE_INFO) + SIZEOF(.CODE_INFO) + SIZEOF(.DOCHG_DMA_CLK):
	AT (LOADADDR(.CODE_INFO) + SIZEOF(.CODE_INFO) + SIZEOF(.DOCHG_DMA_CLK))
    {
        __CTOR_LIST__ = .;
        KEEP (*(SORT(.ctors.*)))
        KEEP (*(.ctors))
        KEEP (*(SORT(.init_array.*)))
        KEEP (*(.init_array))
        __CTOR_END__ = .;
    }

    .dtors ALIGN (0x4) : {
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

    .devtab ALIGN (0x4) : {
        __MAINFUNCTAB_BEGIN__ = .;
        . = .; KEEP(*( SORT (.cmdsys.table.*))) ;
        __MAINFUNCTAB_END__ = .;
        __EXAMFUNCTAB_BEGIN__ = .;
        . = .; KEEP(*( SORT (.examsys.table.*))) ;
        __EXAMFUNCTAB_END__ = .;
    }

    . = ALIGN(4);
    .SECTION_01 :
    {
        *(.boot)
        *(.gcc_except_table)
        *(.newlib)
        *(.jcr)
        /* app */
        ./startup/startup.o (.text* .data* .data1* .rodata* .rodata1* .reginfo* .init* .exit*)
        ./rtos-main.o (.text* .data* .data1* .rodata* .rodata1* .reginfo* .init* .exit*)
        ./system/sys_fdt.o (.text* .data* .data1* .rodata* .rodata1* .reginfo* .init* .exit*)
        ./system/sys_fwload.o (.text* .data* .data1* .rodata* .rodata1* .reginfo* .init* .exit*)
        ./system/sys_fastboot.o (.text* .data* .data1* .rodata* .rodata1* .reginfo* .init* .exit*)
        ./system/sys_storage_partition.o (.text* .data* .data1* .rodata* .rodata1* .reginfo* .init* .exit*)
        ./flow/flow_uvc.o (.text* .data* .data1* .rodata* .rodata1* .reginfo* .init* .exit*)
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
        libkdrv_comm.a (.text* .data* .data1* .rodata* .rodata1* .reginfo* .init* .exit* .init* .exit*)
        libkdrv_cpu.a (.text* .data* .data1* .rodata* .rodata1* .reginfo* .init* .exit*)
        libkdrv_rtosfdt.a (.text* .data* .data1* .rodata* .rodata1* .reginfo* .init* .exit*)
        libkdrv_top.a (.text* .data* .data1* .rodata* .rodata1* .reginfo* .init* .exit*)
        libkdrv_pll.a (.text* .data* .data1* .rodata* .rodata1* .reginfo* .init* .exit*)
        libkdrv_gpio.a (.text* .data* .data1* .rodata* .rodata1* .reginfo* .init* .exit*)
        libkdrv_adc.a (.text* .data* .data1* .rodata* .rodata1* .reginfo* .init* .exit*)
        libkdrv_interrupt.a (.text* .data* .data1* .rodata* .rodata1* .reginfo* .init* .exit*)
		libkdrv_hwcopy.a (.text* .data* .data1* .rodata* .rodata1* .reginfo* .init* .exit*)
        /* os */
        ./startup/bin_info.o(.got*)
        libkwrap.a (.text.* .rodata.* .text* .data* .data1* .rodata* .rodata1* .reginfo* .init* .exit*)
        libvos.a (.text.* .rodata.* .text* .data* .data1* .rodata* .rodata1* .reginfo* .init* .exit*)
        /* freertos.a must be located after kwrap.a for linking text.xEventGroup* */
        libfreertos.a (.text* .data* .data1* .rodata* .rodata1* .reginfo* .init* .exit*)
        /* std-c */
        libc.a (.text* .data* .data1* .rodata* .rodata1* .reginfo* .init* .exit*)
        libgcc.a (.text* .data* .data1* .rodata* .rodata1* .reginfo* .init* .exit*)
        libstdc++.a (.text* .data* .data1* .rodata* .rodata1* .reginfo* .init* .exit*)
    }

    . = ALIGN(4);
    .SECTION_02 :
    {
        _section_01_size = ABSOLUTE(.) - _section_01_addr;
        _section_02_addr = ABSOLUTE(.);
        /* fdt */
        _fdt_sensor = ABSOLUTE(.);
        KEEP(./dts/sensor.o (.sensor*));
        /* ext_drivers */
        $SENSOR1.a(.text* .data* .data1* .rodata* .rodata1* .reginfo* .init* .exit*)
        /* kflow */
        libkflow_videocapture.a (.text* .data* .data1* .rodata* .rodata1* .reginfo* .init* .exit*)
        /* kdrv */
        libkdrv_videocapture.a (.text* .data* .data1* .rodata* .rodata1* .reginfo* .init* .exit*)
        libvendor_media.a (.text* .data* .data1* .rodata* .rodata1* .reginfo* .init* .exit*)
        /* IO */
        libkdrv_i2c.a (.text* .data* .data1* .rodata* .rodata1* .reginfo* .init* .exit*)
    }

    . = ALIGN(4);
    .SECTION_03 :
    {
        _section_02_size = ABSOLUTE(.) - _section_02_addr;
        _section_03_addr = ABSOLUTE(.);
        /* app */
        ./system/sys_nvtmpp.o (.text* .data* .data1* .rodata* .rodata1* .reginfo* .init* .exit*)
        ./system/sys_mempool.o (.text* .data* .data1* .rodata* .rodata1* .reginfo* .init* .exit*)
        /* hdal */
        libhdal.a(.text* .data* .data1* .rodata* .rodata1* .reginfo* .init* .exit*)
#if (_PACKAGE_VIDEO_)
        /* kflow */
        libkflow_common.a (.text* .data* .data1* .rodata* .rodata1* .reginfo* .init* .exit*)
        libkflow_videoprocess.a (.text* .data* .data1* .rodata* .rodata1* .reginfo* .init* .exit*)
        libnvt_videosprite.a (.text* .data* .data1* .rodata* .rodata1* .reginfo* .init* .exit*)
        libnvt_gfx.a (.text* .data* .data1* .rodata* .rodata1* .reginfo* .init* .exit*)
        /* kdrv */
        libkdrv_videocapture.a (.text* .data* .data1* .rodata* .rodata1* .reginfo* .init* .exit*)
        libkdrv_videoprocess.a (.text* .data* .data1* .rodata* .rodata1* .reginfo* .init* .exit*)
        libkdrv_gfx2d.a (.text* .data* .data1* .rodata* .rodata1* .reginfo* .init* .exit*)
        libkdrv_builtin.a (.text* .data* .data1* .rodata* .rodata1* .reginfo* .init* .exit*)
        /* ISP */
        libnvt_ae.a (.text* .data* .data1* .rodata* .rodata1* .reginfo* .init* .exit*)
        libnvt_awb.a (.text* .data* .data1* .rodata* .rodata1* .reginfo* .init* .exit*)
        libnvt_iq.a (.text* .data* .data1* .rodata* .rodata1* .reginfo* .init* .exit*)
        libnvt_isp.a (.text* .data* .data1* .rodata* .rodata1* .reginfo* .init* .exit*)
#endif
    }

    . = ALIGN(4);
    .SECTION_04 :
    {
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
    }

    . = ALIGN(4);
    .SECTION_05 :
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
        libaud_aec.a(.text* .data* .data1* .rodata* .rodata1* .reginfo* .init* .exit*)
        libaud_aace.a(.text* .data* .data1* .rodata* .rodata1* .reginfo* .init* .exit*)
        libaudlib_IIR.a(.text* .data* .data1* .rodata* .rodata1* .reginfo* .init* .exit*)
#endif
    }

    . = ALIGN(4);
    .SECTION_06 :
    {
        _section_05_size = ABSOLUTE(.) - _section_05_addr;
        _section_06_addr = ABSOLUTE(.);
    }

    . = ALIGN(4);
    .SECTION_07 :
    {
        _section_06_size = ABSOLUTE(.) - _section_06_addr;
        _section_07_addr = ABSOLUTE(.);
    }

    . = ALIGN(4);
    .SECTION_08 :
    {
        _section_07_size = ABSOLUTE(.) - _section_07_addr;
        _section_08_addr = ABSOLUTE(.);
    }

    . = ALIGN(4);
    .SECTION_09 :
    {
        _section_08_size = ABSOLUTE(.) - _section_08_addr;
        _section_09_addr = ABSOLUTE(.);
    }

    . = ALIGN(4);
    .SECTION_10 :
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
        *(.got)
        *(.got.plt)

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

    .ttb (NOLOAD) : {
        . = ALIGN(0x4000);
        _ttb = .;
        . = . + 0x4000;
    }

    /* Generate Stack and Heap definitions */
    .stack (NOLOAD) : {
        . = ALIGN(16);
        _stack_end = .;
        . += _STACK_SIZE;
        . = ALIGN(16);
        _stack = .;
        __stack = _stack;
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
    }

    .rtos_heap (NOLOAD) : {
        . = ALIGN(16);
        _rtos_heap_start = .;
        . += _RTOS_HEAP_SIZE;
        _rtos_heap_end = .;
    }

    .libc_heap (NOLOAD) : {
        . = ALIGN(64); /* 64 bytes alignment for cache issue */
        _libc_heap_start = .; /* LibC's heap is from here to end of rtos memory */
    }

    end = .;
}

/* libkdrv_videocapture.a is for sensor_register_rate out of section range */
GROUP(libgcc.a libc.a libdrv_portable.a libfreertos.a libkwrap.a libkdrv_comm.a libnvt_sif.a libkdrv_top.a libkdrv_videocapture.a libkflow_common.a)

EXTERN("_init")

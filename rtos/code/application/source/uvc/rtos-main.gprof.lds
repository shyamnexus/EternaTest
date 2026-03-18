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
_RTOS_HEAP_SIZE = DEFINED(_RTOS_HEAP_SIZE) ? _RTOS_HEAP_SIZE : 0x100000;

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
        KEEP(./SrcCode/Startup/bin_info.o(.data*));
    }
    .CODE_INFO $CODE_INFO: {
        KEEP(./SrcCode/Startup/code_info.o(.data*));
    }

    . = ALIGN(4);

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
	_stext = ABSOLUTE(.);

    .ctors ALIGN (0x4) :
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

	.boot : {	
		_fdt_sensor = ABSOLUTE(.);
        KEEP(./dts/sensor.o (.sensor*));
        libdrv_portable.a (.text*)
	}
    .text :
    {	
        _ftext = . ; 
        /*============================ before os start ===============================  */
		_section_01_size = ABSOLUTE(.) - _section_01_addr;
        _section_02_addr = ABSOLUTE(.);
    	_section_02_size = ABSOLUTE(.) - _section_02_addr;
        _section_03_addr = ABSOLUTE(.);
    	_section_03_size = ABSOLUTE(.) - _section_03_addr;
        _section_04_addr = ABSOLUTE(.);
    	_section_04_size = ABSOLUTE(.) - _section_04_addr;
        _section_05_addr = ABSOLUTE(.);
    	_section_05_size = ABSOLUTE(.) - _section_05_addr;
        _section_06_addr = ABSOLUTE(.);
    	_section_06_size = ABSOLUTE(.) - _section_06_addr;
        _section_07_addr = ABSOLUTE(.);
    	_section_07_size = ABSOLUTE(.) - _section_07_addr;
        _section_08_addr = ABSOLUTE(.);
    	_section_08_size = ABSOLUTE(.) - _section_08_addr;
        _section_09_addr = ABSOLUTE(.);
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

    .ttb (NOLOAD) : {
        . = ALIGN(0x4000);
        _ttb = .;
        . = . + 0x4000;
    }

    . = ALIGN(4);
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

GROUP(libgcc.a libc.a libdrv_portable.a libfreertos.a libkwrap.a libkdrv_comm.a libnvt_sif.a libkdrv_top.a libkflow_common.a)
EXTERN("_init")

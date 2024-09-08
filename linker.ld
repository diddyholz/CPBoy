STARTUP(crt0.o);
ENTRY(_start);

SECTIONS {
	start_address = 0x8CFE6000;
	.bootstrap.start start_address : AT(start_address) {
		*(.bootstrap.start*)
	}
	info_address = 0x8CFE6010;
	. = info_address;
	.hollyhock_name : {
		KEEP(*(.hollyhock_name))
	}
	.hollyhock_description : {
		KEEP(*(.hollyhock_description))
	}
	.hollyhock_author : {
		KEEP(*(.hollyhock_author))
	}
	.hollyhock_version : {
		KEEP(*(.hollyhock_version))
	}

	.text : {
		*(.text*)
		*(.rodata*)
	}

	.data : {
		*(.data*)
	}

	_edata = .;
	PROVIDE(edata = .);

	.bss : {
		*(.bss*)
		*(COMMON)
	}

	_end = .;
	PROVIDE(end = .);
	
	x_mem_addr = 0xE5007000;
	. = x_mem_addr;

	.oc_mem.x : {
		*(.oc_mem.x)
	}
	
	y_mem_addr = 0xE5017000;
	. = y_mem_addr;

	.oc_mem.y : {
		*(.oc_mem.y.dma)
		*(.oc_mem.y.text)
		*(.oc_mem.y.data)
	}
	
	il_mem_addr = 0xE5200000;
	. = il_mem_addr;

	.oc_mem.il : {
		*(.oc_mem.il.data)
		*(.oc_mem.il.text)
	}
}

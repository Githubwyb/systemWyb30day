ENTRY(start_kernel)

SECTIONS {
    . = 0x00280000;
	.text :  {
		*(.text.first)
        *(.text)
	}
}

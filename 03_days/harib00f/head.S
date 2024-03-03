; head.S bootloader加载软盘后，从此汇编作为入口
; TAB=4

	ORG		0xc400	; 软盘中此文件在0x4400，拷贝软盘整体从0x8000开始，计算的此文件在0xc400
fin:
	HLT
	JMP		fin

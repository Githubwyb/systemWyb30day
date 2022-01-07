; haribote-os
; TAB=4

		ORG		0xc200			; ?个程序要被装?到内存的地址

		MOV		AL,0x13			; VGA??，320x200x8位彩色
		MOV		AH,0x00
		INT		0x10
fin:
		HLT
		JMP		fin

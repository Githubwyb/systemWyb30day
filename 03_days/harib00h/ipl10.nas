; helloOS
; TAB=4

CYLS    EQU     10          ; è?CYLS?10

    ORG     0x7c00          ; w¾öI?n¬

; ÈºI?qp°?yFAT12i®I??
    JMP     entry
    DB      0x90
    DB      "HELLOIPL"      ; ??æI¼ÌÂÈ¥CÓIø
    DW      512             ; ?¢îæisectorjIå¬iK??512?j
    DB      1               ; âÆiclusterjIå¬iK??1¢îæj
    DW      1               ; FATINnÊuiêÊ¸æê¢îæ?nj
    DB      2               ; FATI¢iK??2j
    DW      224             ; ªÚ?Iå¬iêÊ?¬224?j
    DW      2880            ; ?¥?Iå¬iK?¥2880îæj
    DB      0xf0            ; ¥?I??
    DW      9               ; FATI?xiK?¥9îæj
    DW      18              ; 1¢¥¹itrackjL{¢îæiK?¥18j
    DW      2               ; ¥?iK?¥2j
    DD      0               ; sgpªæCK?¥0
    DD      2880            ; dÊê¥?å¬
    DB      0, 0, 0x29      ; Ó?s¾CÅè
    DD      0xffffffff      ; iÂ\¥jÉ??
    DB      "HARIBOTEOS "   ; ¥?I¼Ìi11?j
    DB      "FAT12   "      ; ¥?i®¼Ìi8?j
    RESB    18              ; æóo18?

; öjS
entry:
    MOV     AX, 0           ; n»ñ¶í
    MOV     SS, AX
    MOV     SP, 0x7c00
    MOV     DS, AX

; ?¥?
    MOV     AX, 0x0820
    MOV     ES, AX
    MOV     CH, 0           ; Ê0
    MOV     DH, 0           ; ¥?0
    MOV     CL, 2           ; îæ2
readloop:
    MOV     SI, 0           ; ??¸?
retry:
    MOV     AH, 0x02        ; AH = 0x02: ??
    MOV     AL, 1           ; 1¢îæ
    MOV     BX, 0
    MOV     DL, 0x00        ; A??í
    INT     0x13            ; ?p¥?BIOS
    JNC     next            ; vo?µ?next
    ADD     SI, 1           ; SIÁ1
    CMP     SI, 5           ; ä?SIa5
    JAE     error           ; SI >= 5Cµ?error
    MOV     AH, 0x00
    MOV     DL, 0x00        ; A??í
    INT     0x13            ; du??í
    JMP     retry
next:
    MOV     AX, ES          ; à¶n¬@Ú0x200
    ADD     AX, 0x0020      ; 0x0020 = 512 / 16
    MOV     ES, AX          ; Ù@¼ÚESÁ0x0020CpAXñ¶í??
    ADD     CL, 1           ; CLÁ1
    CMP     CL, 18          ; ä?CL^18
    JBE     readloop        ; CL <= 18Cµ?readloop
    MOV     CL, 1
    ADD     DH, 1
    CMP     DH, 2
    JB      readloop        ; DH < 2Cµ?readloop
    MOV     DH, 0
    ADD     CH, 1
    CMP     CH, CYLS
    JB      readloop        ; CH < 10Cµ?readloopC10¢Ê

; ?Ýäß???¹C?s haribote.sysI
	MOV		[0x0ff0],CH		; Ó IPL ?¹½?
	JMP		0xc200

error:
    MOV     SI, msg

putloop:
    MOV     AL, [SI]
    ADD     SI, 1           ; ?SIÁ1
    CMP     AL, 0
    JE      fin
    MOV     AH, 0x0e        ; ?¦ê¢¶
    MOV     BX, 15          ; wè?F
    INT     0x10            ; ?p??BIOS
    JMP     putloop
fin:
    HLT                     ; ?CPUâ~CÒwß
    JMP     fin             ; ÙÀz?
msg:
    DB      0x0a, 0x0a      ; ?s?
    DB      "Load error"
    DB      0x0a            ; ?s
    DB      0

    RESB    0x7dfe-$        ; UÊ0x00C¼0x7dfe

    DB      0x55, 0xaa

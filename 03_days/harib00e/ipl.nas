; helloOS
; TAB=4

CYLS    EQU     10          ; ่?CYLS?10

    ORG     0x7c00          ; wพ๖I?nฌ

; ศบI?qpฐ?yFAT12iฎI??
    JMP     entry
    DB      0x90
    DB      "HELLOIPL"      ; ??ๆIผฬยศฅCำI๘
    DW      512             ; ?ข๎ๆisectorjIๅฌiK??512?j
    DB      1               ; โฦiclusterjIๅฌiK??1ข๎ๆj
    DW      1               ; FATINnสui๊สธๆ๊ข๎ๆ?nj
    DB      2               ; FATIขiK??2j
    DW      224             ; ชฺ?Iๅฌi๊ส?ฌ224?j
    DW      2880            ; ?ฅ?IๅฌiK?ฅ2880๎ๆj
    DB      0xf0            ; ฅ?I??
    DW      9               ; FATI?xiK?ฅ9๎ๆj
    DW      18              ; 1ขฅนitrackjL{ข๎ๆiK?ฅ18j
    DW      2               ; ฅ?iK?ฅ2j
    DD      0               ; sgpชๆCK?ฅ0
    DD      2880            ; dส๊ฅ?ๅฌ
    DB      0, 0, 0x29      ; ำ?sพCล่
    DD      0xffffffff      ; iย\ฅjษ??
    DB      "HARIBOTEOS "   ; ฅ?Iผฬi11?j
    DB      "FAT12   "      ; ฅ?iฎผฬi8?j
    RESB    18              ; ๆ๓o18?

; ๖jS
entry:
    MOV     AX, 0           ; nป๑ถํ
    MOV     SS, AX
    MOV     SP, 0x7c00
    MOV     DS, AX

; ?ฅ?
    MOV     AX, 0x0820
    MOV     ES, AX
    MOV     CH, 0           ; ส0
    MOV     DH, 0           ; ฅ?0
    MOV     CL, 2           ; ๎ๆ2
readloop:
    MOV     SI, 0           ; ??ธ?
retry:
    MOV     AH, 0x02        ; AH = 0x02: ??
    MOV     AL, 1           ; 1ข๎ๆ
    MOV     BX, 0
    MOV     DL, 0x00        ; A??ํ
    INT     0x13            ; ?pฅ?BIOS
    JNC     next            ; vo?ต?next
    ADD     SI, 1           ; SIม1
    CMP     SI, 5           ; ไ?SIa5
    JAE     error           ; SI >= 5Cต?error
    MOV     AH, 0x00
    MOV     DL, 0x00        ; A??ํ
    INT     0x13            ; du??ํ
    JMP     retry
next:
    MOV     AX, ES          ; เถnฌ@ฺ0x200
    ADD     AX, 0x0020      ; 0x0020 = 512 / 16
    MOV     ES, AX          ; ู@ผฺESม0x0020CpAX๑ถํ??
    ADD     CL, 1           ; CLม1
    CMP     CL, 18          ; ไ?CL^18
    JBE     readloop        ; CL <= 18Cต?readloop
    MOV     CL, 1
    ADD     DH, 1
    CMP     DH, 2
    JB      readloop        ; DH < 2Cต?readloop
    MOV     DH, 0
    ADD     CH, 1
    CMP     CH, CYLS
    JB      readloop        ; CH < 10Cต?readloopC10ขส

; ?ๆฌ๗CPU?
fin:
    HLT                     ; ?CPUโ~Cาw฿
    JMP     fin             ; ูภz?

error:
    MOV     SI, msg

putloop:
    MOV     AL, [SI]
    ADD     SI, 1           ; ?SIม1
    CMP     AL, 0
    JE      fin
    MOV     AH, 0x0e        ; ?ฆ๊ขถ
    MOV     BX, 15          ; w่?F
    INT     0x10            ; ?p??BIOS
    JMP     putloop

msg:
    DB      0x0a, 0x0a      ; ?s?
    DB      "Load error"
    DB      0x0a            ; ?s
    DB      0

    RESB    0x7dfe-$        ; Uส0x00Cผ0x7dfe

    DB      0x55, 0xaa

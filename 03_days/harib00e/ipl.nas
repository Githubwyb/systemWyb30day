; helloOS
; TAB=4

CYLS    EQU     10          ; 定?CYLS?10

    ORG     0x7c00          ; 指明程序的装?地址

; 以下的?述用于?准FAT12格式的??
    JMP     entry
    DB      0x90
    DB      "HELLOIPL"      ; ??区的名称可以是任意的字符串
    DW      512             ; ?个扇区（sector）的大小（必??512字?）
    DB      1               ; 簇（cluster）的大小（必??1个扇区）
    DW      1               ; FAT的起始位置（一般从第一个扇区?始）
    DB      2               ; FAT的个数（必??2）
    DW      224             ; 根目?的大小（一般?成224?）
    DW      2880            ; ?磁?的大小（必?是2880扇区）
    DB      0xf0            ; 磁?的??
    DW      9               ; FAT的?度（必?是9扇区）
    DW      18              ; 1个磁道（track）有几个扇区（必?是18）
    DW      2               ; 磁?数（必?是2）
    DD      0               ; 不使用分区，必?是0
    DD      2880            ; 重写一次磁?大小
    DB      0, 0, 0x29      ; 意?不明，固定
    DD      0xffffffff      ; （可能是）卷?号?
    DB      "HARIBOTEOS "   ; 磁?的名称（11字?）
    DB      "FAT12   "      ; 磁?格式名称（8字?）
    RESB    18              ; 先空出18字?

; 程序核心
entry:
    MOV     AX, 0           ; 初始化寄存器
    MOV     SS, AX
    MOV     SP, 0x7c00
    MOV     DS, AX

; ?磁?
    MOV     AX, 0x0820
    MOV     ES, AX
    MOV     CH, 0           ; 柱面0
    MOV     DH, 0           ; 磁?0
    MOV     CL, 2           ; 扇区2
readloop:
    MOV     SI, 0           ; ??失?次数
retry:
    MOV     AH, 0x02        ; AH = 0x02: ??
    MOV     AL, 1           ; 1个扇区
    MOV     BX, 0
    MOV     DL, 0x00        ; A??器
    INT     0x13            ; ?用磁?BIOS
    JNC     next            ; 没出?跳?next
    ADD     SI, 1           ; SI加1
    CMP     SI, 5           ; 比?SI和5
    JAE     error           ; SI >= 5，跳?到error
    MOV     AH, 0x00
    MOV     DL, 0x00        ; A??器
    INT     0x13            ; 重置??器
    JMP     retry
next:
    MOV     AX, ES          ; 内存地址后移0x200
    ADD     AX, 0x0020      ; 0x0020 = 512 / 16
    MOV     ES, AX          ; 无法直接ES加0x0020，利用AX寄存器??
    ADD     CL, 1           ; CL加1
    CMP     CL, 18          ; 比?CL与18
    JBE     readloop        ; CL <= 18，跳?到readloop
    MOV     CL, 1
    ADD     DH, 1
    CMP     DH, 2
    JB      readloop        ; DH < 2，跳?readloop
    MOV     DH, 0
    ADD     CH, 1
    CMP     CH, CYLS
    JB      readloop        ; CH < 10，跳?readloop，10个柱面

; ?取成功CPU睡?
fin:
    HLT                     ; ?CPU停止，等待指令
    JMP     fin             ; 无限循?

error:
    MOV     SI, msg

putloop:
    MOV     AL, [SI]
    ADD     SI, 1           ; ?SI加1
    CMP     AL, 0
    JE      fin
    MOV     AH, 0x0e        ; ?示一个文字
    MOV     BX, 15          ; 指定字符?色
    INT     0x10            ; ?用??BIOS
    JMP     putloop

msg:
    DB      0x0a, 0x0a      ; ?行?次
    DB      "Load error"
    DB      0x0a            ; ?行
    DB      0

    RESB    0x7dfe-$        ; 填写0x00，直到0x7dfe

    DB      0x55, 0xaa

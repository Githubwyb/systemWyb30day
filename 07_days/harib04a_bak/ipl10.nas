; helloOS
; TAB=4

CYLS    EQU     10          ; 定义CYLS为10

    ORG     0x7c00          ; 指明程序的装载地址

; 以下的记述用于标准FAT12格式的软盘
    JMP     entry
    DB      0x90
    DB      "HELLOIPL"      ; 启动区的名称可以是任意的字符串
    DW      512             ; 每个扇区（sector）的大小（必须为512字节）
    DB      1               ; 簇（cluster）的大小（必须为1个扇区）
    DW      1               ; FAT的起始位置（一般从第一个扇区开始）
    DB      2               ; FAT的个数（必须为2）
    DW      224             ; 根目录的大小（一般设成224项）
    DW      2880            ; 该磁盘的大小（必须是2880扇区）
    DB      0xf0            ; 磁盘的种类
    DW      9               ; FAT的长度（必须是9扇区）
    DW      18              ; 1个磁道（track）有几个扇区（必须是18）
    DW      2               ; 磁头数（必须是2）
    DD      0               ; 不使用分区，必须是0
    DD      2880            ; 重写一次磁盘大小
    DB      0, 0, 0x29      ; 意义不明，固定
    DD      0xffffffff      ; （可能是）卷标号码
    DB      "HARIBOTEOS "   ; 磁盘的名称（11字节）
    DB      "FAT12   "      ; 磁盘格式名称（8字节）
    RESB    18              ; 先空出18字节

; 程序核心
entry:
    MOV     AX, 0           ; 初始化寄存器
    MOV     SS, AX
    MOV     SP, 0x7c00
    MOV     DS, AX

; 读磁盘
    MOV     AX, 0x0820
    MOV     ES, AX
    MOV     CH, 0           ; 柱面0
    MOV     DH, 0           ; 磁头0
    MOV     CL, 2           ; 扇区2
readloop:
    MOV     SI, 0           ; 记录失败次数
retry:
    MOV     AH, 0x02        ; AH = 0x02: 读盘
    MOV     AL, 1           ; 1个扇区
    MOV     BX, 0
    MOV     DL, 0x00        ; A驱动器
    INT     0x13            ; 调用磁盘BIOS
    JNC     next            ; 没出错跳转next
    ADD     SI, 1           ; SI加1
    CMP     SI, 5           ; 比较SI和5
    JAE     error           ; SI >= 5，跳转到error
    MOV     AH, 0x00
    MOV     DL, 0x00        ; A驱动器
    INT     0x13            ; 重置驱动器
    JMP     retry
next:
    MOV     AX, ES          ; 内存地址后移0x200
    ADD     AX, 0x0020      ; 0x0020 = 512 / 16
    MOV     ES, AX          ; 无法直接ES加0x0020，利用AX寄存器实现
    ADD     CL, 1           ; CL加1
    CMP     CL, 18          ; 比较CL与18
    JBE     readloop        ; CL <= 18，跳转到readloop
    MOV     CL, 1
    ADD     DH, 1
    CMP     DH, 2
    JB      readloop        ; DH < 2，跳转readloop
    MOV     DH, 0
    ADD     CH, 1
    CMP     CH, CYLS
    JB      readloop        ; CH < 10，跳转readloop，10个柱面

; 现在我已经阅读了它，运行 haribote.sys！
	MOV		[0x0ff0],CH		; 注意 IPL 读了多远
	JMP		0xc200

error:
    MOV     SI, msg

putloop:
    MOV     AL, [SI]
    ADD     SI, 1           ; 给SI加1
    CMP     AL, 0
    JE      fin
    MOV     AH, 0x0e        ; 显示一个文字
    MOV     BX, 15          ; 指定字符颜色
    INT     0x10            ; 调用显卡BIOS
    JMP     putloop
fin:
    HLT                     ; 让CPU停止，等待指令
    JMP     fin             ; 无限循环
msg:
    DB      0x0a, 0x0a      ; 换行两次
    DB      "Load error"
    DB      0x0a            ; 换行
    DB      0

    RESB    0x7dfe-$        ; 填写0x00，直到0x7dfe

    DB      0x55, 0xaa

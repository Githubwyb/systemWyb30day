TOOLPATH = ../../z_tools

MAKE     = make
NASK     = $(TOOLPATH)/nask.exe
CC1      = $(TOOLPATH)/cc1.exe
GAS2NASK = $(TOOLPATH)/gas2nask.exe -a
OBJ2BIM  = $(TOOLPATH)/obj2bim.exe
MAKEFONT = $(TOOLPATH)/makefont.exe
BIN2OBJ  = $(TOOLPATH)/bin2obj.exe
BIM2HRB  = $(TOOLPATH)/bim2hrb.exe
RULEFILE = $(TOOLPATH)/haribote/haribote.rul
EDIMG    = $(TOOLPATH)/edimg.exe
IMGTOL   = $(TOOLPATH)/imgtol.com
COPY     = cp
DEL      = rm -f

INCLUDE  = -I$(TOOLPATH)/haribote/ -I./include/ -I./arch/x86/include/
CFLAGS   = -D_SIZE_T -DCONFIG_X86_32 -Os -Wall -Werror -quiet

default :
	$(MAKE) run

# 最终生成的img文件
#   将ipl10.bin的512B内容拷贝到磁盘的C0-H0-S1位置，用于启动扇区
haribote.img : ipl10.bin haribote.sys
	$(EDIMG)   imgin:$(TOOLPATH)/fdimg0at.tek \
		wbinimg src:ipl10.bin len:512 from:0 to:0 \
		copy from:haribote.sys to:@: \
		imgout:haribote.img

# bin文件需要使用nas生成
%.bin : %.nas
	$(NASK) $*.nas $*.bin $*.lst

# haribote.sys文件需要c生成hrb和生成的bin
haribote.sys : asmhead.bin bootpack.hrb
	cat asmhead.bin bootpack.hrb > haribote.sys

%.hrb : %.bim
	$(BIM2HRB) $*.bim $*.hrb 0

#下面四个命令通过模式匹配获取当前目录下的所有C文件
SRCDIR = ./ ./lib/
C_SOURCES = $(foreach d,$(SRCDIR),$(wildcard $(d)*.c))
C_OBJS = $(patsubst %.c,%.obj,$(C_SOURCES))
C_NASES = $(patsubst %.c,%.nas,$(C_SOURCES))
C_GASES = $(patsubst %.c,%.gas,$(C_SOURCES))
C_LSTS = $(patsubst %.c,%.lst,$(C_SOURCES))

bootpack.bim : $(C_OBJS) naskfunc.obj hankaku.obj
	$(OBJ2BIM) @$(RULEFILE) out:bootpack.bim stack:3136k map:bootpack.map \
		$^
# 3MB+64KB=3136KB

hankaku.bin : hankaku.txt Makefile
	$(MAKEFONT) hankaku.txt hankaku.bin

hankaku.obj : hankaku.bin Makefile
	$(BIN2OBJ) hankaku.bin hankaku.obj _hankaku

naskfunc.obj : naskfunc.nas
	$(NASK) naskfunc.nas naskfunc.obj naskfunc.lst

%.obj : %.c
	$(CC1) $(INCLUDE) $(CFLAGS) -o $*.gas $*.c
	$(GAS2NASK) $*.gas $*.nas
	$(NASK) $*.nas $*.obj $*.lst

img :
	$(MAKE) haribote.img

run :
	$(MAKE) img
	$(COPY) haribote.img ../../common/haribote.img

install :
	$(MAKE) img
	$(IMGTOL) w a: haribote.img

clean :
	-$(DEL) *.bin
	-$(DEL) *.lst
	-$(DEL) *.obj
	-$(DEL) *.img
	-$(DEL) $(C_NASES)
	-$(DEL) $(C_GASES)
	-$(DEL) $(C_LSTS)
	-$(DEL) $(C_OBJS)
	-$(DEL) bootpack.map
	-$(DEL) bootpack.bim
	-$(DEL) bootpack.hrb
	-$(DEL) haribote.sys

src_only :
	$(MAKE) clean
	-$(DEL) haribote.img

TOOLPATH = ../../z_tools
INCPATH  = $(TOOLPATH)/haribote/

MAKE     = make
NASK     = $(TOOLPATH)/nask.exe
CC1      = $(TOOLPATH)/cc1.exe -I$(INCPATH) -Os -Wall -quiet
GAS2NASK = $(TOOLPATH)/gas2nask.exe -a
OBJ2BIM  = $(TOOLPATH)/obj2bim.exe
BIM2HRB  = $(TOOLPATH)/bim2hrb.exe
RULEFILE = $(TOOLPATH)/haribote/haribote.rul
EDIMG    = $(TOOLPATH)/edimg.exe
IMGTOL   = $(TOOLPATH)/imgtol.com
COPY     = cp
DEL      = rm -f

default :
	$(MAKE) img

# 最终生成的img文件
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
C_SOURCES = $(wildcard $(d)*.c)
C_OBJS = $(patsubst %.c, %.obj, $(C_SOURCES))

bootpack.bim : $(C_OBJS)
	$(OBJ2BIM) @$(RULEFILE) out:bootpack.bim stack:3136k map:bootpack.map \
		$^

%.obj : %.c
	$(CC1) -o $*.gas $*.c
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
	-$(DEL) *.gas
	-$(DEL) *.obj
	-$(DEL) bootpack.nas
	-$(DEL) bootpack.map
	-$(DEL) bootpack.bim
	-$(DEL) bootpack.hrb
	-$(DEL) haribote.sys

src_only :
	$(MAKE) clean
	-$(DEL) haribote.img

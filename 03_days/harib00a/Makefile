TOOLPATH 	= ../z_tools/
MAKE 		= $(TOOLPATH)make.exe -r
NASK 		= $(TOOLPATH)nask.exe
EDIMG 		= $(TOOLPATH)edimg.exe
IMGTOL		= $(TOOLPATH)imgtol.com
COPY 		= copy
DEL 		= del

# 默认生成img并复制到目录

default :
	$(MAKE) run

# 文件生成规则

ipl.bin : ipl.nas Makefile
	$(NASK) ipl.nas ipl.bin ipl.lst

haribote.img : ipl.bin Makefile
	$(EDIMG) imgin:../z_tools/fdimg0at.tek \
		wbinimg src:ipl.bin len:512 from:0 to:0 imgout:haribote.img

# 命令

img :
	$(MAKE) haribote.img

asm :
	$(MAKE) ipl.bin

run :
	$(MAKE) img
	$(COPY) haribote.img ..\..\common\haribote.img

clean :
	-$(DEL) ipl.bin
	-$(DEL) ipl.lst

src_only :
	$(MAKE) clean
	-$(DEL) haribote.img
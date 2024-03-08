#!/bin/bash

kernel_root="/home/wangyubo/work/src/local/archlinux-soft/linux/trunk/src/archlinux-linux"

cur_dir="$(dirname "$0")"
cd "$cur_dir"

# 生成一个filelist
cd kernel_files
find ./ -type f | grep -E "*.c|*.h" > ../filelist.txt
cd ..

# 先生成一个放有原始文件的文件夹
mkdir -p kernel_files_org
while read -r line
do
    mkdir -p kernel_files_org/"$(dirname "$line")"
    cp "$kernel_root/$line" "kernel_files_org/$line"
done < filelist.txt

rm -f filelist.txt

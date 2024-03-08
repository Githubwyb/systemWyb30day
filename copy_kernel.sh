#!/bin/bash
file_path="$1"
kernel_root="/home/wangyubo/work/src/local/archlinux-soft/linux/trunk/src/archlinux-linux"

if [ ! -f "$kernel_root/$file_path" ]; then
    echo "$kernel_root/$file_path is not exist"
    exit 1
fi

cur_dir="$(dirname "$0")"
cd "$cur_dir"

mkdir -p "kernel_files/$(dirname "$file_path")"
cp "$kernel_root/$file_path" "kernel_files/$file_path"

cd -

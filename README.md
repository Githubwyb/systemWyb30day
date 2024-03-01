# systemWyb30day

30天自制操作系统代码

## 一、qemu启动镜像

```shell
qemu-system-x86_64 -enable-kvm -m 4G -smp 1 -fda common/haribote.img
```

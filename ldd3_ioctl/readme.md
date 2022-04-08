```bash
sudo insmod orange.ko

# 查看设备号
cat /proc/devices


mknod /dev/orange c major 0

# 最后记得删除字符设备
```


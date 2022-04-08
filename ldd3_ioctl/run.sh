#! /bin/bash
module="orange"
device="orange"
tmp="ioctl"
mode="666"
group=0

function load(){
	insmod ./$module.ko $* || exit 1

	rm -f /dev/${device}

	major=$(awk "$2==$tmp {print $1}" /proc/devices)
	mknod /dev/${device} c $major 0


	chgrp $group /dev/$device
	chmod $mode /dev/$device
}

function unload() {
	rm -f /dev/${device}[0-2]
	rmmod $module || exit 1
}

arg=${1:-"load"}
case $arg in
       	load)
		load ;;
	unload)
		unload ;;
	reload)
		(unload )
		load
		;;
	*)
		echo "usage: $0 {load | unload | reload}"
		echo "Default is load"
		exit 1
		;;
esac

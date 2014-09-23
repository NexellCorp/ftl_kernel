#!/bin/bash

KDIR_M310=/home/ews/storage/project/kernels/linux-m310/
KDIR_SUNXI=/home/ews/storage/project/kernels/linux-sunxi/
KDIR_NXP4330=/home/ews/storage/project/kernels/linux-nxp4330/
KDIR_X86_64=/home/ews/storage/project/kernels/linux-x86/

OUTDIR_M310=$HOME/storage/project/project-android/+.out/m310
OUTDIR_SUNXI=$HOME/storage/project/project-android/+.out/sunxi
OUTDIR_NXP4330=$HOME/storage/project/project-android/+.out/nxp4330

case $1 in

    "m310")
        echo "m310"
        export ARCH=arm
        export CROSS_COMPILE=arm-none-linux-gnueabi-
        export PATH=/usr/local/arm/arm-2010.09/bin/:$PATH
        export KDIR=$KDIR_M310
        make
        ;;

    "sunxi")
        echo "sunxi"
        export ARCH=arm
        export CROSS_COMPILE=arm-none-linux-gnueabi-
        export PATH=/usr/local/arm/arm-2010.09/bin/:/usr/local/allwinner/bins:$PATH
        export KDIR=$KDIR_SUNXI
        make
        ;;

    "nxp4330" | *)
        echo "nxp4330"
        export ARCH=arm
        export CROSS_COMPILE=arm-cortex_a9-linux-gnueabi-
       #export PATH=/usr/local/arm/arm-cortex_a9-eabi-4.7-eglibc-2.18/bin:$PATH
        export PATH=/usr/local/arm/arm-cortex_a9-eabi-4.6.3-eglibc-2.16/bin:$PATH
        export KDIR=$KDIR_NXP4330
        make clean
        make
        rm $OUTDIR_NXP4330/nfs/rootfs/mio.ko
        cp mio.ko $OUTDIR_NXP4330/nfs/rootfs/
        ;;

    "clean")
        echo "cleaning"
        make clean
        ;;

    "x86_64")
        echo "x86_64"
        export KDIR=$KDIR_X86_64
        make
        ;;

esac


#!/bin/bash
path=$1
cd $path
datstr=`date "+%Y_%m_%d_%H_%M_%S"`
mkdir /mnt/log
mkdir /mnt/log/olalog
mkdir /mnt/log/olalog/`hostname`

# 判断是否在运行
if ps -e | grep "tar jcvf /mnt/log/`hostname`" > /dev/null
then
    echo "Tar Running!"
else
    echo "Tar start!"
    tar jcvf /mnt/log/olalog/`hostname`/$datstr.tar.bz2 $1 
    chmod 7777 /mnt/log/olalog/ -R
    echo "Tar end!"
fi



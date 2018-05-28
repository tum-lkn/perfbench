#!/bin/bash
folder=$1
file=$2
timestamp=$3
pid=`ps -aux | grep ovs-vswitchd | grep openvswitch | awk '{print $2}'`

mkdir -p $folder
rm $folder/$file
echo $timestamp > $folder/start_cpu_ovs.txt
~/sh/dataplane/audria/audria -o $folder/$file -d 0.5 -f Name,CurCPUPerc,VmSizekB,VmRsskB $pid

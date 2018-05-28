#!/bin/sh

PAIRS=$1

# Install ifconfig for Ubuntu 18.04
sudo apt-get install net-tools

for i in $(seq $PAIRS)
do   
    VETH1=veth$((($i * 2) - 2))
    VETH2=veth$((($i * 2) - 1))
    
    echo "Adding Pair $VETH1 -- $VETH2"
    sudo ip link add type veth
    sudo ifconfig $VETH1 up
    sudo ifconfig $VETH2 up    
done


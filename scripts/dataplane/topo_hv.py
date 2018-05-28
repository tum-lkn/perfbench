#!/usr/bin/python

from topo_base import BaseTopo

if __name__ == '__main__':
    controller_ip = "10.10.1.2" # IP to OVX
    controller_port = 6633
    print "Starting Mininet with OVX (%s:%d)" % (controller_ip,controller_port)
    Topo = BaseTopo(controller_ip = controller_ip, controller_port = controller_port)
    Topo.start()
    

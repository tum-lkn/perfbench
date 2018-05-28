#!/usr/bin/python

from topo_base import BaseTopo

if __name__ == '__main__':
    controller_ip = "192.168.1.2"
    controller_port = 6633
    print "Starting Mininet without HV (%s:%d)" % (controller_ip,controller_port)
    Topo = BaseTopo(controller_ip = controller_ip, controller_port = controller_port)
    Topo.start()
    

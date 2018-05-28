#!/usr/bin/python

from mininet.net import Mininet
from mininet.topo import Topo
from mininet.log import lg, setLogLevel
from mininet.cli import CLI
from mininet.node import RemoteController
from mininet.link import Intf

CORES = [
  ('SEA', {'dpid': '000000000000010%s'}),
  #('SFO', {'dpid': '000000000000020%s'}),
  #('LAX', {'dpid': '000000000000030%s'}),
  #('ATL', {'dpid': '000000000000040%s'}),
  #('IAD', {'dpid': '000000000000050%s'}),
  #('EWR', {'dpid': '000000000000060%s'}),
  #('SLC', {'dpid': '000000000000070%s'}),
  #('MCI', {'dpid': '000000000000080%s'}),
  #('ORD', {'dpid': '000000000000090%s'}),
  #('CLE', {'dpid': '0000000000000a0%s'}),
  #('IAH', {'dpid': '0000000000000b0%s'}),
  ]

FANOUT = 2
    
class BaseTopo(Topo):

    def __init__(self, enable_all = True, controller_ip = "127.0.0.1", controller_port = 6633): 
        self.controller_ip = controller_ip
        self.controller_port = controller_port
        
        # Add default members to class.
        super(BaseTopo, self).__init__()

        # Add core switches
        self.cores = {}
        for switch in CORES:
          self.cores[switch[0]] = self.addSwitch(switch[0], dpid=(switch[1]['dpid'] % '0'), protocols="OpenFlow10")

        # Add hosts and connect them to their core switch
        switchNr = 0
        for switch in CORES:
          switchNr += 1
          
          for count in xrange(1, FANOUT + 1):
            # Add hosts
            host = 'h_%s_%s' % (switch[0], count)
            ip = '10.0.%d.%d' % (switchNr,count)
            mac = switch[1]['dpid'][4:] % count
            h = self.addHost(host, ip=ip, mac=mac)
            # Connect hosts to core switches
            self.addLink(h, self.cores[switch[0]])
          

        # Connect core switches
        #self.addLink(self.cores['SFO'], self.cores['SEA'])    
        """self.addLink(self.cores['SFO'], self.cores['LAX'])
        self.addLink(self.cores['SEA'], self.cores['SLC'])
        self.addLink(self.cores['LAX'], self.cores['SLC'])
        self.addLink(self.cores['LAX'], self.cores['IAH'])
        self.addLink(self.cores['SLC'], self.cores['MCI'])
        self.addLink(self.cores['MCI'], self.cores['IAH'])
        self.addLink(self.cores['MCI'], self.cores['ORD'])
        self.addLink(self.cores['IAH'], self.cores['ATL'])
        self.addLink(self.cores['ORD'], self.cores['ATL'])
        self.addLink(self.cores['ORD'], self.cores['CLE'])
        self.addLink(self.cores['ATL'], self.cores['IAD'])
        self.addLink(self.cores['CLE'], self.cores['IAD'])
        self.addLink(self.cores['CLE'], self.cores['EWR'])
        self.addLink(self.cores['EWR'], self.cores['IAD'])
        """
	
    def create_hwintf(self,intfName,switch):

        print( '*** Adding hardware interface', intfName, 'to switch',
              switch.name, '\n' )
        _intf = Intf( intfName, node=switch )
        
    def start(self):
       
       c = RemoteController('c', ip=self.controller_ip, port=self.controller_port)
       net = Mininet(topo=self, autoSetMacs=True, xterms=False, controller=None)
       net.addController(c)

       """ HW Interfaces """
       self.create_hwintf("eth2",net.switches[0])       
       print(net.switches)

       net.start()       
       CLI(net)
       net.stop()
	
 
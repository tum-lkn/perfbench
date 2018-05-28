# perfbench
Welcome to the **perfbench** repository.

Perfbench is a SDN Switch/Hypervisor benchmarking tool.

It was tested with Ubuntu 14.04 up to 18.04.

## Installation
Please run **install.sh** to completely install perfbench.

## Compile
To compile perfbench go to the root directory and type `cmake .;cmake --build . --target perfbench -- -j 4`.

## Run
Just run `./perfbench`. Start with parameter **--help** to show command line parameters

For some testcases you need root privileges. Start perfbench with `sudo ./perfbench`

Here are some example use-cases. A working management network infrastructure between switch/hypervisor/perfbench is needed.

### Switch Only

I recommend using OVS/Mininet as the switch. You can easily start a switch topology with: 

`sudo mn --topo=single,1 --controller=remote,ip=<IP_TO_PERFBENCH> --switch ovs,protocols=OpenFlow10`

Then start perfbench it with the following parameters:

Here we are using the FeaturesRequest testcase with a OpenFlow packet-rate of 1000 pps per tenant.

`./perfbench --tenants <TENANT_COUNT> --testcase FeaturesRequest --pps 1000`

### Hypervisor

If you want to use perfbench with a hypervisor, start the switch first (OVS/Mininet):

`sudo mn --topo=single,1 --controller=remote,ip=<IP_TO_PERFBENCH> --switch ovs,protocols=OpenFlow10`

Then create the virtual SDNs (slicing, you can find it in the documentation of the hypervisor)

Here we use a packet-rate of 100pps with a send-interval of 1000ms. That means it sends bursts of 100 packets each second.

`./perfbench --tenants <TENANT_COUNT> --testcase FeaturesRequest --pps 100 --send-interval 1000`

### Hypervisor with Perfbench as Dataplane (Sandwich testcases)

If you want to use perfbench with a hypervisor and perfbench acting as the dataplane/switches start it with the following parameters:

#### Non-Shared switches

Here we are using the FlowModSandwich testcase with a OpenFlow packet-rate of 100 pps per tenant per switch connection. We are using 5 switches per virtual-SDN/Tenant where all the tenants use their own switches (non-shared). That results in a total number of switches: tenants * num-switches

`./perfbench --tenants <TENANT_COUNT> --testcase FlowModSandwich --num-switches 5 --pps 100 --switch-ip <IP_OF_HYPERVISOR>`

#### Shared Switches

Here we are using the FlowModSandwich testcase with a OpenFlow packet-rate of 100 pps per tenant per switch connection. We are using 5 switches where all the tenants share the same switches.

`./perfbench --tenants <TENANT_COUNT> --testcase FlowModSandwichSingle --num-switches 5 --pps 100 --switch-ip <IP_OF_HYPERVISOR>`

## Logging

Perfbench logs the tx/rx OpenFlow packets timestamps, type, XID and size in a file after each run. You can specify the logfile path with the: `--logfile` parameter. It automatically adds the tenant-id and "\*.csv" to the end of the filename.
e.g. `--logfile path/to/dir/perfbench_log` results in **path/to/dir/perfbench_log_1.csv** and **path/to/dir/perfbench_log_2.csv** if you used 2 tenants.

The logfile is structured as following (columns sepparated by " "):

xid | request_msgtype | request_timestamp | request_size | response_msgtype | response_timestamp | response_size | latency | conn_id
--- | --------------- | ----------------- | ------------ | ---------------- | ------------------ | ------------- | ------- | ------- 
The xid of each OpenFlow Request/Response | Request Msgtype | Request Timestamp | Request Size (of OF packet) | Response Msgtype | Response Timestamp | Response Size | Latency in ms (Response Timestamp - Request Timestamp) | Connection-ID (Multi-Switch testcases) 


e.g.

```
0 5 1444052493.642726 8 6 1444052493.643310 176 0.584126 0
1 5 1444052493.646452 8 6 1444052493.646632 176 0.180006 0
2 5 1444052493.655894 8 6 1444052493.656299 176 0.405073 0
...
```

## Documentation
Please visit: https://wiki.lkn.ei.tum.de/intern:lkn:all:sdn:perfbench:start
for documentation



#!/bin/bash
# MT25074_Part_A_Namespaces.sh
# Roll: MT25074. I'm setting up two network namespaces (ns1, ns2) and a veth pair
# so the server runs in one namespace and the client in the other, as required.

# I create two separate network namespaces
sudo ip netns add ns1
sudo ip netns add ns2

# I build a virtual ethernet link pair - traffic from veth1 goes to veth2 and vice versa
sudo ip link add veth1 type veth peer name veth2

# I attach each end of the veth to a namespace so they're in isolated networks
sudo ip link set veth1 netns ns1
sudo ip link set veth2 netns ns2

# I assign 10.0.0.1 to ns1's interface and bring it up (server will bind here)
sudo ip netns exec ns1 ip addr add 10.0.0.1/24 dev veth1
sudo ip netns exec ns1 ip link set veth1 up
sudo ip netns exec ns1 ip link set lo up

# I assign 10.0.0.2 to ns2's interface and bring it up (client will connect to 10.0.0.1)
sudo ip netns exec ns2 ip addr add 10.0.0.2/24 dev veth2
sudo ip netns exec ns2 ip link set veth2 up
sudo ip netns exec ns2 ip link set lo up

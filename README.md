# fake-eth

# Tested on:
Linux work.n 5.4.0-55-generic #61-Ubuntu SMP Mon Nov 9 20:49:56 UTC 2020 x86_64 x86_64 x86_64 GNU/Linux
LSB Version:	core-11.1.0ubuntu2-noarch:printing-11.1.0ubuntu2-noarch:security-11.1.0ubuntu2-noarch
Distributor ID:	Ubuntu
Description:	Ubuntu 20.04.1 LTS
Release:	20.04
Codename:	focal

# Build
git clone https://github.com/kt-315/fake-eth.git
cd fake-eth
make
sudo insmod ./fake-eth.ko

# Using
sudo sh -c 'echo "+fake%d;00:01:02:03:04:05" > /proc/fake-eth'
cat /proc/fake-eth
sudo sh -c 'echo "-fake0" > /proc/fake-eth'

# spdk-seastar-demo
Just for study Seastar and SPDK, learn to use Seastar instead of SPDK event as the async. framework. 

Trying to use (DPDK + Seastar + SPDK) to do some run-to-completion experiments.

Known issues:
1. It's hard to build with static library(you could see from my comments in CMake file)
2. Not compile with Seastar's DPDK(--enable-dpdk), may conflict with SPDK


Replace the traddr in demo_device_init()(demo.cc) with the right PCIe addr of your machine, because not parse cli arguments yet.

Usage:
1. git clone git@github.com:lanceflee/spdk-seastar-demo.git
2. cd spdk-seastar-demo
3. sh install-dep.sh
4. mkdir build && cd  build
5. cmake ..
6. make
7. ./demo -c2 (may not work if not pass -c)

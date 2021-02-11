# spdk-seastar-demo
Studying Seastar and SPDK, and learning to use Seastar instead of SPDK event as the async. framework. 

Trying to use (DPDK + Seastar + SPDK) to do some run-to-completion experiments.
This demo is implemented with a simple Seastar shard(with unique qpair allocated) model while shared nothing writing and reading "Hello World" in parallel.

Known issues:
1. It's hard to build with static library(you could see from my comments in CMake file)
2. Not compile with Seastar's DPDK(--enable-dpdk), may conflict with SPDK
3. Not parse positional cli arguments, just pass addr like"0000:01:00.0". 


Usage:
1. git clone git@github.com:lanceflee/spdk-seastar-demo.git
2. cd spdk-seastar-demo
3. sh install-dep.sh
4. mkdir build && cd  build
5. cmake ..
6. make
7. ./demo -c2 --device 0000:01:00.0(your PCIe device addr)
(may not work if not pass -c)

PS: make sure: setup system hugepage before run.
sh ${spdk}/scripts/setup.sh

# spdk-seastar-demo

Replace the traddr in demo_device_init()(demo.cc) with the right PCIe addr of your machine, because not parse cli arguments yet.

Usage:
1. git clone git@github.com:lanceflee/spdk-seastar-demo.git
2. cd spdk-seastar-demo
3. sh install-dep.sh
4. mkdir build && cd  build
5. cmake ..
6. make

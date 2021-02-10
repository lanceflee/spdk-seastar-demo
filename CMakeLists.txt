cmake_minimum_required (VERSION 2.8)
project (demo)


set(ROOT_DIR ${CMAKE_SOURCE_DIR})
set(SEASTAR_SOURCE_DIR "${ROOT_DIR}/seastar")
set(SPDK_SOURCE_DIR "${ROOT_DIR}/spdk")
set(DPDK_SOURCE_DIR "${ROOT_DIR}/spdk/dpdk")

include_directories(
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${CMAKE_CURRENT_BINARY_DIR}
    ${SEASTAR_SOURCE_DIR}/include
    ${DPDK_SOURCE_DIR}/build/include
    ${SPDK_SOURCE_DIR}/include
)
link_directories(
    ${SPDK_SOURCE_DIR}/build/lib
    ${DPDK_SOURCE_DIR}/build/lib
)


#Seastar
list(APPEND CMAKE_PREFIX_PATH 
            ${SEASTAR_SOURCE_DIR}/build/release
            ${SEASTAR_SOURCE_DIR}/build/release/_cooking/installed)
list(APPEND CMAKE_MODULE_PATH 
            ${SEASTAR_SOURCE_DIR}/cmake)
find_package (Seastar REQUIRED)

add_executable(demo demo.cc)

#rollback to use sharedlib
target_link_libraries (demo PRIVATE Seastar::seastar -lspdk -lspdk_env_dpdk -lrte_bus_pci -lrte_eal
                        -lrte_mbuf -lrte_kvargs -lrte_mempool -lrte_mempool_ring -lrte_pci -lrte_ring -lrte_telemetry
                        -lrte_power -lrte_timer)

#build Seastar+DPDK+SPDK outside, see install-dep.sh
#Refer to ceph/cmake/modules/Build{SPDK/DPDK}.cmake

#[[
foreach(lib nvme env_dpdk log util)
    add_library(spdk_${lib} STATIC IMPORTED)
    set_target_properties(spdk_${lib} PROPERTIES IMPORTED_LOCATION "${SPDK_SOURCE_DIR}/build/lib/libspdk_${lib}.a"
                          INTERFACE_INCLUDE_DIRECTORIES "${SPDK_SOURCE_DIR}/build/include")
    target_link_libraries(demo PRIVATE  spdk_${lib})
endforeach()

#do not need when seastar with  -enable-dpdk
list(APPEND dpdk_components
	    bus_pci
	    eal
	    kvargs
	    mbuf
	    mempool
	    mempool_ring
	    pci
 	    ring
	    telemetry
        bus_vdev
        cfgfile
        hash
        net
        pmd_bnxt
        pmd_cxgbe
        pmd_e1000
        pmd_ena
        pmd_enic
        pmd_i40e
        pmd_ixgbe
        pmd_nfp
        pmd_qede
        pmd_ring
        pmd_sfc_efx
        timer)

 foreach(c ${dpdk_components})
        add_library(dpdk::${c} STATIC IMPORTED)
        set(dpdk_${c}_LIBRARY "${DPDK_SOURCE_DIR}/build/lib/librte_${c}.a")
        set_target_properties(dpdk::${c} PROPERTIES INTERFACE_INCLUDE_DIRECTORIES ${SPDK_SOURCE_DIR}/dpdk/build/include
                             IMPORTED_LOCATION "${dpdk_${c}_LIBRARY}")
        list(APPEND DPDK_LIBRARIES dpdk::${c})
        list(APPEND DPDK_ARCHIVES "${dpdk_${c}_LIBRARY}")
  endforeach()

add_library(dpdk::dpdk INTERFACE IMPORTED)
add_dependencies(dpdk::dpdk ${DPDK_LIBRARIES})
set_target_properties(dpdk::dpdk PROPERTIES
    INTERFACE_INCLUDE_DIRECTORIES ${DPDK_INCLUDE_DIR}
    INTERFACE_LINK_LIBRARIES
    "-Wl,--whole-archive $<JOIN:${DPDK_ARCHIVES}, > -Wl,--no-whole-archive -Wl,-lnuma -Wl,-lpthread,-ldl")


set_target_properties(spdk_env_dpdk PROPERTIES INTERFACE_LINK_LIBRARIES "dpdk::dpdk;rt")
set_target_properties(spdk_util PROPERTIES INTERFACE_LINK_LIBRARIES uuid)


target_link_libraries (demo PRIVATE Seastar::seastar)
]]

# Distributed under the Boost Software License, Version 1.0.
# See http://www.boost.org/LICENSE_1_0.txt

cmake_minimum_required(VERSION 2.8.11)
project(BoostThreadpool)

list(APPEND CMAKE_MODULE_PATH "${PROJECT_SOURCE_DIR}/cmake")

set(LIBRARY_VERSION 0.2.5)

add_library(boost_threadpool INTERFACE)
add_library(boost::threadpool ALIAS boost_threadpool)
set_property(TARGET boost_threadpool PROPERTY EXPORT_NAME threadpool)

include(RypplWriteConfig)

if(${CMAKE_TARGET_ARCHITECTURE} STREQUAL ${CMAKE_HOST_SYSTEM_PROCESSOR})
    include(FindCXX11)
else()
    set(CXX11_FLAGS "-std=gnu++11")
endif()

target_compile_options(boost_threadpool INTERFACE ${CXX11_FLAGS})

target_include_directories(boost_threadpool INTERFACE
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}>
        )
if(NOT Boost_FOUND)
    find_package(Boost 1.61 COMPONENTS thread)
endif()

if(Boost_FOUND)
    target_include_directories(boost_threadpool INTERFACE
            $<BUILD_INTERFACE:${Boost_INCLUDE_DIRS}>)
    target_link_libraries(boost_threadpool INTERFACE ${Boost_LIBRARIES})
else()
    target_link_libraries(boost_threadpool INTERFACE
            boost::thread
            boost::type_traits
            boost::function
            boost::bind
            )
endif()

install(DIRECTORY boost DESTINATION include/)

install(TARGETS boost_threadpool EXPORT ${PROJECT_NAME}Targets
        INCLUDES DESTINATION include
        )

install(EXPORT ${PROJECT_NAME}Targets
        FILE ${PROJECT_NAME}Targets.cmake
        NAMESPACE boost::
        DESTINATION lib/cmake/${PROJECT_NAME}
        )

write_config_file("${CMAKE_BINARY_DIR}/lib/cmake/${PROJECT_NAME}/${PROJECT_NAME}Config.cmake"
        Fusion
        Parameter
        UBLAS
        Random
        Tuple
        )
write_basic_package_version_file("${CMAKE_BINARY_DIR}/lib/cmake/${PROJECT_NAME}/${PROJECT_NAME}ConfigVersion.cmake"
        VERSION ${LIBRARY_VERSION}
        COMPATIBILITY AnyNewerVersion
        )

install(FILES
        "${CMAKE_BINARY_DIR}/lib/cmake/${PROJECT_NAME}/${PROJECT_NAME}Config.cmake"
        "${CMAKE_BINARY_DIR}/lib/cmake/${PROJECT_NAME}/${PROJECT_NAME}ConfigVersion.cmake"
        DESTINATION lib/cmake/${PROJECT_NAME}
        )

export(TARGETS boost_threadpool
        NAMESPACE boost::
        FILE ${CMAKE_BINARY_DIR}/lib/cmake/${PROJECT_NAME}/${PROJECT_NAME}Targets.cmake)

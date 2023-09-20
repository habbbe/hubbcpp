find_package(PkgConfig)

pkg_check_modules(GTKMM gtkmm-3.0)

link_directories(
    ${GTKMM_LIBRARY_DIRS}  )

include_directories(
    ${GTKMM_INCLUDE_DIRS}
    /usr/lib/atkmm-1.6/include)

set(PLATFORM_SPECIFIC_LIBS "-lpthread")
set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} -s")

add_executable(hubb-gtk main.cpp)

target_link_libraries(hubb-gtk
    common
    ${GTKMM_LIBRARIES}
    pthread
    )

add_library(common STATIC parser.cpp)

set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} -finline-limit=3500")

target_include_directories(common PUBLIC "${CMAKE_CURRENT_LIST_DIR}/include")

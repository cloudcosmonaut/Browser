
find_package(PkgConfig REQUIRED)
pkg_check_modules(GTKMM REQUIRED gtkmm-3.0)

include(FetchContent)
FetchContent_Declare(
  googletest
  URL https://github.com/google/googletest/archive/e2239ee6043f73722e7aa812a459f54a28552929.zip
)
set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)
FetchContent_MakeAvailable(googletest)

get_property(CMARK_BINARY_DIR GLOBAL PROPERTY COMMONMARKER_BINARY_DIR)

enable_testing()

add_executable(draw draw_test.cc)
target_include_directories(draw PRIVATE ${CMAKE_SOURCE_DIR}/src ${CMARK_BINARY_DIR} ${GTKMM_INCLUDE_DIRS})
target_link_libraries(draw PRIVATE libreweb-browser-lib-draw ipfs-http-client ${GTKMM_LIBRARIES} LibCommonMarker gtest_main)
add_test(NAME draw_test COMMAND draw)

add_executable(file file_test.cc)
target_include_directories(file PRIVATE ${CMAKE_SOURCE_DIR}/src)
target_link_libraries(file PRIVATE libreweb-browser-lib-file gtest_main)
add_test(NAME file_test COMMAND file)

add_executable(ipfs ipfs_test.cc)
target_include_directories(ipfs PRIVATE ${CMAKE_SOURCE_DIR}/src)
target_link_libraries(ipfs PRIVATE libreweb-browser-lib-ipfs ipfs-http-client gtest_main)
add_test(NAME ipfs_test COMMAND ipfs)

add_executable(parser parser_test.cc)
target_include_directories(parser PRIVATE ${CMAKE_SOURCE_DIR}/src ${CMARK_BINARY_DIR}  ${GTKMM_INCLUDE_DIRS})
target_link_libraries(parser PRIVATE libreweb-browser-lib-parser LibCommonMarker LibCommonMarkerExtensions gtest_main)
add_test(NAME parser_test COMMAND parser)

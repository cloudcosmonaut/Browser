
include(FetchContent)
FetchContent_Declare(
  googletest
  URL https://github.com/google/googletest/archive/e2239ee6043f73722e7aa812a459f54a28552929.zip
)
set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)
FetchContent_MakeAvailable(googletest)

enable_testing()
add_executable(example example.cc)
# Add gtest and libreweb libary to target
target_link_libraries(example libreweb-browser-lib gtest_main)
target_include_directories(example PRIVATE ${CMAKE_SOURCE_DIR}/src)
add_test(NAME example_test COMMAND example)

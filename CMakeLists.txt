cmake_minimum_required(VERSION 3.4)

project(redis_module_bench)

add_definitions(-DREDISMODULE_EXPERIMENTAL_API)
if(APPLE)
  set(REDIS_MODULE_CFLAGS -W -Wall -dynamic -fno-common -g -ggdb -std=c++11 -O2)
  set(REDIS_MODULE_LDFLAGS "-undefined dynamic_lookup")
  set(CMAKE_SHARED_LIBRARY_SUFFIX ".so")
else()
  set(REDIS_MODULE_CFLAGS -W -Wall -fno-common -g -ggdb -std=c++11 -O2)
  set(REDIS_MODULE_LDFLAGS -shared)
endif()

add_library(redis_module_bench SHARED bench.cc)

target_compile_options(redis_module_bench PUBLIC ${REDIS_MODULE_CFLAGS} -fPIC)
target_link_libraries(redis_module_bench ${REDIS_MODULE_LDFLAGS})

if(NOT TARGET OpenMP::OpenMP_CXX)
  find_package(Threads REQUIRED)
  add_library(OpenMP::OpenMP_CXX IMPORTED INTERFACE)
  set_property(TARGET OpenMP::OpenMP_CXX PROPERTY INTERFACE_COMPILE_OPTIONS
                                                  ${OpenMP_CXX_FLAGS})
  set_property(
    TARGET OpenMP::OpenMP_CXX PROPERTY INTERFACE_LINK_LIBRARIES
                                       ${OpenMP_CXX_FLAGS} Threads::Threads)

endif()

add_executable(scpa-omp main.c ../common/utils.c ../common/matrix.c)

target_link_libraries(scpa-omp PUBLIC OpenMP::OpenMP_CXX)

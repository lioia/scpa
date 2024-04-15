find_package(OpenMP REQUIRED)

add_executable(scpa-omp main.c ../common/utils.c ../common/matrix.c)

if(OpenMP_C_FOUND)
  target_link_libraries(scpa-omp PUBLIC OpenMP::OpenMP_C)
endif()

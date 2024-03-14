# Compute capability. See -arch option in nvcc --help.
set(CC 75)

find_package(CUDA REQUIRED)
include_directories(${CUDA_INCLUDE_DIRS})

include_directories(${CUDA_TOOLKIT_ROOT_DIR}/samples/Common)

set(CUDA_PROPAGATE_HOST_FLAGS OFF)
set(CUDA_NVCC_FLAGS
    ${CUDA_NVCC_FLAGS};-DSM_${CC};-arch=sm_${CC};-lineinfo;-Xcompiler=${CMAKE_CXX_FLAGS};-Xptxas=-v
)

cuda_add_executable(scpa-cuda main.cu)

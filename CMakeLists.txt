cmake_minimum_required(VERSION 3.26)
set(PROJECT_NAME cvhub-objectDetection)
project(${PROJECT_NAME})

# -------------- Support C++17 for using filesystem  ------------------#
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS ON)
set(CMAKE_INCLUDE_CURRENT_DIR ON)

# -------------- OpenCV  ------------------#
find_package(OpenCV REQUIRED)
include_directories(${OpenCV_INCLUDE_DIRS})

# -------------- Compile CUDA for FP16 inference if needed  ------------------#
set(USE_CUDA  ON)
if (USE_CUDA)
    find_package(CUDAToolkit REQUIRED)
    include_directories(${CUDAToolkit_INCLUDE_DIRS})
    add_definitions(-DUSE_CUDA)
endif()

# -------------- ONNXRUNTIME  ------------------#

# Set ONNXRUNTIME_VERSION
set(ONNXRUNTIME_VERSION 1-28-0)
set(ONNXRUNTIME_ROOT "${CMAKE_CURRENT_SOURCE_DIR}/onnxruntime_linux_gpu_cuda12_${ONNXRUNTIME_VERSION}")

include_directories(${CMAKE_CURRENT_SOURCE_DIR}/include ${CMAKE_CURRENT_SOURCE_DIR} ${ONNXRUNTIME_ROOT}/include)

set(PROJECT_SOURCES
        src/main.cpp
        src/inference.cpp
        src/logger.cpp
)

add_executable(${PROJECT_NAME} ${PROJECT_SOURCES})

set(EXECUTABLE_OUTPUT_PATH ${CMAKE_CURRENT_SOURCE_DIR}/bin)

# target_include_directories(${PROJECT_NAME} PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/common)

target_link_libraries(${PROJECT_NAME} ${OpenCV_LIBS} ${ONNXRUNTIME_ROOT}/lib/libonnxruntime.so)

if(USE_CUDA)
    target_link_libraries(${PROJECT_NAME} ${CUDAToolkit_LIBRARIES})
endif()

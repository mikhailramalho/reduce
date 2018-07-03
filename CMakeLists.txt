cmake_minimum_required(VERSION 3.10)
project(reduce)

set(CMAKE_CXX_STANDARD 17)

set(LIBRARY_LIST clangTooling clangFrontend clangParse clangSema clangEdit)
set(LIBRARY_LIST ${LIBRARY_LIST} clangAnalysis clangAST clangLex clangBasic)
set(LIBRARY_LIST ${LIBRARY_LIST} clangDriver clangSerialization clangRewrite)
set(LIBRARY_LIST ${LIBRARY_LIST} clangRewriteFrontend)

set(COMPONENT_LIST mcparser bitreader support mc option)

find_package(LLVM REQUIRED CONFIG)
message(STATUS "Found LLVM ${LLVM_PACKAGE_VERSION}")
message(STATUS "Using LLVMConfig.cmake in: ${LLVM_DIR}")

include_directories(${LLVM_INCLUDE_DIRS})
if(LLVM_BUILD_MAIN_SRC_DIR)
    include_directories(${LLVM_BUILD_MAIN_SRC_DIR}/tools/clang/include)
    include_directories(${LLVM_BUILD_BINARY_DIR}/tools/clang/include)
endif()
link_directories(${LLVM_LIBRARY_DIRS})
add_definitions(${LLVM_DEFINITIONS})

add_definitions(
        -D__STDC_LIMIT_MACROS
        -D__STDC_CONSTANT_MACROS
)

set(SOURCE_FILES main.cpp Rewriter.cpp Rewriter.h AddDefinitions.h DumpHeaders.cpp DumpHeaders.h)
add_executable(reduce ${SOURCE_FILES})

add_subdirectory(headers)

if ("${CMAKE_CXX_COMPILER_ID}" STREQUAL "MSVC")
    foreach(link_lib IN LISTS LIBRARY_LIST)
        target_link_libraries(${PROJECT_NAME} optimized ${link_lib})
        target_link_libraries(${PROJECT_NAME} debug     ${link_lib}d)
    endforeach()
else()
    target_link_libraries(${PROJECT_NAME} ${LIBRARY_LIST})
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11 -Wno-unused-parameter -fno-strict-aliasing -fno-exceptions -fno-rtti")
endif()

#llvm_map_components_to_libnames(llvm_libs ${COMPONENT_LIST})
#target_link_libraries(${PROJECT_NAME} ${llvm_libs})
target_link_libraries(${PROJECT_NAME}
        LLVMX86AsmParser # MC, MCParser, Support, X86Desc, X86Info
        LLVMX86Desc # MC, Support, X86AsmPrinter, X86Info
        LLVMX86AsmPrinter # MC, Support, X86Utils
        LLVMX86Info # MC, Support, Target
        LLVMX86Utils # Core, Support
        LLVMipo
        LLVMScalarOpts
        LLVMInstCombine
        LLVMTransformUtils
        LLVMAnalysis
        LLVMTarget
        LLVMOption # Support
        LLVMMCParser # MC, Support
        LLVMMC # Object, Support
        LLVMObject # BitReader, Core, Support
        LLVMBitReader # Core, Support
        LLVMCore # Support
        LLVMSupport
        )

message(STATUS "User selected libraries = ${LIBRARY_LIST}")
message(STATUS "User selected components = ${COMPONENT_LIST}")

LINK_DIRECTORIES(${CMAKE_BINARY_DIR}/headers)
TARGET_LINK_LIBRARIES(${PROJECT_NAME} clangheaders)
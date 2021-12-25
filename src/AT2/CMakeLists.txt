cmake_minimum_required (VERSION 3.8 FATAL_ERROR)

project (AT2_Engine)

file(GLOB_RECURSE ${PROJECT_NAME}_SOURCES
    "*.h"
    "*.hpp"
    "*.cpp"
)

add_library(${PROJECT_NAME} STATIC
    ${${PROJECT_NAME}_SOURCES}
)

option(USE_ASSIMP "Use ASSIMP library" ON)
option(USE_PLATFORM_HACKS "speed-up execution in exchange for some non-portable code" OFF)


target_compile_definitions( ${PROJECT_NAME} PRIVATE
    $<$<BOOL:${USE_ASSIMP}>:USE_ASSIMP>
    $<$<BOOL:${USE_PLATFORM_HACKS}>:USE_PLATFORM_HACKS>
)

target_include_directories(${PROJECT_NAME} PUBLIC "${CMAKE_SOURCE_DIR}/src/")


target_link_libraries(${PROJECT_NAME} PUBLIC ${CONAN_LIBS})

if(DEVIL_FOUND)
    target_compile_definitions(${PROJECT_NAME} PRIVATE USE_DEVIL)
    target_link_libraries(${PROJECT_NAME} PRIVATE ${IL_LIBRARIES} ${ILU_LIBRARIES}) 
endif()


target_include_directories(${PROJECT_NAME} PRIVATE "${CMAKE_BINARY_DIR}/fx-gltf/include/")

target_compile_features(${PROJECT_NAME} PRIVATE cxx_std_20)

# MacOS - specific
if (CMAKE_SYSTEM_NAME STREQUAL "Darwin")
  message ("Apple frameworks will be linked")
  target_include_directories(${PROJECT_NAME} PRIVATE "${CMAKE_SOURCE_DIR}/3rd_party/metal-cpp/")
  
  find_library(QUARTZ_CORE_LIB QuartzCore)
  find_library(METAL_LIB Metal)
  find_library(FOUNDATION_LIB Foundation)
  
  if ((NOT QUARTZ_CORE_LIB) OR (NOT METAL_LIB) OR (NOT FOUNDATION_LIB) )
    message(FATAL_ERROR "Apple frameworks not found")
  endif()

  
  target_link_libraries(${PROJECT_NAME} PRIVATE ${QUARTZ_CORE_LIB} ${METAL_LIB} ${FOUNDATION_LIB})
  
endif()

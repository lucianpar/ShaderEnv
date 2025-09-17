cmake_minimum_required(VERSION 3.20)

project(agent-embedding LANGUAGES C CXX)

# C++ setup
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_POSITION_INDEPENDENT_CODE ON)

# Build llama.cpp the way we want (static, no extras)
set(LLAMA_METAL ON CACHE BOOL "" FORCE)          # Metal backend on Apple GPUs
set(LLAMA_STATIC ON CACHE BOOL "" FORCE)         # build static libllama
set(BUILD_SHARED_LIBS OFF CACHE BOOL "" FORCE)   # avoid dylib/RPATH headaches
set(LLAMA_BUILD_EXAMPLES OFF CACHE BOOL "" FORCE)
set(LLAMA_BUILD_TESTS OFF CACHE BOOL "" FORCE)

# Pull in the vendored llama.cpp
add_subdirectory(third_party/llama.cpp EXCLUDE_FROM_ALL)

# Our library
add_library(agent_embedding
agent/src/Agent.cpp
agent/src/AgentEmbedding.cpp
)
target_include_directories(agent_embedding PUBLIC
  ${CMAKE_CURRENT_SOURCE_DIR}/agent/src
  ${CMAKE_CURRENT_SOURCE_DIR}/agent/third_party/json
  ${CMAKE_CURRENT_SOURCE_DIR}/agent/third_party/llama.cpp/include
  ${CMAKE_SOURCE_DIR}/agent/third_party/llama.cpp/ggml/include

)

target_link_directories(agent_embedding PRIVATE
  ${CMAKE_SOURCE_DIR}/agent/third_party/llama.cpp/build/bin
)
target_link_libraries(agent_embedding PRIVATE llama)
set_target_properties(agent_embedding PROPERTIES
  INSTALL_RPATH "@loader_path/../../third_party/llama.cpp/build/bin"
)




# Demo executable
add_executable(encode_demo examples/encode_demo.cpp)
target_link_libraries(encode_demo PRIVATE agent_embedding)

find_package(CURL REQUIRED)

add_executable(embed_file scripts/embed_file.cpp)
target_link_libraries(embed_file PRIVATE CURL::libcurl)
target_include_directories(embed_file PRIVATE ${CMAKE_CURRENT_LIST_DIR}/third_party)

add_executable(search_embed scripts/search.cpp)
target_link_libraries(search_embed PRIVATE CURL::libcurl)
target_include_directories(search_embed PRIVATE ${CMAKE_CURRENT_LIST_DIR}/third_party)




# (Optional) copy any local models into the build tree for convenience
if(EXISTS ${CMAKE_CURRENT_SOURCE_DIR}/models)
  add_custom_target(agent_assets ALL
    COMMAND ${CMAKE_COMMAND} -E make_directory ${CMAKE_CURRENT_BINARY_DIR}/models
    COMMAND ${CMAKE_COMMAND} -E copy_directory
            ${CMAKE_CURRENT_SOURCE_DIR}/models
            ${CMAKE_CURRENT_BINARY_DIR}/models
    BYPRODUCTS ${CMAKE_CURRENT_BINARY_DIR}/models
    COMMENT "Copying GGUF models to build directory"
  )
  add_dependencies(encode_demo agent_assets)
endif()

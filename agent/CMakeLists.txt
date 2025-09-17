cmake_minimum_required(VERSION 3.20)
project(agent-embedding LANGUAGES C CXX)

# --- C++ setup ---------------------------------------------------------------
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_POSITION_INDEPENDENT_CODE ON)

# Output all binaries to agent/build to match your manual invocations
set(AGENT_OUT "${CMAKE_SOURCE_DIR}/agent/build")
file(MAKE_DIRECTORY "${AGENT_OUT}")

# --- Build llama.cpp (static; Metal on macOS) --------------------------------
# Avoid dylib/RPATH headaches by statically linking libllama
set(LLAMA_METAL ON CACHE BOOL "" FORCE)
set(LLAMA_STATIC ON CACHE BOOL "" FORCE)
set(BUILD_SHARED_LIBS OFF CACHE BOOL "" FORCE)
set(LLAMA_BUILD_EXAMPLES OFF CACHE BOOL "" FORCE)
set(LLAMA_BUILD_TESTS OFF CACHE BOOL "" FORCE)

# Locate the submodule whether it's at agent/third_party or top-level third_party
set(LLAMA_PATH "${CMAKE_CURRENT_SOURCE_DIR}/agent/third_party/llama.cpp")
if(NOT EXISTS "${LLAMA_PATH}/CMakeLists.txt")
  set(LLAMA_PATH "${CMAKE_CURRENT_SOURCE_DIR}/third_party/llama.cpp")
endif()
if(NOT EXISTS "${LLAMA_PATH}/CMakeLists.txt")
  message(FATAL_ERROR
    "Could not find llama.cpp. Run: git submodule update --init --recursive")
endif()

add_subdirectory(${LLAMA_PATH} EXCLUDE_FROM_ALL)

# --- Our static library: agent_embedding ------------------------------------
add_library(agent_embedding
  src/AgentEmbedding.cpp
)

# Public includes so downstream tools inherit them automatically
target_include_directories(agent_embedding PUBLIC
  ${CMAKE_CURRENT_SOURCE_DIR}/agent/src
  ${CMAKE_CURRENT_SOURCE_DIR}/agent/third_party/json
  ${LLAMA_PATH}/include
  ${LLAMA_PATH}/ggml/include
  ${LLAMA_PATH}/ggml/src        # some headers include from ggml/src
)

# Link against llama (static)
target_link_libraries(agent_embedding PUBLIC llama)

# Put library/artifacts under agent/build
set_target_properties(agent_embedding PROPERTIES
  ARCHIVE_OUTPUT_DIRECTORY "${AGENT_OUT}"
  LIBRARY_OUTPUT_DIRECTORY "${AGENT_OUT}"
  RUNTIME_OUTPUT_DIRECTORY "${AGENT_OUT}"
)

# --- Tools / executables -----------------------------------------------------
# 1) embed_vocab: embeds shader vocab JSON -> JSONL
add_executable(embed_vocab src/embed_vocab.cpp)
target_link_libraries(embed_vocab PRIVATE agent_embedding)
set_target_properties(embed_vocab PROPERTIES RUNTIME_OUTPUT_DIRECTORY "${AGENT_OUT}")

# 2) plan_shader: turns a prompt into a filled ShaderTemplate plan
add_executable(plan_shader src/plan_shader.cpp)
target_link_libraries(plan_shader PRIVATE agent_embedding)
set_target_properties(plan_shader PROPERTIES RUNTIME_OUTPUT_DIRECTORY "${AGENT_OUT}")

# (Add more tools later as needed and link to agent_embedding)

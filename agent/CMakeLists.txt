cmake_minimum_required(VERSION 3.20)
project(agent-embedding LANGUAGES C CXX)

# --- C++ setup ---------------------------------------------------------------
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_POSITION_INDEPENDENT_CODE ON)

# Output all binaries to agent/build (to match your manual workflow)
set(AGENT_OUT "${CMAKE_SOURCE_DIR}/build")
file(MAKE_DIRECTORY "${AGENT_OUT}")

# --- Build llama.cpp (static; Metal on macOS) --------------------------------
# Force static lib build to avoid runtime dylib headaches
set(LLAMA_METAL ON CACHE BOOL "" FORCE)
set(LLAMA_STATIC ON CACHE BOOL "" FORCE)
set(BUILD_SHARED_LIBS OFF CACHE BOOL "" FORCE)
set(LLAMA_BUILD_EXAMPLES OFF CACHE BOOL "" FORCE)
set(LLAMA_BUILD_TESTS OFF CACHE BOOL "" FORCE)

# Locate the llama.cpp submodule (try both agent/ and top-level third_party/)
set(LLAMA_PATH "${CMAKE_CURRENT_SOURCE_DIR}/third_party/llama.cpp")
if(NOT EXISTS "${LLAMA_PATH}/CMakeLists.txt")
  set(LLAMA_PATH "${CMAKE_CURRENT_SOURCE_DIR}/third_party/llama.cpp")
endif()
if(NOT EXISTS "${LLAMA_PATH}/CMakeLists.txt")
  message(FATAL_ERROR
    "Could not find llama.cpp. Run: git submodule update --init --recursive")
endif()

add_subdirectory(${LLAMA_PATH} EXCLUDE_FROM_ALL)

# --- Agent embedding library -------------------------------------------------
add_library(agent_embedding
  src/AgentEmbedding.cpp
)

target_include_directories(agent_embedding PUBLIC
  ${CMAKE_CURRENT_SOURCE_DIR}/src
  ${CMAKE_CURRENT_SOURCE_DIR}/third_party/json
  ${LLAMA_PATH}/include
  ${LLAMA_PATH}/ggml/include
  ${LLAMA_PATH}/ggml/src
)

target_link_libraries(agent_embedding PUBLIC llama)

# Ensure all library artifacts go into agent/build
set_target_properties(agent_embedding PROPERTIES
  ARCHIVE_OUTPUT_DIRECTORY "${AGENT_OUT}/lib"
  LIBRARY_OUTPUT_DIRECTORY "${AGENT_OUT}/lib"
  RUNTIME_OUTPUT_DIRECTORY "${AGENT_OUT}/bin"
)

# --- Executables -------------------------------------------------------------
# Helper to reduce boilerplate
function(add_agent_tool name srcfile)
  add_executable(${name} ${srcfile})
  target_link_libraries(${name} PRIVATE agent_embedding)
  set_target_properties(${name} PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY "${AGENT_OUT}/bin"
  )
endfunction()

# Tools
add_agent_tool(embed_vocab src/embed_vocab.cpp)
add_agent_tool(plan_shader src/plan_shader.cpp)

# --- Models (optional) -------------------------------------------------------
if(EXISTS ${CMAKE_CURRENT_SOURCE_DIR}/models)
  add_custom_target(agent_assets ALL
    COMMAND ${CMAKE_COMMAND} -E make_directory ${AGENT_OUT}/models
    COMMAND ${CMAKE_COMMAND} -E copy_directory
            ${CMAKE_CURRENT_SOURCE_DIR}/models
            ${AGENT_OUT}/models
    BYPRODUCTS ${AGENT_OUT}/models
    COMMENT "Copying GGUF models to agent/build/models"
  )
  add_dependencies(embed_vocab agent_assets)
  add_dependencies(plan_shader agent_assets)
endif()

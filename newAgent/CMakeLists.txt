cmake_minimum_required(VERSION 3.10)
project(newAgent)

set(CMAKE_CXX_STANDARD 17)

# Include directories (relative to newAgent)
include_directories(third_party/llama.cpp)  # This covers ggml.h if it's directly in llama.cpp/
include_directories(third_party/nlohmann_json/single_include)

# Link directories (relative to newAgent)
link_directories(third_party/llama.cpp/build)

# Add executable
add_executable(agent agent.cpp)

# Link libraries
target_link_libraries(agent llama)

# Optional: Set output directory for the executable
set_target_properties(agent PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}
)
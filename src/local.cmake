# Add source files
file(GLOB_RECURSE SRC_CC "src/*.cc")

# Compile zerok as a library
add_library(zerok SHARED ${SRC_CC})
# Link gflags to zerok
target_link_libraries(zerok gflags Threads::Threads)
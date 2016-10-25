#
# COMP 308
# If you add new source files, you must list them here
#

# TODO list your header files (.hpp) here
SET(headers
	"comp308.hpp"
	"imageLoader.hpp"
	"shaderLoader.hpp"
	"geometry.hpp"
)


# TODO list your source files (.cpp) here
SET(sources
	"main.cpp"
	"geometry.cpp"
)

# Add executable target and link libraries
# You do not need to touch this
add_executable(${COMP308_ASSIGNMENT} ${headers} ${sources})
target_link_libraries(${COMP308_ASSIGNMENT} PRIVATE GLUT::GLUT glew)
target_link_libraries(${COMP308_ASSIGNMENT} PRIVATE stb)
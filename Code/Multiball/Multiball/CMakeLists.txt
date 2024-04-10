# CMakeList.txt : CMake project for Multiball, include source and define
# project specific logic here.
#

find_package(SDL2 REQUIRED)

set(CMAKE_CXX_FLAGS  "${CMAKE_CXX_FLAGS} -mwindows")

# Add source to this project's executable.
add_executable (Multiball "Multiball.cpp" "Multiball.h" "Game.cpp" "Game.h")


#target_link_libraries(Multiball SDL2::SDL2)
include_directories(${SDL2_INCLUDE_DIRS})
target_link_libraries(${PROJECT_NAME} ${SDL2_LIBRARIES})

#target_link_libraries(Multiball SDL2::SDL2 SDL2::SDL2main)

if (CMAKE_VERSION VERSION_GREATER 3.12)
  set_property(TARGET Multiball PROPERTY CXX_STANDARD 20)
endif()

# TODO: Add tests and install targets if needed.

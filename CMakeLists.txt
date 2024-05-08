cmake_minimum_required(VERSION 3.5)
project( DEMO )

set(CMAKE_INSTALL_PREFIX .)

set(MASTER_DEPEND "I:/Dependencies" CACHE PATH "Set this if necessary to your directory for dependencies.")	
set(CMAKE_PREFIX_PATH ${MASTER_DEPEND} ${CMAKE_PREFIX_PATH})

#####################################
# Require C++17
#####################################

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

#####################################
# Find necessary libraries
#####################################

find_package(assimp REQUIRED)
find_package(glfw3 3.3 REQUIRED) 
find_package(GLEW REQUIRED)	
find_package(glm CONFIG REQUIRED)

add_definitions(-DGLEW_STATIC)

#####################################
# Get general sources
#####################################

include_directories("src/include" ${ASSIMP_INCLUDE_DIRS} ${GLEW_INCLUDE_DIRS})

file(GLOB GENERAL_SOURCES    
    "src/lib/*.cpp"  
    "src/lib/*.hpp"  
)

#####################################
# Create executables 
# and install targets
#####################################

set(ALL_LIBRARIES ${ASSIMP_LIBRARIES} ${ASSIMP_ZLIB} glfw GLEW::glew_s glm::glm)
 
# CGDemo
add_executable(CGDemo ${GENERAL_SOURCES} "./src/app/CGDemo.cpp")
target_link_libraries(CGDemo ${ALL_LIBRARIES})
install(TARGETS CGDemo RUNTIME DESTINATION bin/CGDemo)
install(DIRECTORY shaders/CGDemo DESTINATION bin/CGDemo/shaders)
# Project info
cmake_minimum_required(VERSION 3.11)
project(CSHook)

add_library(CSHook SHARED ${CMAKE_CURRENT_SOURCE_DIR}/src/main.cpp)

# Unicode for Windows (wchar_t)
ADD_DEFINITIONS(-DUNICODE)
ADD_DEFINITIONS(-D_UNICODE)
# Dont do me like this windows
ADD_DEFINITIONS(-DNOMINMAX)

set(CMAKE_SYSTEM_PROCESSOR "i686")

# Statically link all libraries
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -m32" CACHE STRING "c++ flags")
set(CMAKE_C_FLAGS   "${CMAKE_C_FLAGS} -m32" CACHE STRING "c flags")

set_target_properties(CSHook PROPERTIES LINK_SEARCH_START_STATIC 1)
set_target_properties(CSHook PROPERTIES LINK_SEARCH_END_STATIC 1)
set(CMAKE_FIND_LIBRARY_SUFFIXES ".a")

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -static-libgcc -static-libstdc++ -static")

# Debugging stuff
add_compile_options(-Wall -Wextra -Wpedantic)

#Language feature
target_compile_features(CSHook PUBLIC cxx_std_17)

#Libraries
find_library(DWMAPI_LIBRARIES dwmapi libdwmapi)                 # Windows DWMAPI for DwmExtendFrameIntoClientArea
find_library(PSAPI_LIBRARIES psapi libpsapi)                    # Windows PSAPI for GetModuleInformation
find_package(OpenGL)                                            # OpenGL

set(
    HEADER_ONLY_LIBS                                            # Libraries that require only headers

    "${CMAKE_CURRENT_SOURCE_DIR}/dependencies/glad/include"     # GLAD (OpenGL loader)
#    "${CMAKE_CURRENT_SOURCE_DIR}/dependencies/sol2/include"     # Sol2 (Lua)
#    "${CMAKE_CURRENT_SOURCE_DIR}/dependencies/glm/include"      # GLM
#    "${CMAKE_CURRENT_SOURCE_DIR}/dependencies/stb/include"      # STB image and truetype
)

set(
    LIB_HEADERS                                                 # Libraries header directories

#    ${LUA_INCLUDE_DIR}                                          # Lua headers
)

include_directories(CSHook ${LIB_HEADERS} ${HEADER_ONLY_LIBS}) # Including library headers to project


add_subdirectory("${CMAKE_CURRENT_SOURCE_DIR}/src/HLSDK/")
add_subdirectory("${CMAKE_CURRENT_SOURCE_DIR}/src/Hooks/")
add_subdirectory("${CMAKE_CURRENT_SOURCE_DIR}/src/Memory/")
add_subdirectory("${CMAKE_CURRENT_SOURCE_DIR}/src/ImGui/")
add_subdirectory("${CMAKE_CURRENT_SOURCE_DIR}/src/Utils/")
add_subdirectory("${CMAKE_CURRENT_SOURCE_DIR}/src/Features/")

target_link_libraries(
    CSHook                                                     # Linking various libraries

    ${DWMAPI_LIBRARIES}                                         # DWMAPI
    ${PSAPI_LIBRARIES}                                          # PSAPI
    ${OPENGL_LIBRARIES}                                         # OpenGL
)


# Installation
install(
    TARGETS                                                     # Setting the output directory
        CSHook
    
    RUNTIME
        DESTINATION
            "${CMAKE_CURRENT_SOURCE_DIR}/bin"
)
#file(
#    COPY                                                        # Copying LuaJIT header files to /bin/
#        "${CMAKE_CURRENT_SOURCE_DIR}/dependencies/LuaJIT-2.0.5/lib/x86/lua51.dll"
#
#    DESTINATION
#        "${CMAKE_CURRENT_SOURCE_DIR}/bin/"
#)
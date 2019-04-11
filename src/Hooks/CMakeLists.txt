target_sources(
    CSHook

    PUBLIC
        "${CMAKE_CURRENT_LIST_DIR}/hooks.cpp"

        "${CMAKE_CURRENT_LIST_DIR}/hooks.hpp"
        "${CMAKE_CURRENT_LIST_DIR}/Hooks/studio.hpp"
        "${CMAKE_CURRENT_LIST_DIR}/Hooks/messages.hpp"
        "${CMAKE_CURRENT_LIST_DIR}/Hooks/commands.hpp"
        "${CMAKE_CURRENT_LIST_DIR}/Hooks/netchan.hpp"
        "${CMAKE_CURRENT_LIST_DIR}/Hooks/client.hpp"
        "${CMAKE_CURRENT_LIST_DIR}/Hooks/opengl.hpp"
        "${CMAKE_CURRENT_LIST_DIR}/Hooks/windows.hpp"
)
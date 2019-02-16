target_sources(
    CSHook

    PUBLIC
        "${CMAKE_CURRENT_LIST_DIR}/hooks.cpp"

        "${CMAKE_CURRENT_LIST_DIR}/hooks.hpp"
        "${CMAKE_CURRENT_LIST_DIR}/studio.hpp"
        "${CMAKE_CURRENT_LIST_DIR}/messages.hpp"
        "${CMAKE_CURRENT_LIST_DIR}/commands.hpp"
        "${CMAKE_CURRENT_LIST_DIR}/netchan.hpp"
)
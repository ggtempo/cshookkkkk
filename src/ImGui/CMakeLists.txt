target_sources(
    CSHook

    PUBLIC
        "${CMAKE_CURRENT_LIST_DIR}/imgui_demo.cpp"
        "${CMAKE_CURRENT_LIST_DIR}/imgui_draw.cpp"
        "${CMAKE_CURRENT_LIST_DIR}/imgui_impl.cpp"
        "${CMAKE_CURRENT_LIST_DIR}/imgui_widgets.cpp"
        "${CMAKE_CURRENT_LIST_DIR}/imgui.cpp"

        "${CMAKE_CURRENT_LIST_DIR}/imconfig.h"
        "${CMAKE_CURRENT_LIST_DIR}/imgui_internal.h"
        "${CMAKE_CURRENT_LIST_DIR}/imgui_impl.h"
        "${CMAKE_CURRENT_LIST_DIR}/imstb_truetype.h"
        "${CMAKE_CURRENT_LIST_DIR}/imstb_textedit.h"
        "${CMAKE_CURRENT_LIST_DIR}/imstb_rectpack.h"
        "${CMAKE_CURRENT_LIST_DIR}/imgui.h"
        "${CMAKE_CURRENT_LIST_DIR}/imgui_custom.hpp"
)
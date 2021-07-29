# Copyright(c) 2021 - Electro Team - All rights reserved

add_library(imgui
    "imconfig.h"
    "imgui.h"
    "imgui.cpp"
    "imgui_stdlib.cpp"
    "imgui_stdlib.h"
    "imgui_draw.cpp"
    "imgui_internal.h"
    "imgui_widgets.cpp"
    "imstb_rectpack.h"
    "imstb_textedit.h"
    "imstb_truetype.h"
    "imgui_demo.cpp"
    "imgui_tables.cpp"
    "ImGuizmo.cpp"
    "ImGuizmo.h"
)
set_target_properties (imgui PROPERTIES FOLDER Dependencies)
message(STATUS "Configuring ImGui... done!")
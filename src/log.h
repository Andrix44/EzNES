#pragma once

#include <string>

#include "imgui/imgui.h"


class Logging {
public:
    bool scroll_enabled = true;

    Logging();
    void AddLog(std::string entry);
    void Draw(bool *show_log_window);

private:
    ImGuiTextBuffer buff;
    bool scroll_to_bottom;
};

extern Logging log_helper;
#pragma once

#include <string>

#include "imgui/imgui.h"


class Logging {
public:
    bool scroll_enabled = true;

    void AddLog(std::string entry);
    void Clear();
    void Draw(bool *show_log_window);

private:
    ImGuiTextBuffer buff;
    bool scroll_to_bottom = false;
};

extern Logging log_helper;

#pragma once

#include <string>

#include "imgui/imgui.h"


class Logging {
public:
    bool scroll_enabled = true;

    void AddLog(std::string entry);
    void Clear();
    void Draw(bool *show_log_window);
    std::string GetLastMessage();

private:
    ImGuiTextBuffer buff;
    bool scroll_to_bottom = false;
    std::string last_message{};
};

extern Logging log_helper;

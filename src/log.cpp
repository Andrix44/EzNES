#include "log.h"


void Logging::AddLog(std::string entry) {
    buff.append(entry.c_str());
    if (scroll_enabled) {
        scroll_to_bottom = true;
    }
}

void Logging::Clear() {
    buff.clear();
}

void Logging::Draw(bool *show_log_window) {
    if (!ImGui::Begin("Log", show_log_window)) {
        ImGui::End();
        return;
    }

    ImGui::Separator();
    ImGui::BeginChild("scrolling", ImVec2(0, 0), false, ImGuiWindowFlags_HorizontalScrollbar);

    const char* buff_begin = buff.begin();
    const char* buff_end = buff.end();

    ImGui::TextUnformatted(buff_begin, buff_end);

    if (scroll_to_bottom) {
        ImGui::SetScrollHereY(1.0f);
    }
    scroll_to_bottom = false;
    ImGui::EndChild();
    ImGui::End();
}

Logging log_helper;

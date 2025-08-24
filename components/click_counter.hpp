#pragma once

#include <cstdint>

#include "imgui.h"

namespace components {

struct ClickCounter {
    static inline int count{0};

    static void render() {
        if (ImGui::Button("Button")) {
            count++;
        }
        ImGui::SameLine();
        ImGui::Text("counter = %d", count);
    }
};
}  // namespace components
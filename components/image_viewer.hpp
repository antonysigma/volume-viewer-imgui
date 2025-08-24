#pragma once
#include <optional>

#include "data_models/frame2d.h"
#include "imgui.h"
#include "view_models/scale.hpp"

namespace components {

struct ImageViewer {
    static inline std::optional<view_models::Frame2D> frame{std::nullopt};
    static void render() {
        using view_models::scale;
        if (!frame) {
            return;
        }

        ImGui::Begin("New image");
        ImGui::Image(frame->texture, ImVec2(frame->width * scale, frame->height * scale));
        ImGui::End();
    }
};

}  // namespace components
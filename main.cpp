#include <opencv2/opencv.hpp>
#include <Windows.h>
#include <thread>
#include <atomic>
#include "overlay.h"
#include "detection_lib.h"

int fov = 500;
int x_offset = 0;
int y_offset = 5;

cv::Scalar color_lower(55, 200, 200);
cv::Scalar color_upper(70, 255, 255);

std::atomic<DetectionLib::Target> detected_target;

void drawOverlay(int screenWidth, int screenHeight) {
    ImDrawList* draw_list = ImGui::GetBackgroundDrawList();

    DetectionLib::Target target = detected_target.load();
    if (target.valid && target.pos.x != -1 && target.pos.y != -1) {
        float boxSize = 30.0f;
        ImVec2 boxTopLeft((float)target.pos.x - boxSize / 2.0f, (float)target.pos.y - boxSize / 2.0f);
        ImVec2 boxBottomRight((float)target.pos.x + boxSize / 2.0f, (float)target.pos.y + boxSize / 2.0f);
        draw_list->AddRect(boxTopLeft, boxBottomRight, IM_COL32(255, 255, 255, 255), 0.0f, 0, 1.f);
    }
}

void detectionThreadFunc() {
    while (true) {
        int offsetX = 0, offsetY = 0;
        cv::Mat region = DetectionLib::CaptureFOVRegion(fov, offsetX, offsetY);
        DetectionLib::Target target = DetectionLib::FindTargetInRegion(
            region, fov, x_offset + offsetX, y_offset + offsetY, color_lower, color_upper
        );
        detected_target.store(target);
        std::this_thread::sleep_for(std::chrono::milliseconds(5));
    }
}

int main() {
    overlay::SetupWindow();
    if (!(overlay::CreateDeviceD3D(overlay::Window)))
        return 1;
    int screenWidth = GetSystemMetrics(SM_CXSCREEN);
    int screenHeight = GetSystemMetrics(SM_CYSCREEN);

    std::thread(detectionThreadFunc).detach();

    while (!overlay::ShouldQuit) {
        overlay::Render();
        drawOverlay(screenWidth, screenHeight);
        overlay::EndRender();
    }

    overlay::CloseOverlay();
    return 0;
}

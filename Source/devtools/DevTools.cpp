// Advanced Devtools UI for Axmol - Scene Inspector, Performance Profiling, and Debugging Tools
#define DEVTOOLS

#include "DevTools.h"
#include "ImGui/ImGuiPresenter.h"
#include "ImGui/imgui.h"

#include <fmt/format.h>
#include <mutex>
#include <queue>
#include <stack>
#include <unordered_map>
#include <chrono>
#include <array>

#include <axmol.h>

USING_NS_AX;
USING_NS_AX_EXT;

// ============================================================================
// Devtools State
// ============================================================================

static bool devtools_open = false;
static Node* selected_node = nullptr;
static Node* hovered_node = nullptr;
static bool show_node_creation = false;
static char search_filter[256] = "";

// Performance stats
static struct {
    std::array<float, 60> frame_times;
    size_t frame_idx = 0;
    float avg_fps = 0.f;
    float min_frame_time = 0.f;
    float max_frame_time = 0.f;
    int frame_count = 0;
    std::chrono::time_point<std::chrono::high_resolution_clock> last_update = std::chrono::high_resolution_clock::now();
} perf_stats;

// Panels state
static bool show_scene_panel = true;
static bool show_properties_panel = true;
static bool show_performance_panel = true;
static bool show_settings_panel = false;
static bool show_search_panel = true;

// Input state
static bool key_f11_pressed = false;

// ============================================================================
// Utility Functions
// ============================================================================

static Node* createNodeOfType(int type) {
    switch (type) {
        case 0: return Node::create();
        case 1: {
            auto spr = Sprite::create();
            spr->setContentSize({32, 32});
            return spr;
        }
        case 2: {
            auto label = Label::createWithSystemFont("Text", "Arial", 24);
            return label;
        }
        case 3: return DrawNode::create();
        default: return Node::create();
    }
}

static const char* getNodeName(Node* node)
{
    const char* name = typeid(*node).name() + 6;
    return name;
}

ImVec2 cocos_to_vec2(const ax::Point& a)
{
    const auto size = ImGui::GetMainViewport()->Size;
    const auto win_size = ax::Director::getInstance()->getWinSize();
    return {a.x / win_size.width * size.x, (1.f - a.y / win_size.height) * size.y};
}

void highlight(ax::Node* node, bool selected)
{
    auto& foreground = *ImGui::GetForegroundDrawList();

    const auto size = node->getContentSize();
    const auto anchor = node->getAnchorPoint();

    ax::Vec3 local_corners[4] = {
        {-anchor.x * size.width,            -anchor.y * size.height,            0.0f},
        {(1.0f - anchor.x) * size.width,    -anchor.y * size.height,            0.0f},
        {(1.0f - anchor.x) * size.width,    (1.0f - anchor.y) * size.height,    0.0f},
        {-anchor.x * size.width,            (1.0f - anchor.y) * size.height,    0.0f}
    };

    auto world = node->getNodeToWorldTransform();
    ImVec2 pts[4];
    for (int i = 0; i < 4; ++i) {
        ax::Vec3 wc;
        world.transformPoint(local_corners[i], &wc);
        pts[i] = cocos_to_vec2(ax::Vec2{wc.x, wc.y});
    }

    ImU32 col = selected ? IM_COL32(200, 200, 255, 60) : IM_COL32(255, 255, 255, 70);
    auto border = selected ? IM_COL32(80, 80, 180, 200) : IM_COL32(180, 180, 180, 200);
    foreground.AddQuadFilled(pts[0], pts[1], pts[2], pts[3], col);
    foreground.AddQuad(pts[0], pts[1], pts[2], pts[3], border, 1.0f);
}

// ============================================================================
// Performance Monitoring
// ============================================================================

static void updatePerformanceStats(float frame_time_ms)
{
    perf_stats.frame_times[perf_stats.frame_idx++ % 60] = frame_time_ms;
    perf_stats.frame_count++;

    float sum = 0.f;
    float min_val = FLT_MAX;
    float max_val = 0.f;
    for (float t : perf_stats.frame_times) {
        sum += t;
        min_val = std::min(min_val, t);
        max_val = std::max(max_val, t);
    }
    perf_stats.avg_fps = 1000.f / (sum / 60.f);
    perf_stats.min_frame_time = min_val;
    perf_stats.max_frame_time = max_val;
}

static void drawPerformancePanel()
{
    if (!show_performance_panel) return;

    ImGui::SetNextWindowSize(ImVec2(400, 300), ImGuiCond_FirstUseEver);
    if (ImGui::Begin("Performance##perf", &show_performance_panel, ImGuiWindowFlags_NoMove)) {
        ImGui::Text("FPS: %.1f", perf_stats.avg_fps);
        ImGui::SameLine(150);
        ImGui::Text("Min: %.2f ms", perf_stats.min_frame_time);
        ImGui::SameLine(300);
        ImGui::Text("Max: %.2f ms", perf_stats.max_frame_time);

        ImGui::Separator();
        ImGui::Text("Frame Time History");
        ImGui::PlotLines("##frametimes", perf_stats.frame_times.data(), 60, 
                         perf_stats.frame_idx % 60, nullptr, 0.f, 50.f, ImVec2(0, 80));

        ImGui::Separator();
        auto director = Director::getInstance();
        ImGui::Text("Display refresh: %.1f Hz", director->getAnimationInterval() > 0 
                    ? 1.0 / director->getAnimationInterval() : 0.f);
        
        auto win_size = director->getWinSize();
        ImGui::Text("Window: %.0f x %.0f", win_size.width, win_size.height);
        
        auto content_scale = director->getContentScaleFactor();
        ImGui::Text("Content Scale: %.2f", content_scale);
    }
    ImGui::End();
}

// ============================================================================
// Scene Graph & Properties
// ============================================================================

void drawProperties()
{
    if (selected_node == nullptr)
    {
        ImGui::Text("Select a node to edit its properties :-)");
        return;
    }

    ImGui::Separator();
    if (ImGui::Button("Delete", ImVec2(100, 0)))
    {
        selected_node->removeFromParentAndCleanup(true);
        selected_node = nullptr;
        return;
    }
    ImGui::Separator();

    ImGui::Text("Type: %s", getNodeName(selected_node));
    ImGui::Text("Addr: 0x%p", selected_node);
    
    static char node_name[256] = "";
    std::string name_str(selected_node->getName());
    if (ImGui::IsWindowAppearing()) {
        strcpy(node_name, name_str.c_str());
    }
    if (ImGui::InputText("##nodename", node_name, 256, ImGuiInputTextFlags_EnterReturnsTrue)) {
        selected_node->setName(node_name);
    }

    if (selected_node->getUserData())
    {
        ImGui::Text("User data: 0x%p", selected_node->getUserData());
    }

    int tag = selected_node->getTag();
    if (ImGui::InputInt("Tag", &tag)) {
        selected_node->setTag(tag);
    }

    ImGui::Separator();
    ImGui::Text("Transform");

    auto pos = selected_node->getPosition();
    float _pos[2] = {pos.x, pos.y};
    if (ImGui::DragFloat2("Position", _pos)) {
        selected_node->setPosition({_pos[0], _pos[1]});
    }

    float _scale[3] = {selected_node->getScale(), selected_node->getScaleX(), selected_node->getScaleY()};
    if (ImGui::DragFloat3("Scale", _scale, 0.025f)) {
        if (selected_node->getScale() != _scale[0])
            selected_node->setScale(_scale[0]);
        else
        {
            selected_node->setScaleX(_scale[1]);
            selected_node->setScaleY(_scale[2]);
        }
    }

    auto anchor = selected_node->getAnchorPoint();
    float _anch[2] = {anchor.x, anchor.y};
    if (ImGui::DragFloat2("Anchor Point", _anch)) {
        selected_node->setAnchorPoint({_anch[0], _anch[1]});
    }

    float rotation[3] = {selected_node->getRotation(), selected_node->getRotationSkewX(),
                        selected_node->getRotationSkewY()};
    if (ImGui::DragFloat3("Rotation", rotation, 1.0f))
    {
        if (selected_node->getRotation() != rotation[0])
            selected_node->setRotation(rotation[0]);
        else
        {
            selected_node->setRotationSkewX(rotation[1]);
            selected_node->setRotationSkewY(rotation[2]);
        }
    }

    ImGui::Separator();
    ImGui::Text("Appearance");

    auto content = selected_node->getContentSize();
    float _cont[2] = {content.x, content.y};
    if (ImGui::DragFloat2("Content Size", _cont)) {
        selected_node->setContentSize({_cont[0], _cont[1]});
    }

    int zOrder = selected_node->getLocalZOrder();
    if (ImGui::InputInt("Z Order", &zOrder)) {
        selected_node->setLocalZOrder(zOrder);
    }

    auto visible = selected_node->isVisible();
    if (ImGui::Checkbox("Visible", &visible)) {
        selected_node->setVisible(visible);
    }

    auto color = selected_node->getColor();
    float _color[4] = {color.r / 255.f, color.g / 255.f, color.b / 255.f, selected_node->getOpacity() / 255.f};
    if (ImGui::ColorEdit4("Color & Opacity", _color)) {
        selected_node->setColor({static_cast<GLubyte>(_color[0] * 255), static_cast<GLubyte>(_color[1] * 255),
                                static_cast<GLubyte>(_color[2] * 255)});
        selected_node->setOpacity(static_cast<int>(_color[3] * 255));
    }

    // Label-specific
    if (dynamic_cast<ax::LabelProtocol*>(selected_node) != nullptr)
    {
        ImGui::Separator();
        ImGui::Text("Label");
        
        auto labelNode = dynamic_cast<ax::LabelProtocol*>(selected_node);
        std::string labelStr(labelNode->getString());
        static char text[512];
        strcpy(text, labelStr.c_str());
        if (ImGui::InputTextMultiline("##text", text, 512, ImVec2(-1, 80))) {
            labelNode->setString(text);
        }
    }

    // Sprite-specific
    if (dynamic_cast<ax::Sprite*>(selected_node) != nullptr)
    {
        ImGui::Separator();
        ImGui::Text("Sprite");
        
        auto spr = dynamic_cast<ax::Sprite*>(selected_node);
        bool flipx = spr->isFlippedX(), flipy = spr->isFlippedY();
        if (ImGui::Checkbox("Flip X##spr", &flipx)) {
            spr->setFlippedX(flipx);
        }
        ImGui::SameLine();
        if (ImGui::Checkbox("Flip Y##spr", &flipy)) {
            spr->setFlippedY(flipy);
        }
    }

    // DrawNode-specific
    if (dynamic_cast<ax::DrawNode*>(selected_node) != nullptr)
    {
        ImGui::Separator();
        ImGui::Text("DrawNode");
        
        auto drawNode = dynamic_cast<ax::DrawNode*>(selected_node);
        if (ImGui::Button("Clear##draw")) {
            drawNode->clear();
        }
    }
}

static void generateTree(Node* node, unsigned int i = 0)
{
    std::string node_type = getNodeName(node);
    std::string node_display = fmt::format("[{}] {} ({})", i, 
        node->getName().empty() ? node_type : node->getName(), node_type);

    const auto childrenCount = node->getChildrenCount();
    auto flags = ImGuiTreeNodeFlags_SpanAvailWidth | ImGuiTreeNodeFlags_SpanFullWidth;
    if (selected_node == node)
        flags |= ImGuiTreeNodeFlags_Selected;
    if (childrenCount == 0)
        flags |= ImGuiTreeNodeFlags_Leaf;

    const bool is_open = ImGui::TreeNodeEx(node, flags, "%s", node_display.c_str());

    if (ImGui::IsItemClicked())
    {
        selected_node = node;
    }

    if (ImGui::BeginPopupContextItem(fmt::format("##nodecontext{}", (void*)node).c_str()))
    {
        if (ImGui::MenuItem("Delete"))
        {
            node->removeFromParentAndCleanup(true);
            if (selected_node == node) selected_node = nullptr;
        }
        ImGui::EndPopup();
    }

    if (ImGui::IsItemHovered())
        hovered_node = node;

    if (is_open)
    {
        if (childrenCount)
        {
            const auto& children = node->getChildren();
            unsigned int index = 0;
            for (const auto& child : children)
            {
                generateTree(child, index++);
            }
        }
        ImGui::TreePop();
    }
}

static void drawNodeCreation()
{
    if (ImGui::Button("Create Node##create", ImVec2(-1, 0)))
    {
        show_node_creation = !show_node_creation;
    }

    if (show_node_creation)
    {
        ImGui::Separator();
        ImGui::Text("Add Child Node:");
        
        if (!selected_node)
        {
            ImGui::TextColored(ImVec4(1, 1, 0, 1), "Select a parent node first!");
            return;
        }

        static int node_type = 0;
        ImGui::Combo("##nodetype", &node_type, "Empty Node\0Sprite\0Label\0DrawNode\0");

        if (ImGui::Button("Create##node", ImVec2(-1, 0)))
        {
            auto new_node = createNodeOfType(node_type);
            new_node->setPosition({50, 50});
            selected_node->addChild(new_node);
            show_node_creation = false;
        }
    }
}

static void drawScenePanel()
{
    if (!show_scene_panel) return;

    ImGui::SetNextWindowSize(ImVec2(600, 500), ImGuiCond_FirstUseEver);
    ImGui::SetNextWindowPos(ImVec2(50, 50), ImGuiCond_FirstUseEver);
    if (ImGui::Begin("Scene Graph##scene", &show_scene_panel)) {
        auto current = Director::getInstance()->getRunningScene();
        if (!current) {
            ImGui::TextColored(ImVec4(1, 0.4f, 0.4f, 1), "No active scene yet.");
        } else {
            std::string scene_name(current->getName());
            ImGui::Text("Scene: %s", scene_name.c_str());
            ImGui::SameLine();
            if (ImGui::Button("Refresh##scene")) {
                selected_node = nullptr;
            }

            ImGui::Separator();
            ImGui::BeginChild("explorer.tree", ImVec2(0, -50), true, ImGuiWindowFlags_HorizontalScrollbar);
            generateTree(current);
            ImGui::EndChild();

            ImGui::BeginChild("explorer.create", ImVec2(0, 50), true);
            drawNodeCreation();
            ImGui::EndChild();
        }
    }
    ImGui::End();
}

static void drawPropertiesPanel()
{
    if (!show_properties_panel) return;

    ImGui::SetNextWindowSize(ImVec2(500, 500), ImGuiCond_FirstUseEver);
    ImGui::SetNextWindowPos(ImVec2(700, 50), ImGuiCond_FirstUseEver);
    if (ImGui::Begin("Properties##props", &show_properties_panel)) {
        drawProperties();
    }
    ImGui::End();
}

// ============================================================================
// Settings & Debug Options
// ============================================================================

static void drawSettingsPanel()
{
    if (!show_settings_panel) return;

    ImGui::SetNextWindowSize(ImVec2(400, 300), ImGuiCond_FirstUseEver);
    if (ImGui::Begin("Settings##settings", &show_settings_panel)) {
        ImGui::Text("DevTools Settings");
        ImGui::Separator();

        static bool show_node_bounds = false;
        ImGui::Checkbox("Show Node Bounds", &show_node_bounds);

        static bool show_debug_draw = false;
        ImGui::Checkbox("Enable Debug Draw", &show_debug_draw);

        static bool pause_on_scene_change = false;
        ImGui::Checkbox("Pause on Scene Change", &pause_on_scene_change);

        ImGui::Separator();
        ImGui::Text("Panels:");
        ImGui::Checkbox("Scene Graph##p", &show_scene_panel);
        ImGui::Checkbox("Properties##p", &show_properties_panel);
        ImGui::Checkbox("Performance##p", &show_performance_panel);
    }
    ImGui::End();
}

// ============================================================================
// Main Draw Loop
// ============================================================================

static void draw()
{
#ifdef DEVTOOLS
    if (!devtools_open) return;

    hovered_node = nullptr;

    ImGui::SetNextWindowPos(ImVec2(20, 20), ImGuiCond_FirstUseEver);
    ImGui::SetNextWindowSize(ImVec2(300, 100), ImGuiCond_FirstUseEver);
    if (ImGui::Begin("DevTools##main", &devtools_open, ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoResize)) {
        ImGui::Text("Axmol DevTools (F11 to toggle)");
        ImGui::SameLine();
        ImGui::SetCursorPosX(ImGui::GetWindowWidth() - 100);
        if (ImGui::Button("Settings", ImVec2(80, 0))) {
            show_settings_panel = !show_settings_panel;
        }

        ImGui::Separator();
        ImGui::Checkbox("Scene##vis", &show_scene_panel);
        ImGui::SameLine();
        ImGui::Checkbox("Properties##vis", &show_properties_panel);
        ImGui::SameLine();
        ImGui::Checkbox("Performance##vis", &show_performance_panel);
    }
    ImGui::End();

    drawScenePanel();
    drawPropertiesPanel();
    drawPerformancePanel();
    drawSettingsPanel();

    if (selected_node)
        highlight(selected_node, true);
    if (hovered_node)
        highlight(hovered_node, false);
#endif
}

// ============================================================================
// Public API
// ============================================================================

void DevTools::open()
{
#ifdef DEVTOOLS
    devtools_open = true;
    ImGuiPresenter::getInstance()->addRenderLoop("#devtools", draw, nullptr);
#endif
}

void DevTools::close()
{
#ifdef DEVTOOLS
    devtools_open = false;
    ImGuiPresenter::getInstance()->removeRenderLoop("#devtools");
#endif
}

void DevTools::toggle()
{
#ifdef DEVTOOLS
    if (devtools_open) {
        close();
    } else {
        open();
    }
#endif
}

bool DevTools::isOpen()
{
    return devtools_open;
}

void DevTools::handleKeyPress(int keyCode)
{
    // F11 = 0x7A in Windows virtual key codes
    if (keyCode == 0x7A) {
        DevTools::toggle();
    }
}

void DevTools::update(float dt)
{
    static auto last_update = std::chrono::high_resolution_clock::now();
    auto now = std::chrono::high_resolution_clock::now();
    auto elapsed = std::chrono::duration<float, std::milli>(now - last_update).count();
    last_update = now;

    if (elapsed > 0.f) {
        updatePerformanceStats(elapsed);
    }
}

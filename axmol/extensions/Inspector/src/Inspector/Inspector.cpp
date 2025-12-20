#include "Inspector.h"
#include "ImGuiPresenter.h"
#include "axmol.h"
#include "2d/DrawNode.h"

#if __has_include(<cxxabi.h>)
#    define AX_HAS_CXXABI 1
#    include <cxxabi.h>
#endif

#include "fmt/format.h"
#include <memory>
#include "misc/cpp/imgui_stdlib.h"

// Forward declare LayoutMenu to avoid circular dependency
class LayoutMenu;
#include "../../../../Source/extras/LayoutMenu.h"

NS_AX_EXT_BEGIN

namespace
{
Inspector* g_instance = nullptr;
}

bool InspectorNodePropertyHandler::isSupportedType(Node* node)
{
    return true;
}

void InspectorNodePropertyHandler::drawProperties(Node* node)
{
    if (auto userData = node->getUserData(); userData)
    {
        ImGui::SameLine();
        ImGui::Text("User data: %p", userData);
    }

    auto pos      = node->getPosition();
    float _pos[2] = {pos.x, pos.y};
    ImGui::DragFloat2("Position", _pos);
    node->setPosition({_pos[0], _pos[1]});

    // need to use getScaleX() because of assert
    float _scale[3] = {node->getScaleX(), node->getScaleX(), node->getScaleY()};
    ImGui::DragFloat3("Scale", _scale, 0.025f);

    if (auto scale_0 = node->getScaleX(); scale_0 != _scale[0])
    {
        node->setScale(_scale[0]);
    }
    else
    {
        node->setScaleX(_scale[1]);
        node->setScaleY(_scale[2]);
    }

    float rotation[3] = {node->getRotationSkewX(), node->getRotationSkewX(),
                         node->getRotationSkewY()};
    if (ImGui::DragFloat3("Rotation", rotation, 1.0f))
    {
        if (node->getRotationSkewX() != rotation[0])
        {
            node->setRotation(rotation[0]);
        }
        else
        {
            node->setRotationSkewX(rotation[1]);
            node->setRotationSkewY(rotation[2]);
        }
    }

    auto contentSize = node->getContentSize();
    float _cont[2]   = {contentSize.x, contentSize.y};
    ImGui::DragFloat2("Content Size", _cont);
    node->setContentSize({_cont[0], _cont[1]});

    auto anchor    = node->getAnchorPoint();
    float _anch[2] = {anchor.x, anchor.y};
    ImGui::DragFloat2("Anchor Point", _anch);
    node->setAnchorPoint({_anch[0], _anch[1]});

    int localZOrder = node->getLocalZOrder();
    ImGui::InputInt("Local Z", &localZOrder);
    if (node->getLocalZOrder() != localZOrder)
    {
        node->setLocalZOrder(localZOrder);
    }

    float globalZOrder = node->getGlobalZOrder();
    ImGui::InputFloat("Global Z", &globalZOrder);
    if (node->getGlobalZOrder() != globalZOrder)
    {
        node->setGlobalZOrder(globalZOrder);
    }

    auto visible = node->isVisible();
    ImGui::Checkbox("Visible", &visible);
    if (visible != node->isVisible())
    {
        node->setVisible(visible);
    }

    auto color      = node->getColor();
    float _color[4] = {color.r / 255.f, color.g / 255.f, color.b / 255.f, node->getOpacity() / 255.f};
    ImGui::ColorEdit4("Color", _color);
    node->setColor({static_cast<GLubyte>(_color[0] * 255), static_cast<GLubyte>(_color[1] * 255),
                              static_cast<GLubyte>(_color[2] * 255)});
    node->setOpacity(static_cast<int>(_color[3] * 255.f));
}

bool InspectorSpritePropertyHandler::isSupportedType(Node* node)
{
    return dynamic_cast<Sprite*>(node) != nullptr;
}

void InspectorSpritePropertyHandler::drawProperties(Node* node)
{
    auto* sprite = dynamic_cast<ax::Sprite*>(node);

    ImGui::SameLine();
    bool flipx = sprite->isFlippedX();
    bool flipy = sprite->isFlippedY();
    ImGui::Checkbox("FlipX", &flipx);
    ImGui::SameLine();
    ImGui::Checkbox("FlipY", &flipy);
    sprite->setFlippedX(flipx);
    sprite->setFlippedY(flipy);

    auto texture = sprite->getTexture();
    ImGui::TextWrapped("Texture: %s", texture->getPath().c_str());

    ImGuiPresenter::getInstance()->image(sprite, ImVec2(256, 256));
}

bool InspectorLabelProtocolPropertyHandler::isSupportedType(Node* node)
{
    return dynamic_cast<LabelProtocol*>(node) != nullptr;
}

void InspectorLabelProtocolPropertyHandler::drawProperties(Node* node)
{
    auto label_node = dynamic_cast<LabelProtocol*>(node);

    std::string_view label = label_node->getString();
    std::string label_str(label);

    if (ImGui::InputTextMultiline("Text", &label_str, {0, 50}))
    {
        label_node->setString(label_str);
    }
}

bool InspectorLayoutMenuPropertyHandler::isSupportedType(Node* node)
{
    return dynamic_cast<LayoutMenu*>(node) != nullptr;
}

void InspectorLayoutMenuPropertyHandler::drawProperties(Node* node)
{
    auto* layout = dynamic_cast<LayoutMenu*>(node);
    if (!layout) return;

    ImGui::SeparatorText("Layout Settings");

    // Direction
    const char* directionItems[] = {"Row", "Column"};
    int currentDir = static_cast<int>(layout->getDirection());
    if (ImGui::Combo("Direction", &currentDir, directionItems, 2))
    {
        layout->setDirection(static_cast<LayoutMenu::Direction>(currentDir));
        layout->updateLayout();
    }

    // Order
    const char* orderItems[] = {"Natural", "Name Asc", "Name Desc", "Tag Asc", "Tag Desc", "Z Asc", "Z Desc"};
    int currentOrder = static_cast<int>(layout->getOrder());
    if (ImGui::Combo("Order", &currentOrder, orderItems, 7))
    {
        layout->setOrder(static_cast<LayoutMenu::Order>(currentOrder));
        layout->updateLayout();
    }

    // Gap
    float gap = layout->getGap();
    if (ImGui::DragFloat("Gap", &gap, 0.5f, 0.0f, 100.0f))
    {
        layout->setGap(gap);
        layout->updateLayout();
    }

    // AlignMain
    const char* alignMainItems[] = {"Start", "Center", "End", "Between", "Even"};
    int currentAlignMain = static_cast<int>(layout->getAlignMain());
    if (ImGui::Combo("Align Main", &currentAlignMain, alignMainItems, 5))
    {
        layout->setAlignMain(static_cast<LayoutMenu::AlignMain>(currentAlignMain));
        layout->updateLayout();
    }

    // AlignCross
    const char* alignCrossItems[] = {"Start", "Center", "End"};
    int currentAlignCross = static_cast<int>(layout->getAlignCross());
    if (ImGui::Combo("Align Cross", &currentAlignCross, alignCrossItems, 3))
    {
        layout->setAlignCross(static_cast<LayoutMenu::AlignCross>(currentAlignCross));
        layout->updateLayout();
    }

    // Wrap
    bool wrap = layout->getWrap();
    if (ImGui::Checkbox("Wrap", &wrap))
    {
        layout->setWrap(wrap);
        layout->updateLayout();
    }

    // ReverseMain
    bool revMain = layout->getReverseMain();
    if (ImGui::Checkbox("Reverse Main", &revMain))
    {
        layout->setReverseMain(revMain);
        layout->updateLayout();
    }

    // ReverseCross
    bool revCross = layout->getReverseCross();
    if (ImGui::Checkbox("Reverse Cross", &revCross))
    {
        layout->setReverseCross(revCross);
        layout->updateLayout();
    }

    // IgnoreInvisibleChildren
    bool ignoreInvis = layout->getIgnoreInvisibleChildren();
    if (ImGui::Checkbox("Ignore Invisible Children", &ignoreInvis))
    {
        layout->setIgnoreInvisibleChildren(ignoreInvis);
        layout->updateLayout();
    }

    // GrowCrossAxis
    bool growCross = layout->getGrowCrossAxis();
    if (ImGui::Checkbox("Grow Cross Axis", &growCross))
    {
        layout->setGrowCrossAxis(growCross);
        layout->updateLayout();
    }

    ImGui::Separator();

    // AutoScale
    bool autoScale = layout->getAutoScale();
    if (ImGui::Checkbox("Auto Scale", &autoScale))
    {
        layout->setAutoScale(autoScale);
        layout->updateLayout();
    }

    // AutoGrowAxis
    auto autoGrow = layout->getAutoGrowAxis();
    bool hasAutoGrow = autoGrow.has_value();
    float autoGrowVal = hasAutoGrow ? *autoGrow : 0.0f;
    
    if (ImGui::Checkbox("Auto Grow Axis", &hasAutoGrow))
    {
        layout->setAutoGrowAxis(hasAutoGrow ? std::optional<float>(100.0f) : std::nullopt);
        layout->updateLayout();
    }
    
    if (hasAutoGrow)
    {
        ImGui::SameLine();
        if (ImGui::DragFloat("##AutoGrowVal", &autoGrowVal, 1.0f, 0.0f, 1000.0f))
        {
            layout->setAutoGrowAxis(autoGrowVal);
            layout->updateLayout();
        }
    }

    if (ImGui::Button("Update Layout"))
    {
        layout->updateLayout();
    }
}

Inspector* Inspector::getInstance()
{
    if (g_instance == nullptr)
    {
        g_instance = new Inspector();
        g_instance->init();
    }
    return g_instance;
}

void Inspector::destroyInstance()
{
    if (g_instance)
    {
        g_instance->close();
        g_instance->cleanup();
        delete g_instance;
        g_instance = nullptr;
    }
}

void Inspector::setFontPath(std::string_view fontPath)
{
    _fontPath = std::string(fontPath);
}

void Inspector::setFontSize(float fontSize)
{
    _fontSize = fontSize;
}

void Inspector::init()
{
    _fontPath = "fonts/arial.ttf";
    _fontSize = ImGuiPresenter::DEFAULT_FONT_SIZE;

    addPropertyHandler("__NODE__", std::make_unique<InspectorNodePropertyHandler>());
    addPropertyHandler("__SPRITE__", std::make_unique<InspectorSpritePropertyHandler>());
    addPropertyHandler("__LABEL_PROTOCOL__", std::make_unique<InspectorLabelProtocolPropertyHandler>());
    addPropertyHandler("__LAYOUT_MENU__", std::make_unique<InspectorLayoutMenuPropertyHandler>());

    // Default pages: Tree, Preview, Settings
    addPage("tree", std::unique_ptr<Page>(nullptr));
    addPage("preview", std::unique_ptr<Page>(nullptr));
    addPage("settings", std::unique_ptr<Page>(nullptr));

    _beforeNewSceneEventListener = Director::getInstance()->getEventDispatcher()->addCustomEventListener(
        Director::EVENT_BEFORE_SET_NEXT_SCENE, [this](EventCustom*) {
            if (!_autoAddToScenes)
                return;

        getInstance()->close();
    });
    _afterNewSceneEventListener = Director::getInstance()->getEventDispatcher()->addCustomEventListener(
        Director::EVENT_AFTER_SET_NEXT_SCENE, [this](EventCustom*) {
            if (!_autoAddToScenes)
                return;

        getInstance()->openForCurrentScene();
    });
}

void Inspector::cleanup()
{
    _propertyHandlers.clear();
    auto* eventDispatcher = Director::getInstance()->getEventDispatcher();
    eventDispatcher->removeEventListener(_beforeNewSceneEventListener);
    eventDispatcher->removeEventListener(_afterNewSceneEventListener);

    _beforeNewSceneEventListener = nullptr;
    _afterNewSceneEventListener = nullptr;
}

#if AX_TARGET_PLATFORM == AX_PLATFORM_WIN32

std::string Inspector::demangle(const char* name)
{
    // works because msvc's typeid().name() returns undecorated name
    // typeid(Node).name() == "class ax::Node"
    // the + 6 gets rid of the class prefix
    // "class ax::Node" + 6 == "ax::Node"
    return { name + 6 };
}

#elif AX_HAS_CXXABI

std::string Inspector::demangle(const char* mangled_name)
{
    int status = -4;
    std::unique_ptr<char, void (*)(void*)> res{abi::__cxa_demangle(mangled_name, nullptr, nullptr, &status), std::free};
    return (status == 0) ? res.get() : mangled_name;
}

#else

std::string Inspector::demangle(const char* name)
{
    return { name };
}

#endif

std::string Inspector::getNodeTypeName(Node* node)
{
    return demangle(typeid(*node).name());
}

void Inspector::drawTreeRecursive(Node* node, int index)
{
    if (!_showInvisible && !node->isVisible())
    {
        return;
    }

    std::string str = fmt::format("[{}] {}", index, getNodeTypeName(node));

    if (node->getTag() != -1)
    {
        fmt::format_to(std::back_inserter(str), " ({})", node->getTag());
    }

    const auto nodeName = node->getName();
    if (!nodeName.empty())
    {
        fmt::format_to(std::back_inserter(str), " \"{}\"", nodeName);
    }

    const auto childrenCount = node->getChildrenCount();
    if (childrenCount != 0)
    {
        fmt::format_to(std::back_inserter(str), " {{{}}}", childrenCount);
    }

    auto flags = ImGuiTreeNodeFlags_SpanAvailWidth | ImGuiTreeNodeFlags_SpanFullWidth;
    if (_selected_node == node)
    {
        flags |= ImGuiTreeNodeFlags_Selected;
    }
    if (node->getChildrenCount() == 0)
    {
        flags |= ImGuiTreeNodeFlags_Leaf;
    }

    const bool is_open = ImGui::TreeNodeEx(node, flags, "%s", str.c_str());

    // Track hovered node
    if (ImGui::IsItemHovered())
    {
        _hovered_node = node;
    }

    if (ImGui::IsItemClicked())
    {
        if (_lockSelection)
        {
            // ignore selection changes when locked
        }
        else if (node == _selected_node && ImGui::GetIO().KeyAlt)
        {
            _selected_node = nullptr;
        }
        else
        {
            _selected_node = node;
        }
    }

    if (is_open)
    {
        const auto &children = node->getChildren();
        for (int i = 0; auto* child : children)
        {
            if(!child)
            {
                continue;
            }
            drawTreeRecursive(child, i);
            i++;
        }
        ImGui::TreePop();
    }
}

void Inspector::drawProperties()
{
    if (_selected_node == nullptr)
    {
        ImGui::Text("Select a node to edit its properties :-)");
        return;
    }

    if (_selected_node->getReferenceCount() <= 1)
    {
        // Node no longer exists in the scene, and we're holding the only reference to it, so release it
        _selected_node = nullptr;
        return;
    }

    if (ImGui::Button("Delete"))
    {
        _selected_node->removeFromParentAndCleanup(true);
        _selected_node = nullptr;
        return;
    }

    // ImGui::SameLine();
    //
    // if (ImGui::Button("Add Child"))
    //{
    //     ImGui::OpenPopup("Add Child");
    // }
    //
    // if (ImGui::BeginPopupModal("Add Child"))
    //{
    //     static int item = 0;
    //     ImGui::Combo("Node", &item, "Node\0LabelBMFont\0LabelTTF\0Sprite\0MenuItemSpriteExtra\0");
    //
    //     static int tag = -1;
    //     ImGui::InputInt("Tag", &tag);
    //
    //     static char text[256]{0};
    //     if (item == 1)
    //     {
    //         static char labelFont[256]{0};
    //         ImGui::InputText("Text", text, 256);
    //         ImGui::InputText("Font", labelFont, 256);
    //     }
    //     static int fontSize = 20;
    //     if (item == 2)
    //     {
    //         ImGui::InputText("Text", text, 256);
    //         ImGui::InputInt("Font Size", &fontSize);
    //     }
    //     static bool frame = false;
    //     if (item == 3 || item == 4)
    //     {
    //         ImGui::InputText("Texture", text, 256);
    //         ImGui::Checkbox("Frame", &frame);
    //     }
    //
    //     ImGui::Separator();
    //     ImGui::SameLine();
    //     if (ImGui::Button("Cancel"))
    //     {
    //         ImGui::CloseCurrentPopup();
    //     }
    //     ImGui::EndPopup();
    // }

    ImGui::Text("Addr: %p", _selected_node.get());

    for (auto&& propertyHandler : _propertyHandlers)
    {
        if (propertyHandler.second->isSupportedType(_selected_node))
        {
            propertyHandler.second->drawProperties(_selected_node);
        }
    }
}

void Inspector::openForScene(Scene* target)
{
    if (_target == target)
        return;

    _target = target;

    if (_target == nullptr)
    {
        close();
        return;
    }

    // Create bounds visualization DrawNode
    if (!_boundsDrawNode)
    {
        _boundsDrawNode = DrawNode::create();
        _boundsDrawNode->setGlobalZOrder(10000); // Render on top
    }
    _target->addChild(_boundsDrawNode);

    auto* presenter = ImGuiPresenter::getInstance();
    presenter->addFont(FileUtils::getInstance()->fullPathForFilename(_fontPath), _fontSize);
    presenter->enableDPIScale();
    applyTheme();
    presenter->addRenderLoop("#insp", AX_CALLBACK_0(Inspector::mainLoop , this), target);
}

void Inspector::openForCurrentScene()
{
    openForScene(Director::getInstance()->getRunningScene());
}

void Inspector::close()
{
    _selected_node = nullptr;
    _hovered_node = nullptr;

    if (_boundsDrawNode)
    {
        _boundsDrawNode->removeFromParent();
        _boundsDrawNode = nullptr;
    }

    _target = nullptr;

    auto presenter = ImGuiPresenter::getInstance();
    presenter->removeRenderLoop("#insp");
    presenter->clearFonts();
}

bool Inspector::isVisible() const
{
    return _target != nullptr;
}

bool Inspector::addPropertyHandler(std::string_view handlerId, std::unique_ptr<InspectPropertyHandler> handler)
{
    auto result = _propertyHandlers.try_emplace(std::string(handlerId), std::move(handler));
    return result.second;
}

void Inspector::removePropertyHandler(const std::string& handlerId)
{
    _propertyHandlers.erase(handlerId);
}

void Inspector::setAutoAddToScenes(bool autoAdd)
{
    if (_autoAddToScenes == autoAdd)
        return;

    _autoAddToScenes = autoAdd;
    close();

    if (_autoAddToScenes)
    {
        openForCurrentScene();
    }
}

void Inspector::mainLoop()
{
    if(!_target)
    {
        close();
        return;
    }

    // Reset hovered node at the start of each frame
    _hovered_node = nullptr;

    if (ImGui::Begin("Inspector"))
    {
        const auto avail = ImGui::GetContentRegionAvail();
        // Tabs (Tree, Preview, Settings)
        if (ImGui::BeginTabBar("insp.tabs"))
        {
            if (ImGui::BeginTabItem("Tree"))
            {
                _activeTab = 0;
                drawTreePage();
                ImGui::EndTabItem();
            }
            if (ImGui::BeginTabItem("Preview"))
            {
                _activeTab = 1;
                drawPreviewPage();
                ImGui::EndTabItem();
            }
            if (ImGui::BeginTabItem("Settings"))
            {
                _activeTab = 2;
                drawSettingsPage();
                ImGui::EndTabItem();
            }
            ImGui::EndTabBar();
        }
    }
    ImGui::End();

    // Update bounds visualization
    updateBoundsVisualization();
}
bool Inspector::addPage(std::string_view id, std::unique_ptr<Page> page)
{
    // Allow nullptr pages for default built-ins handled by draw*Page
    auto res = _pages.try_emplace(std::string(id), std::move(page));
    return res.second;
}

void Inspector::removePage(const std::string& id)
{
    _pages.erase(id);
}

void Inspector::drawTreePage()
{
    const auto avail = ImGui::GetContentRegionAvail();
    if (ImGui::BeginChild("insp.tree.left", ImVec2(avail.x * 0.5f, 0), false, ImGuiWindowFlags_HorizontalScrollbar))
    {
        drawTreeRecursive(_target);
    }
    ImGui::EndChild();

    ImGui::SameLine();

    if (ImGui::BeginChild("insp.tree.right"))
    {
        drawProperties();
    }
    ImGui::EndChild();
}

void Inspector::drawPreviewPage()
{
    if (!_selected_node)
    {
        ImGui::TextUnformatted("Select a node to preview.");
        return;
    }

    // If the selected node is a Sprite, show its texture
    if (auto* sprite = dynamic_cast<ax::Sprite*>(_selected_node.get()))
    {
        auto tex = sprite->getTexture();
        std::string path = tex ? tex->getPath() : "<none>";
        ImGui::Text("Texture: %s", path.c_str());
        auto cs = sprite->getContentSize();
        ImGui::Text("Size: %.0fx%.0f", cs.width, cs.height);

        ImGui::Separator();
        ImGui::Checkbox("Fit to window", &_previewFit);
        ImGui::SameLine();
        ImGui::SliderFloat("Zoom", &_previewZoom, 0.1f, 8.0f, "%.2fx");

        ImVec2 avail = ImGui::GetContentRegionAvail();
        ImVec2 drawSize = avail;
        if (_previewFit && cs.x > 0 && cs.y > 0) {
            float scale = std::min(avail.x / cs.width, avail.y / cs.height);
            drawSize = ImVec2(cs.width * scale, cs.height * scale);
        } else {
            drawSize = ImVec2(cs.width * _previewZoom, cs.height * _previewZoom);
        }

        // Checkerboard background
        ImVec2 p0 = ImGui::GetCursorScreenPos();
        ImVec2 p1 = ImVec2(p0.x + drawSize.x, p0.y + drawSize.y);
        auto* dl = ImGui::GetWindowDrawList();
        const float tile = 10.0f;
        for (float y = p0.y; y < p1.y; y += tile) {
            for (float x = p0.x; x < p1.x; x += tile) {
                bool dark = (static_cast<int>((x - p0.x) / tile) + static_cast<int>((y - p0.y) / tile)) % 2 == 0;
                dl->AddRectFilled(ImVec2(x, y), ImVec2(std::min(x + tile, p1.x), std::min(y + tile, p1.y)),
                    dark ? IM_COL32(180,180,180,255) : IM_COL32(220,220,220,255));
            }
        }

        // Draw sprite via presenter at requested size
        ImGui::SetCursorScreenPos(p0);
        ImGuiPresenter::getInstance()->image(sprite, drawSize);
    }
    else
    {
        ImGui::Text("Type: %s", getNodeTypeName(_selected_node.get()).c_str());
        auto cs = _selected_node->getContentSize();
        ImGui::Text("Content Size: %.0fx%.0f", cs.width, cs.height);
        ImGui::TextUnformatted("No specific preview available.");
    }
}

void Inspector::drawSettingsPage()
{
    ImGui::TextUnformatted("Inspector Settings");
    ImGui::Separator();

    // Font settings
    {
        char fontPathBuf[260] = {};
        std::strncpy(fontPathBuf, _fontPath.c_str(), sizeof(fontPathBuf) - 1);
        if (ImGui::InputText("Font Path", fontPathBuf, sizeof(fontPathBuf)))
        {
            setFontPath(fontPathBuf);
            rebuildFonts();
        }
    }
    {
        float fs = _fontSize;
        if (ImGui::DragFloat("Font Size", &fs, 0.25f, 8.0f, 64.0f, "%.2f"))
        {
            setFontSize(fs);
            rebuildFonts();
        }
    }

    if (ImGui::Checkbox("Dark Theme", &_darkTheme))
    {
        applyTheme();
    }

    if (ImGui::DragFloat("DPI Scale", &_dpiScale, 0.05f, 0.5f, 3.0f, "%.2f"))
    {
        applyDPIScale();
    }

    // Auto add to scenes
    bool autoAdd = _autoAddToScenes;
    if (ImGui::Checkbox("Auto Add To Scenes", &autoAdd))
    {
        setAutoAddToScenes(autoAdd);
    }

    // Visibility / selection behaviour
    ImGui::Checkbox("Show Invisible Nodes", &_showInvisible);
    ImGui::Checkbox("Lock Selection", &_lockSelection);
    ImGui::Checkbox("Show Bounds", &_showBounds);
}

void Inspector::applyTheme()
{
    if (!ImGui::GetCurrentContext()) return;
    if (_darkTheme) ImGui::StyleColorsDark();
    else ImGui::StyleColorsLight();
}

void Inspector::rebuildFonts()
{
    auto* presenter = ImGuiPresenter::getInstance();
    presenter->clearFonts();
    presenter->addFont(FileUtils::getInstance()->fullPathForFilename(_fontPath), _fontSize);
}

void Inspector::applyDPIScale()
{
    ImGuiPresenter::getInstance()->enableDPIScale(_dpiScale);
}

void Inspector::updateBoundsVisualization()
{
    if (!_boundsDrawNode || !_showBounds)
    {
        if (_boundsDrawNode)
            _boundsDrawNode->clear();
        return;
    }

    _boundsDrawNode->clear();

    // Draw bounds for hovered node (lighter blue)
    if (_hovered_node && _hovered_node != _selected_node)
    {
        auto worldBounds = _hovered_node->getBoundingBox();
        auto parent = _hovered_node->getParent();
        if (parent)
        {
            // Convert to world coordinates
            Vec2 corners[4] = {
                parent->convertToWorldSpace(Vec2(worldBounds.getMinX(), worldBounds.getMinY())),
                parent->convertToWorldSpace(Vec2(worldBounds.getMaxX(), worldBounds.getMinY())),
                parent->convertToWorldSpace(Vec2(worldBounds.getMaxX(), worldBounds.getMaxY())),
                parent->convertToWorldSpace(Vec2(worldBounds.getMinX(), worldBounds.getMaxY()))
            };

            // Draw bounds rectangle
            _boundsDrawNode->drawLine(corners[0], corners[1], Color4F(0.5f, 0.7f, 1.0f, 0.8f));
            _boundsDrawNode->drawLine(corners[1], corners[2], Color4F(0.5f, 0.7f, 1.0f, 0.8f));
            _boundsDrawNode->drawLine(corners[2], corners[3], Color4F(0.5f, 0.7f, 1.0f, 0.8f));
            _boundsDrawNode->drawLine(corners[3], corners[0], Color4F(0.5f, 0.7f, 1.0f, 0.8f));

            // Draw anchor point dot
            auto anchorInWorld = _hovered_node->convertToWorldSpace(Vec2::ZERO);
            _boundsDrawNode->drawSolidCircle(anchorInWorld, 4.0f, 0, 16, Color4F::WHITE);
        }
    }

    // Draw bounds for selected node (darker blue, on top)
    if (_selected_node)
    {
        auto worldBounds = _selected_node->getBoundingBox();
        auto parent = _selected_node->getParent();
        if (parent)
        {
            // Convert to world coordinates
            Vec2 corners[4] = {
                parent->convertToWorldSpace(Vec2(worldBounds.getMinX(), worldBounds.getMinY())),
                parent->convertToWorldSpace(Vec2(worldBounds.getMaxX(), worldBounds.getMinY())),
                parent->convertToWorldSpace(Vec2(worldBounds.getMaxX(), worldBounds.getMaxY())),
                parent->convertToWorldSpace(Vec2(worldBounds.getMinX(), worldBounds.getMaxY()))
            };

            // Draw bounds rectangle with thicker lines
            _boundsDrawNode->drawLine(corners[0], corners[1], Color4F(0.2f, 0.4f, 1.0f, 1.0f));
            _boundsDrawNode->drawLine(corners[1], corners[2], Color4F(0.2f, 0.4f, 1.0f, 1.0f));
            _boundsDrawNode->drawLine(corners[2], corners[3], Color4F(0.2f, 0.4f, 1.0f, 1.0f));
            _boundsDrawNode->drawLine(corners[3], corners[0], Color4F(0.2f, 0.4f, 1.0f, 1.0f));

            // Draw anchor point dot
            auto anchorInWorld = _selected_node->convertToWorldSpace(Vec2::ZERO);
            _boundsDrawNode->drawSolidCircle(anchorInWorld, 5.0f, 0, 16, Color4F::WHITE);
        }
    }
}

NS_AX_EXT_END

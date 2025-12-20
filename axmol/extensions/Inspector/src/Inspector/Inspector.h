#pragma once

#include <memory>
#include "extensions/ExtensionMacros.h"
#include "base/Config.h"
#include <string>
#include <unordered_map>
#include "EventListenerCustom.h"
#include "RefPtr.h"
#include "2d/Node.h"
#include "2d/DrawNode.h"

namespace ax
{
class Node;
class Scene;
}

NS_AX_EXT_BEGIN

class InspectPropertyHandler
{
public:
    virtual ~InspectPropertyHandler() = default;
    virtual bool isSupportedType(Node* node) = 0;
    virtual void drawProperties(Node* node) = 0;
};

class InspectorNodePropertyHandler : public InspectPropertyHandler
{
public:
    ~InspectorNodePropertyHandler() override = default;
    bool isSupportedType(Node* node) override;
    void drawProperties(Node* node) override;
};

class InspectorSpritePropertyHandler : public InspectPropertyHandler
{
public:
    ~InspectorSpritePropertyHandler() override = default;
    bool isSupportedType(Node* node) override;
    void drawProperties(Node* node) override;
};

class InspectorLabelProtocolPropertyHandler : public InspectPropertyHandler
{
public:
    ~InspectorLabelProtocolPropertyHandler() override = default;
    bool isSupportedType(Node* node) override;
    void drawProperties(Node* node) override;
};

class InspectorLayoutMenuPropertyHandler : public InspectPropertyHandler
{
public:
    ~InspectorLayoutMenuPropertyHandler() override = default;
    bool isSupportedType(Node* node) override;
    void drawProperties(Node* node) override;
};

class Inspector
{
  public:
    static Inspector* getInstance();
    static void destroyInstance();
    static std::string getNodeTypeName(Node*);
    static std::string demangle(const char* mangled);
    void openForScene(Scene*);
    void openForCurrentScene();
    void close();
    bool isVisible() const;

    bool addPropertyHandler(std::string_view handlerId, std::unique_ptr<InspectPropertyHandler> handler);
    void removePropertyHandler(const std::string& handlerId);

    void setAutoAddToScenes(bool autoAdd);

    std::string_view getFontPath() const { return _fontPath; }
    float getFontSize() const { return _fontSize; }
    void setFontPath(std::string_view fontPath);
    void setFontSize(float fontSize);
    class Page {
    public:
            virtual ~Page() = default;
            virtual const char* name() const = 0;
            virtual void draw(Inspector&) = 0;
    };
    bool addPage(std::string_view id, std::unique_ptr<Page> page);
    void removePage(const std::string& id);

  private:
    void init();
    void cleanup();
    void mainLoop();
        void drawTreeRecursive(Node*, int index = 0);
        void drawProperties();
        void drawTreePage();
        void drawPreviewPage();
        void drawSettingsPage();
    void applyTheme();
    void rebuildFonts();
    void applyDPIScale();
    void drawNodeBounds();
    void updateBoundsVisualization();

    ax::RefPtr<ax::Node> _selected_node = nullptr;
    ax::RefPtr<ax::Node> _hovered_node = nullptr;
    ax::Scene* _target = nullptr;
    ax::RefPtr<ax::DrawNode> _boundsDrawNode = nullptr;

    std::unordered_map<std::string, std::unique_ptr<InspectPropertyHandler>> _propertyHandlers;
    std::unordered_map<std::string, std::unique_ptr<Page>> _pages;
    RefPtr<EventListenerCustom> _beforeNewSceneEventListener;
    RefPtr<EventListenerCustom> _afterNewSceneEventListener;

    bool _autoAddToScenes = false;
    std::string _fontPath;
    float _fontSize;
    int _activeTab = 0; // 0: Tree, 1: Preview, 2: Settings (default pages)
    bool _showInvisible = true;
    bool _lockSelection = false;
    bool _darkTheme = true;
    float _dpiScale = 1.0f;
    float _previewZoom = 1.0f;
    bool _previewFit = true;
    bool _showBounds = true;
};

NS_AX_EXT_END

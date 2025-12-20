#include "LayoutMenu.h"

using ax::Vec2;
using ax::Size;

LayoutMenu* LayoutMenu::create() {
    auto p = new LayoutMenu();
    if (p && p->init()) {
        p->autorelease();
        return p;
    }
    delete p;
    return nullptr;
}

bool LayoutMenu::init() { return ax::Menu::init(); }

LayoutMenu* LayoutMenu::setDirection(Direction d) { _dir = d; return this; }
LayoutMenu* LayoutMenu::setOrder(Order o) { _order = o; return this; }
LayoutMenu* LayoutMenu::setGap(float g) { _gap = g; return this; }
LayoutMenu* LayoutMenu::setWrap(bool w) { _wrap = w; return this; }
LayoutMenu* LayoutMenu::setAlignMain(AlignMain a) { _alignMain = a; return this; }
LayoutMenu* LayoutMenu::setAlignCross(AlignCross a) { _alignCross = a; return this; }
LayoutMenu* LayoutMenu::setReverseMain(bool r) { _revMain = r; return this; }
LayoutMenu* LayoutMenu::setReverseCross(bool r) { _revCross = r; return this; }
LayoutMenu* LayoutMenu::setAutoScale(bool enable) { _autoScale = enable; return this; }
LayoutMenu* LayoutMenu::setDefaultScaleLimits(float minScale, float maxScale) { _minScale = minScale; _maxScale = maxScale; return this; }
LayoutMenu* LayoutMenu::setIgnoreInvisibleChildren(bool ignore) { _ignoreInvisible = ignore; return this; }
LayoutMenu* LayoutMenu::setGrowCrossAxis(bool grow) { _growCross = grow; return this; }
LayoutMenu* LayoutMenu::setAutoGrowAxis(std::optional<float> minLength) { _autoGrowAxis = minLength; return this; }

std::vector<ax::Node*> LayoutMenu::collectChildren() const {
    std::vector<ax::Node*> items;
    const auto& children = this->getChildren();
    items.reserve(children.size());
    for (auto* n : children) {
        if (!n) continue;
        if (!_ignoreInvisible || n->isVisible()) {
            items.push_back(n);
        }
    }
    if (_revMain) {
        std::reverse(items.begin(), items.end());
    }
    return items;
}

void LayoutMenu::sortChildren(std::vector<ax::Node*>& items) const {
    switch (_order) {
        case Order::Natural: break;
        case Order::NameAsc:
            std::sort(items.begin(), items.end(), [](ax::Node* a, ax::Node* b){ return a->getName() < b->getName(); });
            break;
        case Order::NameDesc:
            std::sort(items.begin(), items.end(), [](ax::Node* a, ax::Node* b){ return a->getName() > b->getName(); });
            break;
        case Order::TagAsc:
            std::sort(items.begin(), items.end(), [](ax::Node* a, ax::Node* b){ return a->getTag() < b->getTag(); });
            break;
        case Order::TagDesc:
            std::sort(items.begin(), items.end(), [](ax::Node* a, ax::Node* b){ return a->getTag() > b->getTag(); });
            break;
        case Order::ZAsc:
            std::sort(items.begin(), items.end(), [](ax::Node* a, ax::Node* b){ return a->getLocalZOrder() < b->getLocalZOrder(); });
            break;
        case Order::ZDesc:
            std::sort(items.begin(), items.end(), [](ax::Node* a, ax::Node* b){ return a->getLocalZOrder() > b->getLocalZOrder(); });
            break;
    }
}

static Size nodeSize(ax::Node* n) {
    auto s = n->getContentSize();
    return { s.width * n->getScaleX(), s.height * n->getScaleY() };
}

void LayoutMenu::updateLayout() {
    auto items = collectChildren();
    sortChildren(items);

    Size area = this->getContentSize();
    if (_autoGrowAxis && *_autoGrowAxis > 0.f) {
        if (_dir == Direction::Row) area.width = std::max(area.width, *_autoGrowAxis);
        else area.height = std::max(area.height, *_autoGrowAxis);
    }

    if (_dir == Direction::Row) layoutRow(items, area);
    else layoutColumn(items, area);
}

ax::Size LayoutMenu::getCalculatedLayoutSize() const {
    auto items = collectChildren();
    if (items.empty()) return ax::Size::ZERO;
    
    std::vector<ax::Size> sizes(items.size());
    for (size_t i = 0; i < items.size(); ++i) {
        sizes[i] = nodeSize(items[i]);
    }
    
    if (_dir == Direction::Row) {
        float totalWidth = 0.f;
        float maxHeight = 0.f;
        for (const auto& s : sizes) {
            totalWidth += s.width;
            maxHeight = std::max(maxHeight, s.height);
        }
        totalWidth += _gap * (items.size() - 1);
        return ax::Size(totalWidth, maxHeight);
    } else {
        float totalHeight = 0.f;
        float maxWidth = 0.f;
        for (const auto& s : sizes) {
            totalHeight += s.height;
            maxWidth = std::max(maxWidth, s.width);
        }
        totalHeight += _gap * (items.size() - 1);
        return ax::Size(maxWidth, totalHeight);
    }
}

void LayoutMenu::layoutRow(std::vector<ax::Node*>& items, const Size& area) {
    if (items.empty()) return;

    std::vector<Size> sizes(items.size());
    float contentWidth = 0.f;
    float maxHeight = 0.f;

    for (size_t i = 0; i < items.size(); ++i) {
        sizes[i] = nodeSize(items[i]);
        contentWidth += sizes[i].width;
        maxHeight = std::max(maxHeight, sizes[i].height);
    }

    float gaps = _gap * (items.size() - 1);
    contentWidth += gaps;

    if (_growCross) {
        setContentSize({
            std::max(area.width, contentWidth),
            std::max(area.height, maxHeight)
        });
    }

    float startX = 0.f;
    float freeSpace = area.width - contentWidth;

    switch (_alignMain) {
        case AlignMain::Start:  startX = 0.f; break;
        case AlignMain::Center: startX = freeSpace * 0.5f; break;
        case AlignMain::End:    startX = freeSpace; break;
        default: break;
    }

    float x = startX;

    for (size_t i = 0; i < items.size(); ++i) {
        auto* n = items[i];
        const auto& s = sizes[i];
        const auto ap = n->getAnchorPoint();

        float y = 0.f;
        float freeY = area.height - s.height;

        switch (_alignCross) {
            case AlignCross::Start:  y = area.height - s.height; break;
            case AlignCross::Center: y = freeY * 0.5f; break;
            case AlignCross::End:    y = 0.f; break;
        }

        n->setIgnoreAnchorPointForPosition(false);
        n->setPosition({
            x + ap.x * s.width,
            y + ap.y * s.height
        });

        x += s.width + _gap;
    }
}

void LayoutMenu::layoutColumn(std::vector<ax::Node*>& items, const Size& area) {
    if (items.empty()) return;

    std::vector<Size> sizes(items.size());
    float contentHeight = 0.f;
    float maxWidth = 0.f;

    for (size_t i = 0; i < items.size(); ++i) {
        sizes[i] = nodeSize(items[i]);
        contentHeight += sizes[i].height;
        maxWidth = std::max(maxWidth, sizes[i].width);
    }

    float gaps = _gap * (items.size() - 1);
    contentHeight += gaps;

    if (_growCross) {
        setContentSize({
            std::max(area.width, maxWidth),
            std::max(area.height, contentHeight)
        });
    }

    float startY = 0.f;
    float freeSpace = area.height - contentHeight;

    switch (_alignMain) {
        case AlignMain::Start:  startY = area.height - contentHeight; break;
        case AlignMain::Center: startY = freeSpace * 0.5f; break;
        case AlignMain::End:    startY = 0.f; break;
        default: break;
    }

    float y = startY + contentHeight;

    for (size_t i = 0; i < items.size(); ++i) {
        auto* n = items[i];
        const auto& s = sizes[i];

        float x = 0.f;
        float freeX = area.width - s.width;

        switch (_alignCross) {
            case AlignCross::Start:  x = 0.f; break;
            case AlignCross::Center: x = freeX * 0.5f; break;
            case AlignCross::End:    x = freeX; break;
        }

        y -= s.height;
        const auto ap = n->getAnchorPoint();  // Get user-set anchor point
        n->setIgnoreAnchorPointForPosition(false);
        n->setPosition({
            x + ap.x * s.width,
            y + ap.y * s.height
        });

        y -= _gap;
    }
}

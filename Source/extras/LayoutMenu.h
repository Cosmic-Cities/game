#pragma once

#include <axmol.h>
#include <vector>
#include <optional>

class LayoutMenu : public ax::Menu {
public:
    enum class Direction { Row, Column };
    enum class Order { Natural, NameAsc, NameDesc, TagAsc, TagDesc, ZAsc, ZDesc };
    enum class AlignMain { Start, Center, End, Between, Even };
    enum class AlignCross { Start, Center, End };

    static LayoutMenu* create();
    bool init() override;

    // Configuration
    LayoutMenu* setDirection(Direction d);
    LayoutMenu* setOrder(Order o);
    LayoutMenu* setGap(float g);
    LayoutMenu* setWrap(bool w);
    LayoutMenu* setAlignMain(AlignMain a);
    LayoutMenu* setAlignCross(AlignCross a);
    LayoutMenu* setReverseMain(bool r);
    LayoutMenu* setReverseCross(bool r);
    LayoutMenu* setAutoScale(bool enable);
    LayoutMenu* setDefaultScaleLimits(float minScale, float maxScale);
    LayoutMenu* setIgnoreInvisibleChildren(bool ignore);
    LayoutMenu* setGrowCrossAxis(bool grow);
    LayoutMenu* setAutoGrowAxis(std::optional<float> minLength);

    // Query
    Direction getDirection() const { return _dir; }
    Order getOrder() const { return _order; }
    float getGap() const { return _gap; }
    bool getWrap() const { return _wrap; }
    AlignMain getAlignMain() const { return _alignMain; }
    AlignCross getAlignCross() const { return _alignCross; }
    bool getReverseMain() const { return _revMain; }
    bool getReverseCross() const { return _revCross; }
    bool getAutoScale() const { return _autoScale; }
    bool getIgnoreInvisibleChildren() const { return _ignoreInvisible; }
    bool getGrowCrossAxis() const { return _growCross; }
    std::optional<float> getAutoGrowAxis() const { return _autoGrowAxis; }

    // Perform layout
    void updateLayout();

    ax::Size getCalculatedLayoutSize() const;

    ax::Vec2 getLayoutOrigin() const {
        return ax::Vec2::ZERO;
    }

protected:
    // Helpers
    std::vector<ax::Node*> collectChildren() const;
    void sortChildren(std::vector<ax::Node*>& items) const;
    void layoutRow(std::vector<ax::Node*>& items, const ax::Size& area);
    void layoutColumn(std::vector<ax::Node*>& items, const ax::Size& area);

private:
    Direction _dir{Direction::Row};
    Order _order{Order::TagAsc};
    AlignMain _alignMain{AlignMain::Start};
    AlignCross _alignCross{AlignCross::Start};
    float _gap{8.0f};
    bool _wrap{true};
    bool _revMain{false};
    bool _revCross{false};
    bool _autoScale{false};
    bool _ignoreInvisible{true};
    bool _growCross{true};
    float _minScale{0.65f};
    float _maxScale{1.0f};
    std::optional<float> _autoGrowAxis{};
};

#pragma once
#include "Core/Object.h"
#include "Core/Resource.h"
#include "Core/App.h"

namespace aby::ui {

    enum class EAnchor {
        NONE          = 0,
        TOP_LEFT      = 1,    // TOP_LEFT    TOP_MIDDLE    TOP_RIGHT
        TOP_MIDDLE    = 2,    // MIDDLE_LEFT MIDDLE        MIDDLE_RIGHT
        TOP_RIGHT     = 3,    // BOTTOM_LEFT BOTTOM_MIDDLE BOTTOM_RIGHT
        MIDDLE_LEFT   = 4,
        MIDDLE        = 5,
        MIDDLE_RIGHT  = 6,
        BOTTOM_LEFT   = 7,
        BOTTOM_MIDDLE = 8,
        BOTTOM_RIGHT  = 9,
    };

}

namespace aby::ui {

    struct Background {
        glm::vec3 color = glm::vec3(1.f);
        Resource  texture = {};
    };

    struct Border {
        glm::vec3 color;
        float     width;
    };

    struct Style {
        Background bg;
        Border     border;
    };

}

namespace aby::ui { 

    class Widget : public Object {
    public:
        explicit Widget(Ref<Widget> parent = nullptr);
        virtual ~Widget() = default;

        void on_create(App* app, bool deserialized) override;
        void on_event(App* app, Event& event) override;
        void on_tick(App* app, Time deltatime) override;
        void on_destroy(App* app) override;
        virtual void on_resize(const glm::u32vec2& new_size);
        
        void set_size(const glm::u32vec2& size);
        const glm::u32vec2& size();
        Weak<Widget> parent() const;
        Weak<Widget> parent();
    protected:
        friend class Canvas;
        friend class WidgetParent;
    protected:
        Weak<Widget> m_Parent;
        glm::u32vec2 m_Size;
    };

    class WidgetParent : public Widget, public std::enable_shared_from_this<WidgetParent> {
    public:
        explicit WidgetParent(std::size_t reserve = 0);
        virtual ~WidgetParent() = default;

        void on_create(App* app, bool deserialized) override;
        void on_event(App* app, Event& event) override;
        void on_tick(App* app, Time deltatime) override;
        void on_destroy(App* app) override;
        void on_resize(const glm::u32vec2& new_size) override;

        void add_child(Ref<Widget> widget);
        void erase_child(std::size_t index);

        std::span<Ref<Widget>> children();
        std::span<const Ref<Widget>> children() const;

        auto begin();
        auto begin() const;
        auto end();
        auto end() const;
    private:
        std::vector<Ref<Widget>> m_Children;
    };

}
#pragma once
#include "Rendering/UI/UICommon.h"
#include "Core/Object.h"
#include "Core/App.h"

namespace aby::ui { 
    
    class Widget : public virtual Object {
    public:
        explicit Widget(const Transform& transform, const Style& style, Ref<Widget> parent = nullptr);
        virtual ~Widget() = default;

        void on_create(App* app, bool deserialized) override {}
        void on_event(App* app, Event& event) override;
        void on_tick(App* app, Time deltatime) override { /*if (!bVisible) return;*/ }
        void on_destroy(App* app) override {}
        /**
        * @brief Property setters should invalidate the object.
        * @return True:  The object is invalid still.
        * @return False: The object is valid again.
        */
        virtual bool on_invalidate() { return false; }
        virtual bool on_window_resize(WindowResizeEvent& event);

        void set_transform(const Transform& transform);
        void set_size(const glm::vec2& size);
        void set_position(const glm::vec2& position);
        void set_style(const Style& style);
        void set_parent(Ref<Object> parent);
        void set_anchor(Anchor anchor);
        void set_invalid(bool invalid);
        void set_zindex(std::int32_t zindex);
        void set_visible(bool visible);

        bool is_visible() const;
        bool is_invalid() const;

        const Transform& transform() const;
        const Style& style() const;
        std::int32_t zindex() const;
        Weak<Object> parent() const;
        Weak<Object> parent();
    protected:
        void invalidate_self();
        friend class Canvas;
    protected:
        Transform m_Transform;
        Style m_Style;
        bool bInvalid;
        bool bVisible;
        std::int32_t m_ZIndex;
        Weak<Object> m_Parent;
    };

}   
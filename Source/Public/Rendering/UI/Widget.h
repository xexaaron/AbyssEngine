#pragma once
#include "Core/Object.h"
#include "Core/Resource.h"
#include "Core/App.h"
#include <queue>
#include <array>

namespace aby::ui {
    
    enum class EAnchor {
        NONE          = 0,
        TOP_LEFT      = NONE, 
        TOP_MIDDLE    = 1,    
        TOP_RIGHT     = 2,    
        MIDDLE_LEFT   = 3,
        MIDDLE        = 4,
        MIDDLE_RIGHT  = 5,
        BOTTOM_LEFT   = 6,
        BOTTOM_MIDDLE = 7,
        BOTTOM_RIGHT  = 8,
    };
    
    enum class EScaleMode {
        SCALE_TO_FIT = 0,
        FIT_TO_SCALE = 1,
    };

    enum class EDirection {
        HORIZONTAL = 0, // x
        VERTICAL   = 1, // y
    };

    enum class EButtonState {
        DEFAULT = 0,
        HOVERED = 1,
        PRESSED = 2,
        RELEASED = DEFAULT,
    };
}

namespace aby::ui {
    struct Anchor {
        EAnchor   position = EAnchor::NONE;
        glm::vec2 offset   = { 0.f, 0.f };
    };
    struct Transform {
        Anchor    anchor   = {};
        glm::vec2 position = { 0.f, 0.f };
        glm::vec2 size     = { 0.f, 0.f };
    };
    
    struct Background {
        glm::vec4 color = { 1.f, 1.f, 1.f, 1.f };
        Resource  texture = {};
    };
    struct Border {
        glm::vec4 color = { 0.f, 0.f, 0.f, 1.f };
        float     width = 0.f;
    };
    struct Style {
        Background background = {};
        Border     border     = {};
    };

    struct ButtonStyle {
        Background hovered  = {};
        Background pressed  = {};
        Background released = {}; // released and default are the same.
        Border     border   = {};
    };

}

namespace aby::ui { 
    
    class Widget : public virtual Object {
    public:
        explicit Widget(const Transform& transform, const Style& style, Ref<Widget> parent = nullptr);
        virtual ~Widget() = default;

        void on_create(App* app, bool deserialized) override {}
        void on_event(App* app, Event& event) override;
        void on_tick(App* app, Time deltatime) override {}
        void on_destroy(App* app) override {}
        /**
        * @brief Property setters should invalidate the object.
        * @return True:  The object is invalid still.
        * @return False: The object is valid again.
        */
        virtual bool on_invalidate() { return false; }

        void set_transform(const Transform& transform);
        void set_size(const glm::vec2& size);
        void set_position(const glm::vec2& position);
        void set_style(const Style& style);
        void set_parent(Ref<Object> parent);
        void set_anchor(Anchor anchor);
        void set_invalid(bool invalid);
        void set_zindex(std::int32_t zindex);

        bool is_invalid() const;

        const Transform& transform() const;
        const Style& style() const;
        std::int32_t zindex() const;
        Weak<Object> parent() const;
        Weak<Object> parent();
    protected:
        virtual bool on_window_resize(WindowResizeEvent& event);
        friend class Canvas;
    protected:
        Transform m_Transform;
        Style m_Style;
        bool bInvalid;
        std::int32_t m_ZIndex;
        Weak<Object> m_Parent;
    };

    class ParentWidget : public virtual Object, public std::enable_shared_from_this<ParentWidget> {
    public:
        explicit ParentWidget(std::size_t reserve = 0);
        virtual ~ParentWidget() = default;

        void on_create(App* app, bool deserialized) override;
        void on_tick(App* app, Time deltatime) override;
        void on_event(App* app, Event& event) override;
        void on_destroy(App* app) override;
        void on_invalidate();

        virtual std::size_t add_child(Ref<Widget> child);
        virtual void remove_child(Ref<Widget> child);
        virtual void remove_child(std::size_t idx);
        
        std::span<Ref<Widget>> children();
        std::span<const Ref<Widget>> children() const;
    private:
        std::vector<Ref<Widget>> m_Children;
        std::queue<std::size_t> m_Invalidated;
    };

    class Image : public Widget {
    public:
        static Ref<Image> create(const Transform& transform, const Style& style, Ref<Widget> parent = nullptr);
        Image(const Transform& transform, const Style& style, Ref<Widget> parent = nullptr);

        void on_tick(App* app, Time deltatime) override;
        bool on_invalidate() override;
    protected:
        Quad m_Image;
        std::array<Quad, 4> m_Border;
        std::size_t m_BorderCt;
    };

    class Panel : public Image, public ParentWidget {
    public:
        static Ref<Panel> create(const Transform& transform, const Style& style);
        Panel(const Transform& transform, const Style& style);
        virtual ~Panel() = default;

        void on_create(App* app, bool deserialized) override;
        void on_tick(App* app, Time deltatime) override;
        void on_event(App* app, Event& event) override;
        void on_destroy(App* app) override;
        bool on_invalidate() override;
    private:
    };

    class Canvas : public Panel {
    public:
        static Ref<Canvas> create(const Style& style);
        Canvas(const Style& style);
        virtual ~Canvas() = default;
        
        void on_tick(App* app, Time deltatime) override;
        void on_create(App* app, bool deserialized) override;
    protected:
        bool on_window_resize(WindowResizeEvent& event) override;
    };

    class Textbox : public Image {
    public:
        static Ref<Textbox> create(const Transform& transform, const Style& style, const std::string& text, float scale = 1.f);

        Textbox(const Transform& transform, const Style& style, const std::string& text, float scale = 1.f);

        void on_create(App* app, bool deserialized) override;
        void on_tick(App* app, Time deltatime) override;
        bool on_invalidate() override;

        void set_text(const std::string& text, Ref<Font> font);
    protected:
        Text m_Text;
        glm::vec2 m_TextSize;
        Ref<Font> m_Font;
    };

    class Button : public Textbox {
    public:
        static Ref<Button> create(const Transform& transform, const ButtonStyle& style, const std::string& text);
        
        Button(const Transform& transform, const ButtonStyle& style, const std::string& text);

        void on_tick(App* app, Time deltatime) override;
        bool on_invalidate() override;
        void on_event(App* app, Event& event) override;

        virtual void on_pressed();
        virtual void on_released();
    protected:

    private:
        bool on_mouse_moved(MouseMovedEvent& event);
        bool on_mouse_pressed(MousePressedEvent& event);
        bool on_mouse_released(MouseReleasedEvent& event);
    private:
        Background   m_Default;
        Background   m_Hovered;
        Background   m_Pressed;
        EButtonState m_State;
    };

    class LayoutContainer : public Panel {
    public:
        static Ref<LayoutContainer> create(const Transform& transform, const Style& style, EDirection direction);
        LayoutContainer(const Transform& transform, const Style& style, EDirection direction);
        virtual ~LayoutContainer() = default;

        bool on_invalidate() override;

        std::size_t add_child(Ref<Widget> widget) override;
        // void remove_child(Ref<Widget> widget) override;
        // void remove_child(Ref<Widget> widget) override;
    protected:
        void align(Ref<Widget> widget);
        void scale();
    private:
        EDirection m_Direction;
    };

}   
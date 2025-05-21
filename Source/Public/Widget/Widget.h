#pragma once
#include "Widget/UICommon.h"
#include "Core/Object.h"
#include "Core/App.h"

namespace aby::ui { 
    
    class Widget : public Object, public std::enable_shared_from_this<Widget> {
    protected:
        friend class Canvas;
        using Children = std::vector<Ref<Widget>>;
        using Parent = Weak<Widget>;
        using Super = Widget;
    public:
        explicit Widget(const Transform& transform, const std::string& name = "", Ref<Widget> parent = nullptr);
        virtual ~Widget() = default;

        void on_create(App* app, bool deserialized) override;
        void on_event(App* app, Event& event) override;
        void on_tick(App* app, Time deltatime) override;
        void on_destroy(App* app) override;
        virtual bool on_window_resize(WindowResizeEvent& event);

        virtual std::size_t add_child(Ref<Widget> widget);
        virtual void remove_child(std::size_t i);
        virtual void print(std::ostream& os);
        void recurse(std::function<void(Ref<Widget>)>);


        void set_transform(const Transform& transform);
        void set_size(const glm::vec2& size);
        void set_position(const glm::vec2& position);
        void set_parent(Ref<Widget> parent);
        void set_anchor(Anchor anchor);
        void set_zindex(i32 zindex);
        void set_visible(bool visible);
        void set_name(const std::string& name);

        Children&        children();
        const Children&  children() const;
        bool             is_visible() const;
        const Transform& transform() const;
        i32              zindex() const;
        Weak<Object>     parent() const;
        const std::string& name() const;

        virtual void for_each(std::function<void(Ref<Widget>)> fn);
        void for_each(std::function<void(Ref<Widget>, std::size_t)> fn);
    protected:
        bool        bVisible;
        bool        bScalesWithWindow;
        i32         m_ZIndex;
        Transform   m_Transform;
        std::string m_Name;
        Parent      m_Parent;
        Children    m_Children;
    };

}   
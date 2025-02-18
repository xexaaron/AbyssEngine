#pragma once
#include "Rendering/UI/ParentWidget.h"
#include "Rendering/UI/Image.h"

namespace aby::ui {
    
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

}
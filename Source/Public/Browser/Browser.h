#include "Core/Common.h"
#include "Core/Object.h"

namespace aby::web {

    class Browser : public Object {
    public:
        Browser();
        ~Browser(); 

        void on_create(App* app, bool) override;
        void on_destroy(App* app) override;
        void on_tick(App* app, Time dt) override;
        void on_event(App* app, Event& event) override;
        
    private:
        App* m_App;
    };

}
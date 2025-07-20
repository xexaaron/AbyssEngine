#include "Core/Common.h"
#include "Core/Object.h"
#include "Core/App.h"

#include <cef_app.h>
#include <cef_browser_process_handler.h>
#include <cef_client.h>
#include <cef_base.h>

namespace aby::web {

    class WebHandler : public CefClient, 
                       public CefDisplayHandler, 
                       public CefLifeSpanHandler, 
                       public CefLoadHandler, 
                       public CefRenderHandler {
    public:
        explicit WebHandler(App* app, bool is_alloy_style);
        ~WebHandler() override;

        void ShowMainWindow();
        void CloseAllBrowsers(bool force_close);

        void OnTitleChange(CefRefPtr<CefBrowser> browser, const CefString& title) override;
        void OnAfterCreated(CefRefPtr<CefBrowser> browser) override;
        bool DoClose(CefRefPtr<CefBrowser> browser) override;
        void OnBeforeClose(CefRefPtr<CefBrowser> browser) override;
        void OnLoadError(CefRefPtr<CefBrowser> browser, CefRefPtr<CefFrame> frame, ErrorCode errorCode, const CefString& errorText, const CefString& failedUrl) override;
        void OnPaint(CefRefPtr<CefBrowser> browser, PaintElementType type, const RectList& dirtyRects, const void* buffer, int width, int height) override;

        CefRefPtr<CefDisplayHandler> GetDisplayHandler() override;
        CefRefPtr<CefLifeSpanHandler> GetLifeSpanHandler() override;
        CefRefPtr<CefLoadHandler> GetLoadHandler() override;
        CefRefPtr<CefRenderHandler> GetRenderHandler() override;
        Ref<Texture> GetDrawBuffer();
        void GetViewRect(CefRefPtr<CefBrowser> browser, CefRect& rect) override;
        const CefRect& GetViewport() const; 
        CefRefPtr<CefBrowser> GetBrowser();

        bool IsClosing() const;
        bool HasAtleastOneBrowser() const;

        void SetViewport(int x, int y, int width, int height);


        void SendMouseMoveEvent(const CefMouseEvent& mouse_event, bool mouse_leave);
        void SendMouseClickEvent(const CefMouseEvent& mouse_event, CefBrowserHost::MouseButtonType type, bool mouse_up, int click_count);
        void SendKeyEvent(const CefKeyEvent& mouse_event);

    private:
        void PlatformTitleChange(CefRefPtr<CefBrowser> browser, const CefString& title);
        void PlatformShowWindow(CefRefPtr<CefBrowser> browser);

        std::list<CefRefPtr<CefBrowser>> m_BrowserList;
        App* m_App;
        bool bIsClosing;
        const bool bIsAlloyStyle;
        CefRect m_Viewport;
        Ref<Texture> m_Buffer;
        Resource m_Texture;
    private:
        IMPLEMENT_REFCOUNTING(WebHandler);
    };

    class WebApp : public CefApp, public CefBrowserProcessHandler {
    public:
        WebApp(App* app);

        void OnContextInitialized() override;
        
        CefRefPtr<CefBrowserProcessHandler> GetBrowserProcessHandler() override;
        CefRefPtr<CefClient> GetDefaultClient() override;
        CefRefPtr<WebHandler> GetHandler();
        
    private:
        App* m_App;
        CefRefPtr<WebHandler> m_Handler;
    private:
        IMPLEMENT_REFCOUNTING(WebApp);
    };

    class Browser : public Object {
    public:
        Browser(Window* window);
        ~Browser(); 

        void on_create(App* app, bool) override;
        void on_destroy(App* app) override;
        void on_tick(App* app, Time dt) override;
        void on_event(App* app, Event& event) override;
        
        bool on_window_resize(WindowResizeEvent& event);
        bool on_mouse_moved(MouseMovedEvent& event);
        bool on_mouse_pressed(MousePressedEvent& event);
        bool on_mouse_released(MouseReleasedEvent& event);
        bool on_key_pressed(KeyPressedEvent& event);
        bool on_key_released(KeyReleasedEvent& event);
        bool on_key_typed(KeyTypedEvent& event);


        void set_viewport(int x, int y, int w, int h);
        void set_viewport(ImVec2 pos, ImVec2 size);

    private:
        App* m_App;
        CefRefPtr<WebApp> m_WebApp;
        ImVec2 m_ViewportPos;
    };

}
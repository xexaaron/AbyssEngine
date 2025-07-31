#include "Browser/Browser.h"
#include "Core/Log.h"
#include "Platform/Platform.h"

#include <imgui/imgui.h>
#include <cef_command_line.h>
#include <cef_version_info.h>
#include <cef_browser.h>
#include <views/cef_browser_view.h>
#include <views/cef_window.h>
#include <wrapper/cef_helpers.h>
#include <cef_parser.h>
#include <cef_task.h>
#include <base/cef_callback.h>
#include <wrapper/cef_closure_task.h>

namespace aby::web {
    
    std::string GetDataURI(const std::string& data, const std::string& mime_type) {
        return "data:" + mime_type + ";base64," + CefURIEncode(CefBase64Encode(data.data(), data.size()), false).ToString();
    }

    WebHandler::WebHandler(App* app, bool is_alloy_style) : 
        m_App(app),
        bIsAlloyStyle(is_alloy_style), 
        bIsClosing(false),
        m_Buffer(nullptr),
        m_Viewport(0, 0, 800, 600)
    {

    }

    WebHandler::~WebHandler() {}

    void WebHandler::PlatformTitleChange(CefRefPtr<CefBrowser> browser, const CefString& title) {
        m_App->window()->set_title(title);
    }

    void WebHandler::PlatformShowWindow(CefRefPtr<CefBrowser> browser) {
        m_App->window()->set_minimized(false); // Restore Window
    }

    void WebHandler::OnTitleChange(CefRefPtr<CefBrowser> browser, const CefString& title) {
        CEF_REQUIRE_UI_THREAD();
        if (auto browser_view = CefBrowserView::GetForBrowser(browser)) {
            CefRefPtr<CefWindow> window = browser_view->GetWindow();
            if (window) {
                window->SetTitle(title);
            }
        }
        else if (bIsAlloyStyle) {
            PlatformTitleChange(browser, title);
        }
    }
    
    void WebHandler::OnAfterCreated(CefRefPtr<CefBrowser> browser) {
        CEF_REQUIRE_UI_THREAD();

        // Sanity-check the configured runtime style.
        CHECK_EQ(bIsAlloyStyle ? CEF_RUNTIME_STYLE_ALLOY : CEF_RUNTIME_STYLE_CHROME, browser->GetHost()->GetRuntimeStyle());

        m_BrowserList.push_back(browser);
        browser->GetHost()->SetFocus(true);

        m_Buffer = BufferedTexture::create(
            &m_App->ctx(),
            { 1, 1 },
            { std::byte(0xFF), std::byte(0xFF), std::byte(0xFF), std::byte(0xFF) },
            4, 
            ETextureFormat::BGRA,
            2
        );
        m_Buffer->set_dbg_name("Cef Browser Texture");
    }
    
    bool WebHandler::DoClose(CefRefPtr<CefBrowser> browser) {
        CEF_REQUIRE_UI_THREAD();

        // Closing the main window requires special handling. See the DoClose()
        // documentation in the CEF header for a detailed destription of this
        // process.
        if (m_BrowserList.size() == 1) {
            bIsClosing = true;
        }

        // Allow the close. For windowed browsers this will result in the OS close
        // event being sent.
        return false;
    }
    
    void WebHandler::OnBeforeClose(CefRefPtr<CefBrowser> browser) {
        CEF_REQUIRE_UI_THREAD();

        // Remove from the list of existing browsers.
        auto bit = m_BrowserList.begin();
        for (; bit != m_BrowserList.end(); ++bit) {
            if ((*bit)->IsSame(browser)) {
                m_BrowserList.erase(bit);
                break;
            }
        }
    }
    
    void WebHandler::OnLoadError(CefRefPtr<CefBrowser> browser, CefRefPtr<CefFrame> frame, ErrorCode errorCode, const CefString& errorText, const CefString& failedUrl) {
        CEF_REQUIRE_UI_THREAD();

        // Allow Chrome to show the error page.
        if (!bIsAlloyStyle) {
            return;
        }

        // Don't display an error for downloaded files.
        if (errorCode == ERR_ABORTED) {
            return;
        }

        // Display a load error message using a data: URI.
        std::stringstream ss;
        ss << "<html><body bgcolor=\"white\">"
            "<h2>Failed to load URL "
            << std::string(failedUrl) << " with error " << std::string(errorText)
            << " (" << errorCode << ").</h2></body></html>";

        frame->LoadURL(GetDataURI(ss.str(), "text/html"));
    }

    void WebHandler::OnPaint(CefRefPtr<CefBrowser> browser, PaintElementType type, const RectList& dirtyRects, const void* buffer, int width, int height) {
        UNREFERENCED_PARAMETER(dirtyRects); 
        m_Buffer->write({ width, height }, buffer);
    }
    
    CefRefPtr<CefDisplayHandler> WebHandler::GetDisplayHandler() {
        return this;
    }
    
    CefRefPtr<CefLifeSpanHandler> WebHandler::GetLifeSpanHandler()
    {
        return this;
    }
    
    CefRefPtr<CefLoadHandler> WebHandler::GetLoadHandler()
    {
        return this;
    }

    CefRefPtr<CefRenderHandler> WebHandler::GetRenderHandler() {
        return this;
    }
    
    Ref<BufferedTexture> WebHandler::GetDrawBuffer() {
        return m_Buffer;
    }

    void WebHandler::GetViewRect(CefRefPtr<CefBrowser> browser, CefRect& rect) {
        rect = m_Viewport;
    }

    const CefRect& WebHandler::GetViewport() const {
        return m_Viewport;
    }

    CefRefPtr<CefBrowser> WebHandler::GetBrowser() {
        return m_BrowserList.front();
    }


    bool WebHandler::IsClosing() const {
        return bIsClosing;
    }
    bool WebHandler::HasAtleastOneBrowser() const {
        return !m_BrowserList.empty();
    }

    void WebHandler::ShowMainWindow() {
        if (!CefCurrentlyOn(TID_UI)) {
            // Execute on the UI thread.
            CefPostTask(TID_UI, base::BindOnce(&WebHandler::ShowMainWindow, this));
            return;
        }

        if (m_BrowserList.empty()) {
            return;
        }

        auto& main_browser = m_BrowserList.front();

        if (auto browser_view = CefBrowserView::GetForBrowser(main_browser)) {
            // Show the window using the Views framework.
            if (auto window = browser_view->GetWindow()) {
                window->Show();
            }
        }
        else if (bIsAlloyStyle) {
            PlatformShowWindow(main_browser);
        }
    }
    
    void WebHandler::CloseAllBrowsers(bool force_close) {
        if (!CefCurrentlyOn(TID_UI)) {
            // Execute on the UI thread.
            CefPostTask(TID_UI, base::BindOnce(&WebHandler::CloseAllBrowsers, this, force_close));
            return;
        }
        std::vector<CefRefPtr<CefBrowser>> browsers_to_close(m_BrowserList.begin(), m_BrowserList.end());
        for (auto& browser : browsers_to_close) {
            browser->GetHost()->CloseBrowser(force_close);
        }
    }

    void WebHandler::SetViewport(int x, int y, int width, int height) {
        m_Viewport.Set(x, y, width, height);
    }

    void WebHandler::SendMouseMoveEvent(const CefMouseEvent& mouse_event, bool mouse_leave) {
        m_BrowserList.front()->GetHost()->SendMouseMoveEvent(mouse_event, mouse_leave);
    }

    void WebHandler::SendMouseClickEvent(const CefMouseEvent& mouse_event, CefBrowserHost::MouseButtonType type, bool mouse_up, int click_count) {
        m_BrowserList.front()->GetHost()->SendMouseClickEvent(mouse_event, type, mouse_up, click_count);
    }

    
    void WebHandler::SendKeyEvent(const CefKeyEvent& mouse_event) {
        m_BrowserList.front()->GetHost()->SendKeyEvent(mouse_event);
    }
}

namespace aby::web {

    WebApp::WebApp(App* app) : m_App(app), m_Handler(nullptr) {}

    void WebApp::OnContextInitialized() {
        CEF_REQUIRE_UI_THREAD();

        CefRefPtr<CefCommandLine> command_line = CefCommandLine::GetGlobalCommandLine();

        cef_runtime_style_t runtime_style = CEF_RUNTIME_STYLE_ALLOY;
         m_Handler = CefRefPtr<WebHandler>(new WebHandler(m_App, true));

        // Setup OSR (offscren rendering mode)
        CefBrowserSettings browser_settings;
        CefWindowInfo window_info;
        window_info.SetAsWindowless(nullptr);
        
#if defined(OS_WIN)
        // On Windows we need to specify certain flags that will be passed to
        // CreateWindowEx().
        window_info.SetAsPopup(nullptr, "cefsimple");
#endif

        // Alloy style will create a basic native window. Chrome style will create a
        // fully styled Chrome UI window.
        window_info.runtime_style = runtime_style;

        // Create the first browser window.

        std::string url = "https://www.google.com";
        CefBrowserHost::CreateBrowser(window_info, m_Handler, url, browser_settings, nullptr, nullptr);
    }

    CefRefPtr<CefBrowserProcessHandler> WebApp::GetBrowserProcessHandler(){
        return this;
    }

    CefRefPtr<CefClient> WebApp::GetDefaultClient() {
        return m_Handler;
    }

    CefRefPtr<WebHandler> WebApp::GetHandler() {
        return m_Handler;
    }


}

#define CEF_SETTING_STR(x, v) CefString(&x) = v

namespace aby::web {

    Browser::Browser(Window* window) : 
        m_App(nullptr),
        m_WebApp(nullptr)
    {
        
    }

    Browser::~Browser() {

    }

    void Browser::on_create(App* app, bool) {
        m_App = app;
        m_WebApp = CefRefPtr<WebApp>(new WebApp(m_App));
        
        void* sandbox_info = nullptr;
#if defined (CEF_USE_SANDBOX)
        CefScopedSandboxInfo scoped_sandbox;
        sandbox_info = scoped_sandbox.sandbox_info();
#endif

        CefMainArgs cef_args(GetModuleHandleA(nullptr));
        CefSettings settings;
        settings.no_sandbox                   = !sandbox_info;
        settings.windowless_rendering_enabled = true;  // Setup OSR (offscreen rendering).
        settings.multi_threaded_message_loop  = false; // Use CefDoMessageLoopWork instead of CefRunMessageLoop 
        CEF_SETTING_STR(settings.cache_path, (app->cache() / "cef").wstring());
        if (!CefInitialize(cef_args, settings, m_WebApp.get(), sandbox_info)) {
            ABY_ERR("CEF failed to initialize, {}", CefGetExitCode());
            return;
        }

    }
        
    void Browser::on_destroy(App* app) {
        auto handler = m_WebApp->GetHandler();
        handler->CloseAllBrowsers(true);
        CefShutdown();
        m_WebApp = nullptr;
    }   
        
    void Browser::on_tick(App* app, Time dt) {
        CefDoMessageLoopWork(); // Always run this, regardless of window open

        if (!ImGui::Begin("Browser")) {
            ImGui::End();
            return;
        }

        auto texture = m_WebApp->GetHandler()->GetDrawBuffer();
        if (texture && texture->size().x > 0 && texture->size().y > 0) {
            ImVec2 avail = ImGui::GetContentRegionAvail();
            float texWidth = static_cast<float>(texture->size().x);
            float texHeight = static_cast<float>(texture->size().y);

            float aspect = texWidth / texHeight;
            float availAspect = avail.x / avail.y;

            ImVec2 drawSize;
            if (availAspect > aspect) {
                // Fit height
                drawSize.y = avail.y;
                drawSize.x = drawSize.y * aspect;
            }
            else {
                // Fit width
                drawSize.x = avail.x;
                drawSize.y = drawSize.x / aspect;
            }

            ImVec2 cursorPos = ImGui::GetCursorScreenPos();
            set_viewport(cursorPos, drawSize);

            ImGui::Image(texture->imgui_id(), drawSize);
        }

        ImGui::End();
    }
    
    void Browser::on_event(App* app, Event& event) {
        EventDispatcher dsp(event);
        dsp.bind(this, &Browser::on_window_resize);
        if (m_WebApp->GetHandler()->HasAtleastOneBrowser()) {
            dsp.bind(this, &Browser::on_mouse_moved);
            dsp.bind(this, &Browser::on_mouse_pressed);
            dsp.bind(this, &Browser::on_mouse_released);
            dsp.bind(this, &Browser::on_key_pressed);
            dsp.bind(this, &Browser::on_key_released);
            dsp.bind(this, &Browser::on_key_typed);
        }
    }

    bool Browser::on_mouse_moved(MouseMovedEvent& event) {
        auto globalMouse = ImVec2(event.x(), event.y());
        auto localMouse = ImVec2(globalMouse.x - m_ViewportPos.x, globalMouse.y - m_ViewportPos.y);

        CefMouseEvent cef_event;
        cef_event.x = static_cast<int>(localMouse.x);
        cef_event.y = static_cast<int>(localMouse.y);
        m_WebApp->GetHandler()->SendMouseMoveEvent(cef_event, false);
        return false;
    }

    bool Browser::on_mouse_pressed(MousePressedEvent& event) {
        CefMouseEvent cef_event;
        cef_event.x = static_cast<int>(event.pos().x);
        cef_event.y = static_cast<int>(event.pos().y);
        m_WebApp->GetHandler()->SendMouseClickEvent(
            cef_event, 
            static_cast<CefBrowserHost::MouseButtonType>(event.code()),
            false,
            1
        );
        return false;
    }
    
    bool Browser::on_mouse_released(MouseReleasedEvent& event) {
        CefMouseEvent cef_event;
        cef_event.x = static_cast<int>(event.pos().x);
        cef_event.y = static_cast<int>(event.pos().y);
        m_WebApp->GetHandler()->SendMouseClickEvent(
            cef_event,
            static_cast<CefBrowserHost::MouseButtonType>(event.code()),
            true,
            1
        );
        return false;
    }

    bool Browser::on_key_pressed(KeyPressedEvent& event) {
        CefKeyEvent ke;
        ke.windows_key_code = sys::glfw_to_platform_keycode(event.code());
        ke.native_key_code = 0;
        ke.type = KEYEVENT_KEYDOWN;
        m_WebApp->GetHandler()->SendKeyEvent(ke);
        return false;
    }

    bool Browser::on_key_released(KeyReleasedEvent& event) {
        CefKeyEvent ke;
        ke.windows_key_code = sys::glfw_to_platform_keycode(event.code());
        ke.native_key_code = 0;
        ke.type = KEYEVENT_KEYUP;
        m_WebApp->GetHandler()->SendKeyEvent(ke);
        return false;
    }

    bool Browser::on_key_typed(KeyTypedEvent& event) {
        CefKeyEvent ke;
        ke.type = KEYEVENT_CHAR;
        ke.character = static_cast<char16_t>(event.code());             
        ke.unmodified_character = ke.character;                         
        ke.windows_key_code = static_cast<int>(ke.character);           
        m_WebApp->GetHandler()->SendKeyEvent(ke);
        return false;
    }
    bool Browser::on_window_resize(WindowResizeEvent& event)  {
        return false;
    }

    void Browser::set_viewport(int x, int y, int w, int h) {
        if (m_WebApp->GetHandler()->GetViewport() != CefRect(x, y, w, h)) {
            m_WebApp->GetHandler()->SetViewport(x, y, w, h);
            m_ViewportPos = { static_cast<float>(x), static_cast<float>(y) };
            if (auto browser = m_WebApp->GetHandler()->GetBrowser()) {
                browser->GetHost()->WasResized();
            }
        }
    }
    
    void Browser::set_viewport(ImVec2 pos, ImVec2 size) {
        set_viewport(static_cast<int>(pos.x),
                     static_cast<int>(pos.y),
                     static_cast<int>(size.x),
                     static_cast<int>(size.y));
    }



}
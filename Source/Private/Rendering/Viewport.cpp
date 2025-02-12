#include "Rendering/Viewport.h"
#include "Core/App.h"

namespace aby {

	static Resource texture;

	Viewport::Viewport(const glm::u32vec2& size) : 
		m_Size(size),
		m_Camera()
	{

	}

	void Viewport::on_create(App* app, bool deserialized) {
		m_Camera.on_create(app, deserialized);
		auto& ctx = app->ctx();
		texture = aby::Texture::create(&ctx, app->bin() / "Textures/DefaultTexture.png");
	}

	void Viewport::on_event(App* app, Event& event) {
		EventDispatcher dsp(event);
		dsp.bind(this, &Viewport::on_resize);
		m_Camera.on_event(app, event);
	}
	
	void Viewport::on_tick(App* app, Time deltatime) {
		float scale_factor = 200.f;  // Example scale factor, you can adjust this value
		glm::vec2 size = { 400, 400    };
		glm::vec3 pos  = { 200, 200, 0 };
		glm::vec3 col  = {   1,   1, 1 };
		Quad quad(size, pos, col, texture.handle());

		m_Camera.on_tick(app, deltatime);
		auto& ren = app->renderer();
		ren.on_begin(m_Camera.view_projection());
		ren.draw_quad_2d(quad);
		ren.on_end();
	}
	
	void Viewport::on_destroy(App* app) {
		m_Camera.on_destroy(app);
	}

	bool Viewport::on_resize(WindowResizeEvent& event) {
		m_Size = { event.w(), event.h() };
		m_Camera.set_viewport(m_Size);
		return false;
	}


}
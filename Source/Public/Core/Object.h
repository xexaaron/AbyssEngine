#pragma once

#include "Core/Common.h"
#include "Core/Event.h"
#include "Core/Time.h"
#include "Core/Log.h"
#include "Utility/Random.h"
#include <span>

namespace aby {

	class App;	

	class Object {
	public:
		Object() = default;
		Object(const Object&) = default;
		Object(Object&&) noexcept = default;
		virtual ~Object() = default;

		/**
		* Called before the application 'ticks' and polls for events every frame.
		* @param app Pointer to the application.
		* @param deserialized Indicates if the instance was deserialized before creation. 
		* @remarks (Control per instance with flag if necessary for object class).
		*/
		virtual void on_create(App* app, bool deserialized) {}
		/**
		* Called when a window, input, or application event is called.
		* @param app Pointer to the application.
		* @param event Event to be handed off to an EventDispatcher.
		*/
		virtual void on_event(App* app, Event& event) {}
		/**
		* Called every application tick.
		* @param app Pointer to the application.
		* @param deltatime The deltatime in milliseconds.
		*/
		virtual void on_tick(App* app, Time deltatime) {}
		/**
		* Called when the object is removed from a container/manager.
		* @param app Pointer to the application
		*/
		virtual void on_destroy(App* app) {}
	
		util::UUID uuid() const;

		template <typename T> requires (std::is_base_of_v<Object, T>)
		T* as() {
			auto p = dynamic_cast<T*>(this); 
			ABY_ASSERT(p, "Object is not compatible with type '{}'", typeid(T).name());
			return p;
		}

		Object& operator=(const Object&) = default;
		
		bool operator==(const Object& other) const;
		bool operator!=(const Object & other) const;
	private:
		util::UUID m_ID;
	private:
		friend class App;
	};
}

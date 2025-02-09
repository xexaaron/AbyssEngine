#pragma once

#include "Core/Common.h"
#include "Core/Serialize.h"
#include "Core/Event.h"

namespace aby {

	class App;	

	class Object {
	public:
		Object() : m_ID() {}
		Object(const Object& other) : m_ID(other.m_ID) {}
		Object(Object&& other) noexcept : m_ID(other.m_ID) {}

		/**
		* @brief Called when the object is created by some "context" 
		*		 ie. a manager/container (generally when a game starts).
		* @param app Pointer to the application.
		* @param deserialized Indicates if the instance was deserialized before creation. 
		* @remarks (Control per instance with flag if necessary for object class).
		*/
		virtual void on_create(App* app, bool deserialized) {}
		/**
		* @brief Called when a window, input, or application event is called.
		* @param app Pointer to the application.
		* @param event Event to be handed off to an EventDispatcher.
		*/
		virtual void on_event(App* app, Event& event) {};
		/**
		* @brief Called every application tick.
		* @param app Pointer to the application.
		* @param deltatime The deltatime in millseconds.
		*/
		virtual void on_tick(App* app, Time deltatime) {};
		/**
		* @brief Called when the object is removed from a container/manager.
		* @param app Pointer to the application
		*/
		virtual void on_destroy(App* app) {}
		/**
		* @brief Called before the object is destroyed.
		* @param serializer Serializer containing one derived Object instance of data.
		*/
		virtual void on_serialize(Serializer& serializer) {}
		/**
		* @brief Called before the object is created.
		* @param serializier Serializer to write one derived Object instance to.
		* @return true:  If deserialization occured.
		* @return false: If deserialization did not occur.
		*/
		virtual bool on_deserialize(Serializer& serializer) { return false;  }

		inline UUID uuid() const { 
			return m_ID;
		}


		Object& operator=(const Object& other) {
			m_ID = other.m_ID;
		}
		
		bool operator==(const Object& other) const {
			return m_ID == other.m_ID;
		}
		bool operator!=(const Object& other) const  {
			return !this->operator==(other);
		}
	private:
		UUID m_ID;
	private:
		friend class App;
	};


}
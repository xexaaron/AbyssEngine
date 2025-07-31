#include "Core/Resource.h"
#include "Rendering/Context.h"


namespace aby {

    Resource::Resource() : 
        m_Type(EResource::NONE), m_Handle(null) {}
    
    Resource::Resource(EResource type, Handle handle) : 
        m_Type(type), m_Handle(handle) {}

    EResource Resource::type() const { 
        return m_Type;
    }
    
    Resource::Handle Resource::handle() const { 
        return m_Handle;
    }

    Resource::operator bool() const {
        return (m_Type != EResource::NONE);
    }
    
    bool Resource::operator==(const Resource& other) const {
        return ((m_Type == other.m_Type) && (m_Handle == other.m_Handle));
    }
    
    bool Resource::operator!=(const Resource& other) const {
        return !this->operator==(other);
    }

}


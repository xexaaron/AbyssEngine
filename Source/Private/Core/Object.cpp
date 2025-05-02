#include "Core/Object.h"

namespace aby {

	bool Object::operator==(const Object& other) const {
		return m_ID == other.m_ID;
	}
	bool Object::operator!=(const Object& other) const {
		return !this->operator==(other);
	}

	util::UUID Object::uuid() const {
		return m_ID;
	}

	bool Object::on_deserialize(Serializer& serializer) {
		return false;
	}


}
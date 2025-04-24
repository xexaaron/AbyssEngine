#pragma once

#include "Core/Common.h"
#include <algorithm>

namespace aby::util {

	struct Insertion {
		std::size_t pos;
		std::string text;
	
		bool operator<(const Insertion& other) const;
	};

	/**
	* @brief Insert multiple strings into a buffer without having to keep track
	*		 of offsets.
	*/
	void multi_insert(std::string& buffer, std::vector<Insertion> insertions);
}
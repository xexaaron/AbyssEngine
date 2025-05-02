#pragma once

#include "Core/Common.h"
#include <algorithm>
#include <map>
#include <vector>
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

	template <typename Value, typename Fn>
	auto map_vector(const std::vector<Value>& vec, Fn&& get_key) {
		using Key = std::invoke_result_t<Fn, const Value&>;
		std::multimap<Key, Value> result;
		for (const auto& element : vec) {
			result.insert({ get_key(element), element });
		}
		return result;
	}
}
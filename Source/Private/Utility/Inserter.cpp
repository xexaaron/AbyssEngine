 
#include "Utility/Inserter.h"

namespace aby::util {

	bool Insertion::operator<(const Insertion& other) const {
		return pos < other.pos;
	}

	void multi_insert(std::string& buffer, std::vector<Insertion> insertions) {
		std::sort(insertions.begin(), insertions.end());
		std::size_t last_pos = 0, shift = 0;
		std::string result;
		for (const auto& ins : insertions) {
			std::size_t adj_pos = ins.pos + shift;
			result += buffer.substr(last_pos, ins.pos - last_pos);
			result += ins.text;
			last_pos = ins.pos;
			shift += ins.text.length();
		}
		result += buffer.substr(last_pos);
		buffer = std::move(result);
	}

}
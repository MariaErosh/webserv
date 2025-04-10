#pragma once

#include <algorithm>

namespace Utils {

	class Containers {
	public:
		template<class Container>
		static bool   contains(Container container, const typename Container::value_type& value);

	};

	template<class Container> 
	bool	Containers::contains(Container container, const typename Container::value_type& value) {
			return std::find(container.begin(), container.end(), value) != container.end();
	}  
}

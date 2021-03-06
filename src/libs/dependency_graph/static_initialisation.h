#pragma once

#include <map>
#include <functional>
#include <memory>

namespace dependency_graph {

class Data;

/// An explicit static data holder, to allow for explicit control over static data destruction order.
/// This is not a well encapsulated class, but as it is quite internal and not part of the main API, I am leaving it like this for now.
class StaticInitialisation {
	public:
		StaticInitialisation();
		~StaticInitialisation();

		static void registerDataFactory(const std::string& type, std::function<Data()> fn);
		static const std::map<std::string, std::function<Data()>>& dataFactories();

		/// Private implementation structure
		struct Pimpl;

	private:
		std::unique_ptr<Pimpl> m_pimpl;

		StaticInitialisation(const StaticInitialisation&) = delete;
		StaticInitialisation& operator = (const StaticInitialisation&) = delete;
};

}

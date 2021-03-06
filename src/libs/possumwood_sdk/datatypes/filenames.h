#pragma once

#include <string>
#include <set>

#include <boost/filesystem/path.hpp>

#include <dependency_graph/data_traits.h>

#include "actions/io.h"
#include "actions/traits.h"

namespace possumwood {

class Filenames {
	public:
		Filenames(std::initializer_list<std::string> extensions = std::initializer_list<std::string>());

		const std::vector<boost::filesystem::path> filenames(bool makeAbsolute = true) const;

		bool empty() const;
		void addFilename(const boost::filesystem::path& filename);
		void clear();

		const std::set<std::string>& extensions() const;

		Filenames(const Filenames& fn);
		Filenames& operator = (const Filenames& fn);

		bool operator == (const Filenames& fn) const;
		bool operator != (const Filenames& fn) const;

	private:
		std::vector<boost::filesystem::path> m_filenames;
		std::set<std::string> m_extensions;
};

template<>
struct Traits<Filenames> {
	static IO<Filenames> io;

	static constexpr std::array<float, 3> colour() {
		return std::array<float, 3>{{1, 1, 0.5}};
	}
};

std::ostream& operator << (std::ostream& out, const Filenames& f);

}

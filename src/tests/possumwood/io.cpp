#include <dependency_graph/rtti.h>
#include <dependency_graph/data.inl>

#include <actions/io.h>
#include <actions/node_data.h>

#include "io.h"

namespace dependency_graph { namespace io {

struct SaveableRegistration {
	SaveableRegistration() {
		setIsSaveableCallback([](const BaseData&) {
			return true;
		});
	}

	~SaveableRegistration() {
		setIsSaveableCallback(std::function<bool(const BaseData& data)>());
	}
};

static SaveableRegistration s_saveableRegistration;

} }

namespace possumwood { namespace io {

void fromJson(const json& j, dependency_graph::BaseData& data) {
	if(data.type() == "float") {
		data.set<float>(j.get<float>());
	}
	else if(data.type() == dependency_graph::unmangledTypeId<std::string>()) {
		data.set<std::string>(j.get<std::string>());
	}
	else if(data.type() == dependency_graph::unmangledTypeId<possumwood::NodeData>()) {
		data.set<possumwood::NodeData>(possumwood::NodeData());
	}
	else
		assert(false);
}

void toJson(json& j, const dependency_graph::BaseData& data) {
	if(data.type() == "float") {
		j = data.get<float>();
	}
	else if(data.type() == dependency_graph::unmangledTypeId<std::string>()) {
		j = data.get<std::string>();
	}
	else if(data.type() == dependency_graph::unmangledTypeId<possumwood::NodeData>()) {
		// const dependency_graph::TypedData<possumwood::NodeData>& typed = dynamic_cast<const dependency_graph::TypedData<possumwood::NodeData>&>(data);
		j = "test blind data";
	}
	else
		assert(false);
}

bool isSaveable(const dependency_graph::BaseData& data) {
	return true;
}

} }

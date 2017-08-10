#include "editor.h"

#include <dependency_graph/node.h>

namespace possumwood {

Editor::Editor() {
}

Editor::~Editor() {
	for(auto& c : m_connections)
		c.disconnect();
}

void Editor::valueChanged(const dependency_graph::Attr& attr) {
	// do nothing, unless implemented
}

dependency_graph::Values& Editor::values() {
	assert(m_values);
	return *m_values;
}

const dependency_graph::Values& Editor::values() const {
	assert(m_values);
	return *m_values;
}

void Editor::setNodeReference(dependency_graph::Node& node) {
	m_values = std::unique_ptr<dependency_graph::Values>(new dependency_graph::Values(node));

	for(unsigned pi = 0; pi < node.portCount(); ++pi) {
		m_connections.push_back(node.port(pi).valueCallback(
			[&node, pi, this]() {
				valueChanged(node.metadata().attr(pi));
			}
		));
	}
}

}
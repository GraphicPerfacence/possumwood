#include "cgal/datatypes/meshes.h"

#include <boost/algorithm/string/join.hpp>

#include <possumwood_sdk/node_implementation.h>
#include <possumwood_sdk/app.h>

#include <GL/glew.h>
#include <GL/glu.h>

#include <ImathVec.h>

#include "datatypes/vertex_data.inl"

namespace {

template <std::size_t WIDTH, typename T>
void addPerPointVBO(possumwood::VertexData& vd, const std::string& name,
                    const std::string& propertyName, std::size_t triangleCount,
                    const possumwood::Meshes& mesh) {
	vd.addVBO<float>(
	    name, triangleCount * 3, WIDTH, possumwood::VertexData::kStatic,
	    [mesh, propertyName](possumwood::Buffer<float>& buffer) {
		    std::size_t index = 0;

		    for(auto& m : mesh) {
			    auto vertProp =
			        m.mesh().property_map<possumwood::CGALPolyhedron::Vertex_index, T>(
			            propertyName.c_str());

			    auto faceProp =
			        m.mesh().property_map<possumwood::CGALPolyhedron::Face_index, T>(
			            propertyName.c_str());

			    // vertex props, handled by polygon-triangle
			    if(vertProp.second) {
				    for(auto fit = m.mesh().faces_begin(); fit != m.mesh().faces_end();
				        ++fit) {
					    auto vertices =
					        m.mesh().vertices_around_face(m.mesh().halfedge(*fit));

					    if(vertices.size() >= 2) {
						    auto it = vertices.begin();

						    auto& val1 = vertProp.first[*it];
						    ++it;

						    auto& val2 = vertProp.first[*it];
						    ++it;

						    while(it != vertices.end()) {
							    auto& val = vertProp.first[*it];

							    buffer.element(index++) = val1;
							    buffer.element(index++) = val2;
							    buffer.element(index++) = val;

							    ++it;
						    }
					    }
				    }
			    }

			    // face props, constant for all triangles of a face
			    else if(faceProp.second) {
				    // iterate over faces
				    for(auto fit = m.mesh().faces_begin(); fit != m.mesh().faces_end();
				        ++fit) {
					    auto vertices =
					        m.mesh().vertices_around_face(m.mesh().halfedge(*fit));

					    auto& n = faceProp.first[*fit];

					    if(vertices.size() >= 2) {
						    auto it = vertices.begin();

						    ++it;
						    ++it;

						    while(it != vertices.end()) {
							    buffer.element(index++) = n;
							    buffer.element(index++) = n;
							    buffer.element(index++) = n;

							    ++it;
						    }
					    }
				    }
			    }
			    else
				    throw std::runtime_error("Property " + propertyName +
				                             " not found as vertex or face property.");
		    }

		    assert(index == buffer.vertexCount());
		});
}

struct VBOName {
	std::string name, type;
	std::size_t arraySize;
};

VBOName CGALType(const std::string& t) {
	VBOName result;
	result.arraySize = 0;

	auto colonPos = t.find(":");
	if(colonPos != std::string::npos) {
		result.type = t.substr(0, colonPos);
		result.name = t.substr(colonPos + 1);
		result.arraySize = 1;

		assert(colonPos > 0);
		if(result.type.back() == ']') {
			auto bracketPos = t.find("[");
			assert(bracketPos != std::string::npos);

			result.arraySize = std::stoi(result.type.substr(bracketPos+1, result.type.length()-bracketPos-2));
			result.type = result.type.substr(0, bracketPos);
		}
	}

	return result;
}

using possumwood::Meshes;
using possumwood::CGALPolyhedron;

// properties to be skipped during handling - either internal to CGAL, or handled
// separately
static const std::set<std::string> skipProperties{
    {"f:connectivity", "f:removed", "v:connectivity", "v:removed", "v:point"}};

dependency_graph::OutAttr<std::shared_ptr<const possumwood::VertexData>> a_vd;
dependency_graph::InAttr<Meshes> a_mesh;

dependency_graph::State compute(dependency_graph::Values& data) {
	dependency_graph::State result;

	const Meshes mesh = data.get(a_mesh);

	// we're drawing triangles
	std::unique_ptr<possumwood::VertexData> vd(new possumwood::VertexData(GL_TRIANGLES));

	// first, figure out how many triangles we have
	//   there has to be a better way to do this, come on
	std::size_t triangleCount = 0;
	for(auto& m : mesh)
		for(auto it = m.mesh().faces_begin(); it != m.mesh().faces_end(); ++it) {
			auto vertices = m.mesh().vertices_around_face(m.mesh().halfedge(*it));

			if(vertices.size() > 2)
				triangleCount += (vertices.size() - 2);
		}

	// and build the buffers
	if(triangleCount > 0) {
		// transfer the position data
		addPerPointVBO<3, possumwood::CGALKernel::Point_3>(*vd, "position", "v:point",
		                                                   triangleCount, mesh);

		// count the properties - only properties consistent between all meshes can be
		// transfered. We don't care about the type of the properties, though - face or
		// vertex are both fine
		std::map<std::string, std::size_t> propertyCounters;
		for(auto& m : mesh) {
			auto vertPropList =
			    m.mesh().properties<possumwood::CGALPolyhedron::Vertex_index>();
			for(auto& vp : vertPropList)
				++propertyCounters[vp];

			auto facePropList =
			    m.mesh().properties<possumwood::CGALPolyhedron::Face_index>();
			for(auto& fp : facePropList)
				++propertyCounters[fp];
		}

		// and transfer all properties that are common to all meshes
		std::set<std::string> ignoredProperties, inconsistentProperties;
		for(auto& pc : propertyCounters)
			if(skipProperties.find(pc.first) == skipProperties.end()) {
				if(pc.second == mesh.size()) {
					// analyse the type
					VBOName name = CGALType(pc.first);
					if(name.arraySize > 0) {
						// yep, the types are hardcoded for now, sorry :(
						if(name.type == "vec3" && name.arraySize == 1)
 							addPerPointVBO<3, std::array<float, 3>>(
							    *vd, name.name, pc.first, triangleCount, mesh);

						else if(name.type == "float" && name.arraySize == 1)
							addPerPointVBO<1, float>(
							    *vd, name.name, pc.first, triangleCount, mesh);

						// else if(name.type == "float")
						// 	addPerPointVBO<1, std::vector<float>>(
						// 	    *vd, name.name, pc.first, triangleCount, mesh);

						else
							ignoredProperties.insert(pc.first);
					}
					else if(pc.second < mesh.size())
						ignoredProperties.insert(pc.first);
				}
				else
					inconsistentProperties.insert(pc.first + "(" +
					                              std::to_string(pc.second) + "/" +
					                              std::to_string(mesh.size()) + ")");
			}

		if(!inconsistentProperties.empty())
			result.addWarning(
			    "The following properties were not common to all meshes, and had to be "
			    "ignored:\n" +
			    boost::algorithm::join(inconsistentProperties, ", "));

		if(!ignoredProperties.empty())
			result.addWarning("The following property types were not recognized:\n" +
			                  boost::algorithm::join(ignoredProperties, ", "));
	}

	data.set(a_vd, std::shared_ptr<const possumwood::VertexData>(vd.release()));

	return result;
}

void init(possumwood::Metadata& meta) {
	meta.addAttribute(a_vd, "vertex_data");
	meta.addAttribute(a_mesh, "mesh");

	meta.addInfluence(a_mesh, a_vd);

	meta.setCompute(&compute);
}

possumwood::NodeImplementation s_impl("render/vertex_data/cgal", init);
}

/* ------------------------------------------------------------------   */
/*      item            : VSGXMLReader.cxx
        made by         : Rene' van Paassen
        date            : 230201
        category        : body file
        description     :
        changes         : 230201 first version
        language        : C++
        copyright       : (c) 23 TUDelft-AE-C&S
*/

#include "VSGXMLReader.hxx"
#include <pugixml.hpp>
#include <dueca/debug.h>

inline void ltrim(std::string &s) {
  s.erase(s.begin(), std::find_if(s.begin(), s.end(),
                                  [](int ch) {
                                    return !std::isspace(ch);
                                  }));
}

// trim from end (in place)
inline void rtrim(std::string &s) {
  s.erase(std::find_if(s.rbegin(), s.rend(),
                       [](int ch) {
                         return !std::isspace(ch);
                       }).base(), s.end());
}

inline std::string trim_copy(std::string s) {
  ltrim(s);
  rtrim(s);
  return s;
}

inline std::vector<double> getValues(const std::string& s)
{
  std::stringstream invals(s);
  std::vector<double> res;
  double tmp; invals >> tmp;
  while (invals.good()) {
    res.push_back(tmp); invals >> tmp;
  }
  return res;
}

VSGXMLReader::CoordinateMapping::CoordinateMapping(unsigned offset,
						   unsigned size) :
  offset(offset), size(size)
{ }

bool VSGXMLReader::ObjectCoordinateMapping::
getMapping(unsigned &offset, unsigned &size, const std::string& cname)
{
  // if no coordinate names given, assume this simply fills the array
  if (!cname.size()) { return true; }

  const auto idx = mappings.find(cname);
  if (idx == mappings.end()) { return false; }
  offset = idx->second.offset;
  size = idx->second.size;
  return true;
}

VSGXMLReader::VSGXMLReader(const std::string& definitions)
{
  // shortcut exit
  if (!definitions.size()) return;

  // read the coordinate definitions
  pugi::xml_document doc;
  auto result = doc.load_file(definitions.c_str());

  if (result) {
    // basic element is maps
    auto _maps = doc.child("maps");

    // run over all defined types
    for (auto _type = _maps.child("type"); _type;
	 _type = _type.next_sibling("type")) {
      auto nm = object_mappings.emplace
	(_type.attribute("name").value(),
	 ObjectCoordinateMapping());
      
      for (auto _coord = _type.child("coordinates"); _coord;
	   _coord = _coord.next_sibling("coordinates")) {
	unsigned offset = _coord.attribute("offset").as_uint();
	unsigned nelts = _coord.attribute("size").as_uint(1U);
	std::string name = trim_copy(_coord.value());
        nm.first->second.mappings.emplace
	  (std::piecewise_construct,
	   std::forward_as_tuple(name),
	   std::forward_as_tuple(offset, nelts));
      }
    }
  }
}

void VSGXMLReader::readWorld(const std::string& file, VSGViewer &viewer)
{
  pugi::xml_document doc;
  auto result = doc.load_file(file.c_str());
  
  // get the container
  auto world = doc.child("world");

  // each declaration gets translated in data for a createable object
  // either through direct creation or from a channel entry
  for (auto def = world.child("definition"); def;
       def = def.next_sibling("definition")) {

    // Prepare the data for the object
    WorldDataSpec spec;

    // required stuff is key and type
    auto _key = def.attribute("key");
    auto _type = def.attribute("type");
    auto _name = def.attribute("name");
    auto _parent = def.attribute("parent");

    // test required attributes are there
    if (!_key || !_type) {
      W_MOD("Skipping definition, missing key and type");
      continue;
    }

    // store the data
    spec.type = _type.value();
    if (_name) { spec.name = _name.value(); }
    if (_parent) { spec.parent = _parent.value(); }

    // get all the files/string data
    for (auto fname = def.child("file"); fname;
         fname = fname.next_sibling("file")) {
      spec.filename.push_back(trim_copy(fname.value()));
    }

    // now get&translate all coordinates
    for (auto coord = def.child("coordinate"); coord;
         coord = coord.next_sibling("coordinate")) {
      std::string _label = coord.attribute("name").value();
      auto values = getValues(coord.value());
      unsigned offset = 0;
      unsigned n = values.size();
      auto idx = object_mappings.find(spec.type);
      
      if (idx != object_mappings.end()) {
        if (!idx->second.getMapping(offset, n, _label)) {
	  W_MOD("Coordinate index '" << _label << "' for type '" <<
		spec.type << "' missing");
	  continue;
	}
      }
      else if (_label.size()) {
	W_MOD("Coordinate mappings for type '" << spec.type <<
	      "' missing");
	continue;
      }
      spec.setCoordinates(offset, n, values);
    }
  }
}

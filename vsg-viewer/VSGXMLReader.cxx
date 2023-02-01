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
    res.append(tmp); invals >> tmp;
  }
  return res;
}

void VSGXMLRead(const std::string& file, VSGViewer &viewer)
{
  static bool get_mapping = true;
  static std::map<std::string, CoordinateMapping> mapping;
  if (get_mapping) {
    pugi::xml_document doc;
    auto result = doc.load_file
      ("../../../../WorldView/vsg-viewer/mapping.xml");
    if (result) {
      auto _maps = doc.child("maps");
      for (auto _type = _maps.child("type"); _type;
           _type = _maps.next_sibling("type")) {
        CoordinateMapping newmap(_type);
        mapping[newmap.name()] = newmap;
      }
    }
  }

  pugi::xml_document doc;
  auto result = doc.load_file(optionfile.c_str());

  // get the container
  auto world = doc.child("world");

  for (auto dec = world.child("declaration"); dec;
       dec = world.next_sibling("declaration")) {

    WorldDataSpec spec;
    auto _name = dec.attribute("name");
    if (_name) { spec.name = _name.value(); }
    auto _type = dec.attribute("type");
    spec.type = _type.value();
    auto _parent = dec.attribute("parent");
    if (_parent) { spec.parent = _parent.value()};
    auto _key = dec.attribute("key");

    for (auto fname = dec.child("file"); fname;
         fname = dec.next_sibling("file")) {
      spec.filename.append(trim_copy(fname.value()));
    }
    for (auto coord = dec.child("coordinate"); coord;
         coord = dec.next_sibling("coordinate")) {
      auto _label = coord.attribute("name");
      auto values = getValues(coord.value());
      unsigned offset = 0;
      unsigned n = values.size();
      auto idx = mapping.find(spec.type);
      if (idx != mapping.end()) {
        idx->getMapping(offset, n, _label.value());
      }
      spec.setCoordinates(offset, n, values);
    }
  }
}

/* ------------------------------------------------------------------   */
/*      item            : VsgObject.cxx
        made by         : Rene' van Paassen
        date            : 090617
        category        : body file
        description     :
        changes         : 090617 first version
        language        : C++
*/

#define VsgObject_cxx
#include "VSGObject.hxx"
#include <vsgXchange/all.h>
#include "AxisTransform.hxx"
#include <map>
#include <iostream>

#define DEB(A) cout << A << endl;

using namespace std;
using namespace vsg;

VSGObject::VSGObject()
{
  //
}


VSGObject::~VSGObject()
{
  //
}

void VSGObject::unInit(const vsg::ref_ptr<vsg::Group>& root)
{
  // not defined
}

VSGCullGroup::VSGCullGroup() :
  VSGObject()
{
  //
}

VSGCullGroup::~VSGCullGroup()
{
  //
}

std::pair<const std::string, const std::string> nameSplit(const std::string& n)
{
  auto split = n.find('|');
  if (split == std::string::npos) {
    return std::pair<const std::string, const std::string>(std::string(), n);
  }
  return std::pair<const std::string, const std::string>
    (n.substr(0, split), n.substr(split+1));
}

vsg::ref_ptr<vsg::Group> findParent(vsg::ref_ptr<vsg::Group> root,
                                    const std::string& name)
{
  std::string iname;
  vsg::ref_ptr<vsg::Group> sub;

  if (!name.size()) return root;
  for (auto const &i: root->children) {
    vsg::ref_ptr<vsg::Group> g = i.cast<vsg::Group>();
    if (g->getValue("name", iname) && iname == name) {
      return g;
    }
    sub = findParent(g, name);
    if (sub) return sub;
  }
  return sub;
}

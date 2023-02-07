/* ------------------------------------------------------------------   */
/*      item            : VsgObject.cxx
        made by         : Rene' van Paassen
        date            : 090617
        category        : body file
        description     :
        changes         : 090617 first version
        language        : C++
*/

#include "VSGObject.hxx"
#include <vsgXchange/all.h>
#include "AxisTransform.hxx"
#include <map>
#include <iostream>
#include <dueca/debug.h>

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

VSGCullGroup::VSGCullGroup(const WorldDataSpec& data) :
  VSGObject()
{
  name = data.name;
  parent = data.parent;
  D_MOD("Created cull group, name=" << name);
}

VSGCullGroup::~VSGCullGroup()
{
  D_MOD("Destroying cull group, name=" << name);
}

vsg::ref_ptr<vsg::Group> findParent(vsg::ref_ptr<vsg::Group> root,
                                    const std::string& name)
{
  std::string iname;
  vsg::ref_ptr<vsg::Group> sub;

  if (!name.size()) return root;
  for (auto const &i: root->children) {

    // treat as a group
    vsg::ref_ptr<vsg::Group> g = i.cast<vsg::Group>();

    // only proceed if it is a group
    if (g) { 

      // check the name
      if (g->getValue("name", iname) && iname == name) {
	return g;
      }

      // name not matching, check children
      sub = findParent(g, name);
      if (sub) return sub;
    }
  }
  return sub;
}

void VSGObject::visible(bool vis)
{
  // nothing
}

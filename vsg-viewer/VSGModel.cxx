/* ------------------------------------------------------------------   */
/*      item            : VSGModel.cxx
        made by         : Rene' van Paassen
        date            : 230126
        category        : body file
        description     :
        changes         : 230126 first version
        language        : C++
        copyright       : (c) 23 TUDelft-AE-C&S
*/

#include "VSGModel.hxx"
#include "VSGViewer.hxx"
#include "VSGObjectFactory.hxx"
#include <dueca/debug.h>

VSGStaticModel::VSGStaticModel(const WorldDataSpec& data) :
  model(),
  modelfile(data.filename.size() ? std::string() : data.filename[0])
{
  name = data.name;
  parent = data.parent;
  if (!data.filename.size()) {
    W_MOD("Static model needs a model filename");
  }
  D_MOD("Created visual model, name=" << name);
}


VSGStaticModel::~VSGStaticModel()
{
  D_MOD("Destroying visual model, name=" << name);
}

void VSGStaticModel::init(const vsg::ref_ptr<vsg::Group>& root,
			  VSGViewer* master)
{
  model = vsg::read_cast<vsg::Node>(modelfile, master->options);
  model->setValue("name", name);
  auto par = findParent(root, parent);
  if (!par) {
    W_MOD("Cannot find parent, for name=" << name << ", attaching to root");
    par = root;
  }
  par->addChild(model);
  D_MOD("VSG create visual model, name=" << name);
}

static auto VSGStaticModel_maker = new
  SubContractor<VSGObjectTypeKey,VSGStaticModel>
  ("static-model", "3D model from external modeling application");

VSGModel::VSGModel(const WorldDataSpec& data) :
  VSGStaticModel(data),
  VSGAbsoluteTransform(data)
{
  name = data.name;
  D_MOD("Created static model, name=" << name);

}

VSGModel::~VSGModel()
{
  D_MOD("Destroying static model, name=" << name);
}

void VSGModel::init(const vsg::ref_ptr<vsg::Group>& root,
		    VSGViewer* master)
{
  model = vsg::read_cast<vsg::Node>(modelfile, master->options);
  VSGAbsoluteTransform::init(root, master);
  transform->addChild(model);
  D_MOD("VSG create visual model, name=" << name);
}

static auto VSGModel_maker = new
  SubContractor<VSGObjectTypeKey,VSGModel>
  ("model", "controlled 3D model from external modeling application");

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
  if (!data.filename.size()) {
    W_MOD("Static model needs a model filename");
  }
}


VSGStaticModel::~VSGStaticModel()
{
  //
}

void VSGStaticModel::init(const vsg::ref_ptr<vsg::Group>& root,
			  VSGViewer* master)
{
  auto _name = nameSplit(name);
  model = vsg::read_cast<vsg::Node>(modelfile, master->options);
  model->setValue("name", _name.second);
  findParent(root, _name.first)->addChild(model);
}

static auto VSGStaticModel_maker = new
  SubContractor<VSGObjectTypeKey,VSGStaticModel>
  ("static-model", "3D model from external modeling application");

VSGModel::VSGModel(const WorldDataSpec& data) :
  VSGStaticModel(data),
  VSGAbsoluteTransform(data)
{
  //
}

VSGModel::~VSGModel()
{
  //
}

void VSGModel::init(const vsg::ref_ptr<vsg::Group>& root,
		    VSGViewer* master)
{
  auto _name = nameSplit(name);
  model = vsg::read_cast<vsg::Node>(modelfile, master->options);
  VSGAbsoluteTransform::init(root, master);
  transform->addChild(model);
}

static auto VSGModel_maker = new
  SubContractor<VSGObjectTypeKey,VSGModel>
  ("model", "controlled 3D model from external modeling application");

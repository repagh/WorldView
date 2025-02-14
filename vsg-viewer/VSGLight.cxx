/* ------------------------------------------------------------------   */
/*      item            : VSGLight.cxx
        made by         : Rene' van Paassen
        date            : 230125
        category        : body file
        description     :
        changes         : 230125 first version
        language        : C++
        copyright       : (c) 23 TUDelft-AE-C&S
*/

#include "VSGLight.hxx"
#include "VSGObjectFactory.hxx"
#include <dueca/debug.h>

namespace vsgviewer {

  VSGAmbientLight::VSGAmbientLight(const WorldDataSpec& data) :
    color(data.coordinates[0], data.coordinates[1], data.coordinates[2]),
    intensity(data.coordinates[3])
  {
    name = data.name;
    D_MOD("Created ambient light, name=" << name);
  }


  VSGAmbientLight::~VSGAmbientLight()
  {
    D_MOD("Destroying ambient light, name=" << name);
  }

  void VSGAmbientLight::init(const vsg::ref_ptr<vsg::Group>& root,
                             VSGViewer* master)
  {
    light = vsg::AmbientLight::create();
    light->name = name;
    light->color = color;
    light->intensity = intensity;
    root->addChild(light);
    D_MOD("VSG create ambient light, name=" << name);
  }

  static auto VSGAmbientLight_maker = new
    SubContractor<VSGObjectTypeKey,VSGAmbientLight>
    ("ambient-light", "Ambient light level definitions");

  VSGDirectionalLight::VSGDirectionalLight(const WorldDataSpec& data) :
    color(data.coordinates[0], data.coordinates[1], data.coordinates[2]),
    intensity(data.coordinates[3]),
    direction(data.coordinates[5], data.coordinates[4], data.coordinates[6])
  {
    name = data.name;
    parent = data.parent;
    D_MOD("Created directional light, name=" << name);
  }


  VSGDirectionalLight::~VSGDirectionalLight()
  {
    D_MOD("Destroying directional light, name=" << name);
  }

  void VSGDirectionalLight::init(const vsg::ref_ptr<vsg::Group>& root,
                                 VSGViewer* master)
  {
    light = vsg::DirectionalLight::create();
    light->name = name;
    light->color = color;
    light->intensity = intensity;
    light->direction = direction;
    auto par = findParent(root, parent);
    if (!par) {
      W_MOD("Cannot find parent='" << parent << "', for name=" << name <<
            ", attaching to root");
      par = root;
    }
    par->addChild(light);
    D_MOD("VSG create directional light, name=" << name);
  }

  static auto VSGDirectionalLight_maker = new
    SubContractor<VSGObjectTypeKey,VSGDirectionalLight>
    ("directional-light", "Light coming from a specific direction");

  VSGPointLight::VSGPointLight(const WorldDataSpec& data) :
    color(data.coordinates[0], data.coordinates[1], data.coordinates[2]),
    intensity(data.coordinates[3]),
    position(data.coordinates[5], data.coordinates[4], data.coordinates[6]),
    span(data.coordinates[7])
  {
    name = data.name;
    parent = data.parent;
    D_MOD("Created point light, name=" << name);  
  }


  VSGPointLight::~VSGPointLight()
  {
    D_MOD("Destroying point light, name=" << name);
  }

  void VSGPointLight::init(const vsg::ref_ptr<vsg::Group>& root,
                           VSGViewer* master)
  {
    light = vsg::PointLight::create();
    cull = vsg::CullGroup::create();

    light->name = name;
    light->color = color;
    light->intensity = intensity;
    light->position = position;
    cull->bound.center = light->position;
    cull->bound.radius = span;
    cull->addChild(light);
    auto par = findParent(root, parent);
    if (!par) {
      W_MOD("Cannot find parent='" << parent << "', for name=" << name <<
            ", attaching to root");
      par = root;
    }
    par->addChild(light);
    D_MOD("VSG create point light, name=" << name);
  }

  static auto VSGPointLight_maker = new
    SubContractor<VSGObjectTypeKey,VSGPointLight>
    ("point-light", "Point-type light");

  VSGSpotLight::VSGSpotLight(const WorldDataSpec& data) :
    color(data.coordinates[0], data.coordinates[1], data.coordinates[2]),
    intensity(data.coordinates[3]),
    position(data.coordinates[5], data.coordinates[4], data.coordinates[6]),
    span(data.coordinates[7]),
    innerangle(data.coordinates[10]),
    outerangle(data.coordinates[11]),
    direction(data.coordinates[13], data.coordinates[12], data.coordinates[14])
  {
    name = data.name;
    parent = data.parent;
    D_MOD("Created spot light, name=" << name);
  }


  VSGSpotLight::~VSGSpotLight()
  {
    //
  }

  void VSGSpotLight::init(const vsg::ref_ptr<vsg::Group>& root,
                          VSGViewer* master)
  {
    light = vsg::SpotLight::create();
    cull = vsg::CullGroup::create();

    light->name = name;
    light->color = color;
    light->intensity = intensity;
    light->position = position;
    light->direction = direction;
    light->innerAngle = vsg::radians(innerangle);
    light->outerAngle = vsg::radians(outerangle);
    cull->bound.center = light->position;
    cull->bound.radius = span;
    cull->addChild(light);
    auto par = findParent(root, parent);
    if (!par) {
      W_MOD("Cannot find parent='" << parent << "', for name=" << name <<
            ", attaching to root");
      par = root;
    }
    par->addChild(light);
    D_MOD("VSG create spot light, name=" << name);
  }

  static auto VSGSpotLight_maker = new
    SubContractor<VSGObjectTypeKey,VSGSpotLight>
    ("spot-light", "Directed spot light");

}; // namespace

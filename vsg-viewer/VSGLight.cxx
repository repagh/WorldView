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


VSGAmbientLight::VSGAmbientLight(const WorldDataSpec& data) :
  color(data.coordinates[0], data.coordinates[1], data.coordinates[2]),
  intensity(data.coordinates[3])
{
  name = data.name;
}


VSGAmbientLight::~VSGAmbientLight()
{
  //
}

void VSGAmbientLight::init(const vsg::ref_ptr<vsg::Group>& root,
                           VSGViewer* master)
{
  light = vsg::AmbientLight::create();
  auto _name = nameSplit(name);
  light->name = _name.second;
  light->color = color;
  light->intensity = intensity;
  root->addChild(light);
}

VSGDirectionalLight::VSGDirectionalLight(const WorldDataSpec& data) :
  color(data.coordinates[0], data.coordinates[1], data.coordinates[2]),
  intensity(data.coordinates[3]),
  direction(data.coordinates[4], data.coordinates[5], data.coordinates[6])
{
  name = data.name;
}


VSGDirectionalLight::~VSGDirectionalLight()
{
  //
}

void VSGDirectionalLight::init(const vsg::ref_ptr<vsg::Group>& root,
                           VSGViewer* master)
{
  light = vsg::DirectionalLight::create();
  auto _name = nameSplit(name);
  light->name = _name.second;
  light->color = color;
  light->intensity = intensity;
  light->direction = direction;
  findParent(root, _name.first)->addChild(light);
}

VSGPointLight::VSGPointLight(const WorldDataSpec& data) :
  color(data.coordinates[0], data.coordinates[1], data.coordinates[2]),
  intensity(data.coordinates[3]),
  position(data.coordinates[4], data.coordinates[5], data.coordinates[6]),
  span(data.coordinates[7])
{
  name = data.name;
}


VSGPointLight::~VSGPointLight()
{
  //
}

void VSGPointLight::init(const vsg::ref_ptr<vsg::Group>& root,
                           VSGViewer* master)
{
  light = vsg::PointLight::create();
  cull = vsg::CullGroup::create();

  auto _name = nameSplit(name);
  light->name = _name.second;
  light->color = color;
  light->intensity = intensity;
  light->position = position;
  cull->bound.center = light->position;
  cull->bound.radius = span;
  cull->addChild(light);
  findParent(root, _name.first)->addChild(cull);
}

VSGSpotLight::VSGSpotLight(const WorldDataSpec& data) :
  color(data.coordinates[0], data.coordinates[1], data.coordinates[2]),
  intensity(data.coordinates[3]),
  position(data.coordinates[4], data.coordinates[5], data.coordinates[6]),
  span(data.coordinates[7])
{
  name = data.name;
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

  auto _name = nameSplit(name);
  light->name = _name.second;
  light->color = color;
  light->intensity = intensity;
  light->position = position;
  light->direction = direction;
  light->innerAngle = vsg::radians(innerangle);
  light->outerAngle = vsg::radians(outerangle);
  cull->bound.center = light->position;
  cull->bound.radius = span;
  cull->addChild(light);
  findParent(root, _name.first)->addChild(cull);
}

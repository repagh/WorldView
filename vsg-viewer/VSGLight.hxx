/* ------------------------------------------------------------------   */
/*      item            : VSGLight.hxx
        made by         : Rene van Paassen
        date            : 230125
        category        : header file
        description     :
        changes         : 230125 first version
        language        : C++
        copyright       : (c) 23 TUDelft-AE-C&S
*/

#pragma once
#include "VSGObject.hxx"

/** Ambient lighting definition */
class VSGAmbientLight: public VSGObject
{
  /** VSG node */
  vsg::ref_ptr<vsg::AmbientLight> node;
public:
  /** Constructor */
  VSGAmbientLight();

  /** Destructor */
  ~VSGAmbientLight();

  /** Initialise the light with the VSG scene */
  virtual void init(const vsg::ref_ptr<vsg::Group>& root,
		    VSGViewer* master) override;
};

class VSGDirectionalLight: public VSGObject
{
  /** VSG nodes */
  vsg::ref_ptr<vsg::DirectionalLight> node;
public:
  /** Constructor */
  VSGDirectionalLight();

  /** Destructor */
  ~VSGDirectionalLight();

  /** Initialise the light with the VSG scene */
  virtual void init(const vsg::ref_ptr<vsg::Group>& root,
		    VSGViewer* master) override;
};

class VSGPointLight: public VSGObject
{
  /** VSG nodes */
  vsg::ref_ptr<vsg::PointLight> node;
public:
  /** Constructor */
  VSGPointLight();

  /** Destructor */
  ~VSGPointLight();

  /** Initialise the light with the VSG scene */
  virtual void init(const vsg::ref_ptr<vsg::Group>& root,
		    VSGViewer* master) override;
};

class VSGSpotLight: public VSGObject
{
  /** VSG nodes */
  vsg::ref_ptr<vsg::SpotLight> node;
public:
  /** Constructor */
  VSGSpotLight();

  /** Destructor */
  ~VSGSpotLight();

  /** Initialise the light with the VSG scene */
  virtual void init(const vsg::ref_ptr<vsg::Group>& root,
		    VSGViewer* master) override;
};

class VSGHeadLight: public VSGObject
{
  /** VSG nodes */
  vsg::ref_ptr<vsg::SpotLight> node;
public:
  /** Constructor */
  VSGSpotLight();

  /** Destructor */
  ~VSGSpotLight();

  /** Initialise the light with the VSG scene */
  virtual void init(const vsg::ref_ptr<vsg::Group>& root,
		    VSGViewer* master) override;
};

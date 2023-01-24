/* ------------------------------------------------------------------   */
/*      item            : VsgObject.hxx
        made by         : Rene van Paassen
        date            : 090617
        category        : header file
        description     :
        changes         : 090617 first version
        language        : C++
*/

#pragma once
#include <vsg/all.h>
#include <WorldObjectBase.hxx>
#include <string>

class VSGViewer;

/** Base class for OpenSceneGraph objects that are controlled from the
    simulation.

    Typically, a derived class has

    - a constructor with (const WorldDataSpec& x) as argument

    - a connect() function that creates a read token for the data from
      the world channel data

    - an iterate() function that reads the channel data and updates
      the object
*/
class VSGObject: public WorldObjectBase
{
protected:
  /** The vsg node */
  vsg::ref_ptr<vsg::Node> node;

public:
  /** Constructor */
  VSGObject();

  /** Destructor */
  virtual ~VSGObject();

  /** Initialise the avatar with the VSG scene */
  virtual void init(const vsg::ref_ptr<vsg::Group>& root, VSGViewer* master) = 0;

  /** De-initialise the avatar with the VSG scene */
  virtual void unInit(const vsg::ref_ptr<vsg::Group>& root);

  /** Control visibility */
  virtual void visible(bool vis);

public:

  /** Returns true if the object needs drawing post-access */
  virtual bool requirePostDrawAccess() { return false; }
};

#endif

/* ------------------------------------------------------------------   */
/*      item            : WorldObjectBase.hxx
        made by         : Rene van Paassen
        date            : 100122
        category        : header file
        description     :
        changes         : 100122 first version
        language        : C++
*/

#pragma once
#include <string>
#include <dueca/dueca.h>
#include "comm-objects.h"

#ifdef USE_BOOST_IRC
#include <boost/smart_ptr/intrusive_ref_counter.hpp>
#endif

class ObjectMotion;

/** Base object for visual things in some scene graphing
    world that are connected to and controlled by a channel entry.
    Currently appropriate for plib, ogre, OSG and VSG. */
class WorldObjectBase
#ifdef USE_BOOST_IRC
  : public boost::intrusive_ref_counter<WorldObjectBase>
#endif
{
#ifndef USE_BOOST_IRC
  /** ref counter */
  unsigned intrusive_refcount;

  friend void intrusive_ptr_add_ref(WorldObjectBase*);
  friend void intrusive_ptr_release(WorldObjectBase*);
#endif

protected:
  /** Name of this thing. */
  std::string  name;

public:

#ifndef USE_BOOST_IRC
  /** Constructor */
  WorldObjectBase();
#endif
  
  /** Information about the name of this object, might be useful. */
  inline const std::string& getName() const { return name; }
  
  /** Control visibility */
  virtual void visible(bool vis);

  /** Read new input or otherwise update the state.
      
      @param ts    Time for this update
      @param base  Position and orientation of the base listener
      @param late  Time past this update's time spec
  */
  virtual void iterate(TimeTickType ts, const BaseObjectMotion& base,
                       double late);

  /** Connect to a channel entry
      @param master_id ID for opening a channel reader
      @param cname     Channel with object data
      @param entry_id  Entry in the channel */
  virtual void connect(const GlobalId& master_id, const NameSet& cname,
                       entryid_type entry_id,
                       Channel::EntryTimeAspect time_aspect);

};


#ifndef USE_BOOST_IRC
/** Support for intrusive pointer */
void intrusive_ptr_add_ref(WorldObjectBase* t);

/** Support for intrusive pointer */
void intrusive_ptr_release(WorldObjectBase* t);
#endif

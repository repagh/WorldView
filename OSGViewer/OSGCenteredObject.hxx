/* ------------------------------------------------------------------   */
/*      item            : OSGStaticObject.hxx
        made by         : Rene van Paassen
        date            : 100127
	category        : header file 
        description     : 
	changes         : 100127 first version
        language        : C++
*/

#ifndef OSGCenteredObject_hxx
#define OSGCenteredObject_hxx

#include "OSGStaticObject.hxx"
#include "comm-objects.h"

/** Class of objects that (in one or more coordinates) may stay
    centered on the observer */
class OSGCenteredObject: public OSGStaticObject
{
public:
  /** Constructor */
  OSGCenteredObject(const WorldDataSpec &specification);
  
  /** Destructor */
  ~OSGCenteredObject();

  /** Connect to a channel entry 

      @param master_id ID for opening a channel reader
      @param cname     dueca::Channel with object data
      @param entry_id  Entry in the channel */
  virtual void connect(const dueca::GlobalId& master_id, const dueca::NameSet& cname,
                       dueca::entryid_type entry_id,
                       dueca::Channel::EntryTimeAspect time_aspect);

  /** Play, update, recalculate, etc. */
  void iterate(dueca::TimeTickType ts, const BaseObjectMotion& base, double late);
};


#endif

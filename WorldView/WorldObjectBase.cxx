/* ------------------------------------------------------------------   */
/*      item            : WorldObjectBase.cxx
        made by         : Rene' van Paassen
        date            : 100122
	category        : body file 
        description     : 
	changes         : 100122 first version
        language        : C++
*/

#define WorldObjectBase_cxx
#include "WorldObjectBase.hxx"

#ifndef USE_BOOST_IRC
void intrusive_ptr_add_ref(WorldObjectBase* t)
{
  t->intrusive_refcount++;
}

void intrusive_ptr_release(WorldObjectBase* t)
{
  if (--(t->intrusive_refcount) == 0) {
    delete t;
  }
}
#endif

WorldObjectBase::WorldObjectBase() :
  intrusive_refcount(0)
{

}


WorldObjectBase::~WorldObjectBase()
{

}
#if 0
void WorldObjectBase::connect(const GlobalId& master_id, const NameSet& cname,
                              entryid_type entry_id,
                              Channel::EntryTimeAspect time_aspect)
{
  // no action
}

void WorldObjectBase::iterate(const TimeSpec& ts,
			      const BaseObjectMotion& base)
{
  // no action
}
#endif

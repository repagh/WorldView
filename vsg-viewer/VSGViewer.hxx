/* ------------------------------------------------------------------   */
/*      item            : VSGViewer.hxx
        made by         : Rene van Paassen
        date            : 090616
        category        : header file
        description     :
        changes         : 090616 first version
        language        : C++
*/

#ifndef VSGViewer_hxx
#define VSGViewer_hxx

#include <string>
#include <map>
#include <vector>
#include "VSGObject.hxx"

#include <vsg/all.h>
#include <vsgXchange/all.h>
#include <WorldViewerBase.hxx>
#include "VSGObjectFactory.hxx"

// Used the vsgwindows example as inspiration

/** This is a wrapper that can load and display VSG scenes, optionally
    in multiple windows and viewports. It is intended for
    encapsulation in a DUECA module. */
class VSGViewer: public WorldViewerBase
{
  // Advance definition, collection of data for a window.
  struct WindowSet;

  // Advance definition, collection of data for a viewport
  struct Private;

  /** Specification for the render windows. */
  std::list<WinSpec> winspec;

  /** scene manager */
  vsg::ref_ptr<vsg::Group>  root;

  /** observer is a node in the scene */
  vsg::ref_ptr<vsg::Group>  observer;

  /** A single viewer, matching a single scene */
  vsg::ref_ptr<vsg::Viewer> viewer;

  /** Options object */
  vsg::ref_ptr<vsg::Options> options;

  /** counter dynamical creation */
  unsigned config_dynamic_created;

protected:
  /** Accept unknown/unconfigured objects */
  bool allow_unknown;
private:

  /** This class can generate multiple views on the same world. A
      ViewSet encapsulates the stuff needed for a single view. */
  struct ViewSet {

    /** Name, for debugging purposes. */
    std::string name;

    /** The render camera set-up */
    vsg::ref_ptr<vsg::Camera> camera;

    /** The view of this camera */
    vsg::ref_ptr<vsg::View> view;

    /** The view matrix for the camera */
    vsg::ref_ptr<vsg::LookAt> view_matrix;

    /** The camera's offset from the base vehicle point (angle,
        distance or both) */
    vsg::t_mat4<double> eye_offset;

    /** A rendergraph */
    vsg::ref_ptr<vsg::RenderGraph> render_graph;

    /** Constructor */
    ViewSet();

    /** Initialise a view in a window

        @param vs     Specification for the view; viewport coordinates and
                      perspective/frustum, eye position+orientation
        @param viewer Overall scene viewer
        @param root   Scene root
        @param viewmatrix ?? How now
        @param bg_col Background color (4 element)

     */
    void init(const ViewSpec& vs, WindowSet& window,
              vsg::ref_ptr<vsg::Viewer> viewer,
              vsg::ref_ptr<vsg::Group>   root,
              const std::vector<float>& bg_col);

    /** create the camera and window. */
    void complete();
  };

  /** Each render window needs a specification and possibly a set of
      views */
  struct WindowSet {

    /** Descriptive name */
    std::string name;

    /** Display on which it is presented */
    std::string display;

    /** The actual window */
    vsg::ref_ptr<vsg::Window> window;

    /** Traits of the window */
    vsg::ref_ptr<vsg::WindowTraits> traits;

    /** Each window has a command graph */
    vsg::ref_ptr<vsg::CommandGraph> command_graph;

    /** A list of view sets; these represent the different render
        areas within the window */
    std::map<std::string,ViewSet> viewset;

    /** Constructor */
    WindowSet() { }
  };

  /** Map of windows */
  typedef std::map<std::string,WindowSet> WindowsMap;

  /** map with windows. */
  WindowsMap windows;

  /** Object list type */
  typedef std::list< boost::intrusive_ptr<VSGObject> > ObjectListType;

  /** Map with created (due to presence in the world channel)
      objects. They are indexed with channel entry index, and removed
      from the map when the entry is removed from the channel. */
  typedef std::vector<
    std::map<creation_key_t,
	     boost::intrusive_ptr<VSGObject> > > created_objects_t;

  /** Objects creates automatically */
  created_objects_t active_objects;

  /** Objects that are static, just get calls about new positioning */
  ObjectListType static_objects;

  /** Objects that need post-draw access */
  ObjectListType post_draw;

  /** Helper */
  WindowSet myCreateWindow(const WinSpec &ws, vsg::ref_ptr<vsg::Group> root,
                           const WindowsMap& windows);

  /** List of specifications for the wiews, will be applied later */
  std::list<ViewSpec> viewspec;

private:
  /** Helper function, loads resources from resources.cfg */
  void setupResources();

public:
  /** Constructor */
  VSGViewer();

  /** Destructor */
  ~VSGViewer();

  /** Open the window(s) */
  void init(bool waitswap);

  /** Add a window */
  void addViewport(const ViewSpec& vp);

  /** Add a window */
  void addWindow(const WinSpec& window) {winspec.push_back(window);}

  /** Pass the base camera position to the views.

      @param tick  Current DUECA time.
      @param base  Motion definition of the base observer/vehicle.
      @param late  Time [s] into current DUECA time tick.
   */
  void setBase(TimeTickType tick, const BaseObjectMotion& base, double late);

  /** Create a controllable object. Object creation depends on class of
      data supplied, further init may rely on fist data entering. */
  bool createControllable
  (const GlobalId& master_id, const NameSet& cname, entryid_type entry_id,
   uint32_t creation_id, const std::string& data_class,
   const std::string& entry_label, Channel::EntryTimeAspect time_aspect);

  /** Remove a controllable */
  void removeControllable(const dueca::NameSet& cname,
                          uint32_t creation_id);

  /** Create a static (not controlled) controllable object. */
  bool createStatic(const std::vector<std::string>& classname);

  /** Do a re-draw
      \param wait   If true, do now swap the buffers. The application
                    must later wait and swap with the waitSwap function. */
  void redraw(bool wait = false, bool reset_context = false);

  /** Wait for the swap. */
  void waitSwap();

  /** Change the configuration of the scene graph, returns true if
      successful */
  bool adaptSceneGraph(const WorldViewConfig& adapt);

protected:
  /** Path to the resources */
  std::string resourcepath;

  /** Keep the cursor or not? */
  bool keep_pointer;

  /** background/clear color */
  std::vector<float> bg_color;

  /** fog */
  enum FogMode {
    Off,
    Linear,
    Exponential,
    Exponential2
  };

  /** Fog mode */
  FogMode fog_mode;

  /** Fog density */
  double fog_density;

  /** Fog colour */
  vsg::dvec4 fog_colour;

  /** Fog start and end */
  double fog_start, fog_end;
};


#endif

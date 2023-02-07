/* ------------------------------------------------------------------   */
/*      item            : VSGViewer.cxx
        made by         : Rene' van Paassen
        date            : 090616
        category        : body file
        description     :
        changes         : 090616 first version
        language        : C++
*/

#define VSGViewer_cxx
#include "VSGViewer.hxx"
#include "WorldObjectBase.hxx"
#include "AxisTransform.hxx"
#include <boost/lexical_cast.hpp>
#include <unistd.h>
#include <cmath>
#include <deque>
#define W_MOD
#define E_MOD
#include <debug.h>

template<class T>
inline T deg2rad(const T d)
{
  return M_PI/180.0*d;
}

using namespace vsg;
using namespace std;

VSGViewer::ViewSet::ViewSet() :
  camera(NULL)
{
  //
}

namespace vsg
{
  class VSG_DECLSPEC FrustumPerspective : public Inherit<ProjectionMatrix, FrustumPerspective>
  {
  public:
    FrustumPerspective() :
      left(-0.5),
      right(0.5),
      bottom(-0.5),
      top(0.5),
      nearDistance(1.0),
      farDistance(10000.0)
      {
      }

    FrustumPerspective(double left, double right, double bottom, double top,
                       double nd, double fd) :
      left(left),
      right(right),
      bottom(bottom),
      top(top),
      nearDistance(nd),
      farDistance(fd)
      {
      }

    dmat4 transform() const override
    { return perspective(left, right, bottom, top, nearDistance, farDistance); }

    void changeExtent(const VkExtent2D&, const VkExtent2D& newExtent) override
    {
      // no meaningful way to implement this
    }

    void read(Input& input) override
    {
      ProjectionMatrix::read(input);
      input.read("left", left);
      input.read("right", right);
      input.read("bottom", bottom);
      input.read("top", top);
      input.read("nearDistance", nearDistance);
      input.read("farDistance", farDistance);
    }
    void write(Output& output) const override
    {
      ProjectionMatrix::write(output);
      output.write("left", left);
      output.write("right", right);
      output.write("bottom", bottom);
      output.write("top", top);
      output.write("nearDistance", nearDistance);
      output.write("farDistance", farDistance);
    }

    double left;
    double right;
    double bottom;
    double top;
    double nearDistance;
    double farDistance;
    };
  VSG_type_name(vsg::FrustumPerspective);  
}



void VSGViewer::ViewSet::init(const ViewSpec& spec, WindowSet& ws,
                              vsg::ref_ptr<vsg::Viewer> viewer,
                              vsg::ref_ptr<vsg::Group> root,
                              const std::vector<float>& bg_color)
{
  cout << "Creating camera " << spec.name << endl;
  name = spec.name;

  // aspect ratio
  double aspect = double(spec.portcoords[3])/double(spec.portcoords[2]);

  // perspective transformation matrix
  vsg::ref_ptr<vsg::ProjectionMatrix> perspective;

  // option 1, fov, aspect, ...
  if (spec.frustum_data.size() == 3) {
    // from fov, aspect, near dist, far dist
    perspective = vsg::Perspective::create
      (spec.frustum_data[2], aspect,
       spec.frustum_data[0], spec.frustum_data[1]);
  }

  // option 2, as frustum
  else if (spec.frustum_data.size() == 6) {
    perspective = vsg::FrustumPerspective::create
      (spec.frustum_data[2], spec.frustum_data[3],
       spec.frustum_data[4], spec.frustum_data[5],
       spec.frustum_data[0], spec.frustum_data[1]);
  }

  // viewport given
  auto viewportstate = vsg::ViewportState::create
    (spec.portcoords[0],spec.portcoords[1],
     spec.portcoords[2],spec.portcoords[3]);

  // the view matrix transforms to the camera position. At this stage,
  // assume starting at origin, only component is the eye offset 
  view_matrix = vsg::LookAt::create();
  if (spec.eye_pos.size() == 0) {
    // no eye offset
    eye_offset = vsg::t_mat4<double>();
  }
  else if (spec.eye_pos.size() == 3) {
    eye_offset = vsg::translate
      (AxisTransform::vsgPos
       (-spec.eye_pos[0], -spec.eye_pos[1], -spec.eye_pos[2]));
    // only position offset
    view_matrix->set(eye_offset);
  }
  else if (spec.eye_pos.size() == 6) {
    eye_offset = AxisTransform::vsgRotation
      (-spec.eye_pos[3], -spec.eye_pos[4], -spec.eye_pos[5]) *
      vsg::translate
      (AxisTransform::vsgPos
       (-spec.eye_pos[0], -spec.eye_pos[1], -spec.eye_pos[2]));
    // position and angle
    view_matrix->set(eye_offset);
  }

  // create camera and view
  camera = vsg::Camera::create(perspective, view_matrix, viewportstate);
  view = vsg::View::create(camera, root);

  // render graph seems to be the command structure that is called when
  // window needs to refres views
  render_graph = vsg::RenderGraph::create(ws.window, view);
  render_graph->clearValues[0].color = {{
      bg_color[0], bg_color[1], bg_color[2], bg_color[3] }};

  // ensure render graph is called 
  ws.command_graph->addChild(render_graph);

  if (spec.overlay.size()) {
    cout << "Looking for overlay " << spec.overlay << endl;
    // not yet implemented
  }

}

VSGViewer::VSGViewer() :
  winspec(),
  root(NULL),
  observer(NULL),
  viewer(NULL),
  options(NULL),
  config_dynamic_created(0),
  allow_unknown(false),
  windows(),
  active_objects(),
  static_objects(),
  post_draw(),
  viewspec(),
  resourcepath(),
  keep_pointer(false),
  bg_color(4, 0.0),
  fog_mode(Off),
  fog_density(0.0),
  fog_colour(1.0, 1.0, 1.0, 0.001),
  fog_start(10000.0),
  fog_end(100000.0)
{
  bg_color[3] = 1.0;
  bg_color[2] = 0.45;
  // root is created upon window init
}


VSGViewer::~VSGViewer()
{

}

/** Quick exception struct. */
struct DuecaVSGConfigError: public std::exception
{
  /** Say what is the problem */
  const char* what() { return "Configuration error VSG"; }
};

// to simplify programming
inline VSGViewer::WindowSet
VSGViewer::myCreateWindow(const WinSpec &ws, vsg::ref_ptr<vsg::Group> root,
			  const WindowsMap& windows)
{
  // result, to be returned
  WindowSet res;
  res.display = ws.display;
  res.name = ws.name;
  res.traits = vsg::WindowTraits::create();

  // get screen size
  res.traits->windowTitle = ws.name;

  // do we share a device?
  for (auto const &ow: windows) {
    if (ow.second.display == ws.display) {
      I_MOD("VSG window '" << ws.name << "' shares with '" <<
	    ow.second.name << "'");
      res.traits->shareWindow = ow.second.window;
    }
  }

  // Full screen?
  if (ws.size_and_position.size() == 0) {
    res.traits->fullscreen = true;
  }
  else {
    // Check for position information
    if (ws.size_and_position.size() == 4) {
      res.traits->x = ws.size_and_position[2];
      res.traits->y = ws.size_and_position[3];
    }
    res.traits->width = ws.size_and_position[0];
    res.traits->height = ws.size_and_position[1];
    res.traits->fullscreen = false;
  }
  
  // double buffer
  res.traits->swapchainPreferences.imageCount = 2;
  res.traits->synchronizationLayer = true;
  
  res.window = vsg::Window::create(res.traits);
  res.command_graph = vsg::CommandGraph::create(res.window);

  return res;
}

namespace dueca {
  extern int* p_argc;
  extern char*** p_argv;
}

void VSGViewer::init(bool waitswap)
{
  // process what is in the commandline
  vsg::CommandLine arguments(p_argc, *p_argv);
  
  // create root
  options = vsg::Options::create();
  options->fileCache = vsg::getEnv("VSG_FILE_CACHE");
  options->paths = vsg::getEnvPaths("VSG_FILE_PATH");

  // add vsgXchange reading and writing of 3rd party file formats
  options->add(vsgXchange::all::create());
  arguments.read(options);

  // create viewer
  viewer = vsg::Viewer::create();

  // create scene graph root
  root = vsg::Group::create();

  // and the observer/eye group
  observer = vsg::Group::create();
  std::list<vsg::ref_ptr<vsg::Group> > observer_path;
  observer_path.push_back(observer);

  auto viewmatrix = vsg::TrackingViewMatrix::create(observer_path);

  // If no window specified, give a dummy default specification
  if (winspec.empty()) {
    WinSpec window;
    window.name = "DUECA/VSG default window";
    float size_and_position[] = { 400, 300};
    window.size_and_position.resize(2);
    copy(&size_and_position[0], &size_and_position[2],
         window.size_and_position.begin());
    window.display = "";
    addWindow(window);
  }

  // now create all windows
  while (!winspec.empty()) {
    if (windows.find(winspec.front().name) != windows.end()) {
      cerr << "Already specified a window " << winspec.front().name
           << " ignoring second one" << endl;
    }
    windows[winspec.front().name] = myCreateWindow
      (winspec.front(), root, windows);
    viewer->addWindow(windows[winspec.front().name].window);
    winspec.pop_front();
  }

  // create cameras and viewports
  while (!viewspec.empty()) {
    // find the appropriate window
    WindowsMap::iterator ii = windows.find(viewspec.front().winname);
    if (ii == windows.end()) {
      E_MOD("Could not find window \"" << viewspec.front().winname
            << "\" for view \"" << viewspec.front().name << '"');
    }
    else if (ii->second.viewset.count(viewspec.front().name)) {
      E_MOD("Already have view \"" << viewspec.front().name <<
            "\" in window \"" << viewspec.front().winname << '"');
    }
    else {
      ii->second.viewset[viewspec.front().name] = ViewSet();

      // init view
      ii->second.viewset[viewspec.front().name].init
        (viewspec.front(), ii->second, viewer,
         root, bg_color);
    }
    viewspec.pop_front();
  }

  // if applicable, initialize static objects and dynamic objects
  for (auto &ao: active_objects) { ao.second->init(root, this); }
  for (auto &so: static_objects) { so->init(root, this); }

  // add it all to the viewer
  //vsgUtil::Optimizer optimizer;
  //optimizer.optimize(root);

  // viewer->setSceneData(root);
  // viewer->setThreadingModel(vsg::Viewer::SingleThreaded);
  // viewer->setReleaseContextAtEndOfFrameHint(true);
  CommandGraphs cgs;
  for (auto const &win: windows) {
    cgs.push_back(win.second.command_graph);
  }

  viewer->assignRecordAndSubmitTaskAndPresentation(cgs);
  viewer->compile();
}

void VSGViewer::addViewport(const ViewSpec& vp)
{
  viewspec.push_back(vp);
}

void VSGViewer::redraw(bool wait, bool reset_context)
{
  if (viewer->advanceToNextFrame()) {
    viewer->handleEvents();
    viewer->update();
    viewer->recordAndSubmit();
    viewer->present();
  }
}

void VSGViewer::waitSwap()
{
#if 0
  WindowsMap::const_iterator ii = windows.begin();
  if (ii == windows.end()) {
    // strange, no windows to swap
    return;
  }

  // wait for vsync and swap the first buffer
  ii->second.window->swapBuffers(true); ii++;

  // now quickly do the others, let's hope we are real-time enough
  for ( ; ii != windows.end(); ii++) {
    ii->second.window->swapBuffers(false);
  }
#endif
}

#if 0
template <typename T>
inline static void updateTransform(vsg::Node* tf, const T& v)
{
  vsg::PositionAttitudeTransform* t =
    dynamic_cast<vsg::PositionAttitudeTransform*>(tf);
  if (t == NULL) return;
  if (v.size() >= 3) {
    t->setPosition(AxisTransform::vsgPos(v.data()));
  }
  if (v.size() >= 7) {
    t->setAttitude(AxisTransform::vsgQuat(v.data()+3));
  }
  if (v.size() >= 10) {
    t->setScale(AxisTransform::vsgScale(v.data()+7));
  }
}
#endif

bool VSGViewer::adaptSceneGraph(const WorldViewConfig& adapt)
{
  try {

    switch (adapt.command) {

    case WorldViewConfig::ClearModels: {

      // to be updated, only remove static objects
      for (auto &so: static_objects) {
        so->unInit(root);
      }
      static_objects.clear();
    }
      break;

    case WorldViewConfig::RemoveNode:
      // TODO
      break;

    case WorldViewConfig::LoadObject: {

      // creates and adds a specific configuration
      std::string dclass = "on-the-fly-object_" +
        boost::lexical_cast<std::string>(++config_dynamic_created);
      this->addFactorySpec(dclass, adapt.config);

      // run the createStatic call to create the object
      std::vector<std::string> createconf;
      createconf.push_back(dclass);
      this->createStatic(createconf);
    }
      break;

    case WorldViewConfig::MoveObject: {
      // TODO
      /*for (int ii = root->getNumChildren(); ii--; ) {
        if (root->getChild(ii)->getName() == adapt.config.name) {
          updateTransform(root->getChild(ii), adapt.config.coordinates);
        }
	}*/
    }
      break;
    case WorldViewConfig::ListNodes:
    case WorldViewConfig::LoadOverlay:
    case WorldViewConfig::RemoveOverlay:
    case WorldViewConfig::ReadScene:
      W_MOD("VSGViewer " << adapt.command << " is not implemented");
    }
  }
  catch (const std::exception& e) {
    W_MOD("VSGViewer caught " << e.what());
    return false;
  }
  return true;
}

void VSGViewer::setBase(TimeTickType tick, const BaseObjectMotion& ownm,
                        double late)
{
  // transformation from world origin to the base of the vehicle
  auto world2orig =
    vsg::rotate(AxisTransform::vsgQuatInv(ownm.attitude_q)) *
    vsg::translate(-ownm.xyz[1], -ownm.xyz[0], ownm.xyz[2]);

  // update all cameras, as they are in the viewset list
  for (auto &win: windows) {
    for (auto &view: win.second.viewset) {
      view.second.view_matrix->set(view.second.eye_offset * world2orig);
    }
  }

  // run through all active objects, and inform about the vehicle
  // position & time
  for (auto &obj : active_objects) {
    obj.second->iterate(tick, ownm, late);
  }
}

bool VSGViewer::createControllable
(const GlobalId& master_id, const NameSet& cname, entryid_type entry_id,
 uint32_t creation_id, const std::string& data_class,
 const std::string& entry_label, Channel::EntryTimeAspect time_aspect)
{
  creation_key_t keypair(cname.name, creation_id);

  // check we don't have this one yet
  assert(active_objects.count(keypair) == 0);

  // not found, create entry on the basis of data class and entry label
  VSGObject *op = NULL;
  WorldDataSpec obj;

  try {
    obj = retrieveFactorySpec(data_class, entry_label, creation_id);

    op = VSGObjectFactory::instance().create(obj.type, obj);
    op->connect(master_id, cname, entry_id, time_aspect);
    if (root) {
      op->init(root, this);
      viewer->compile();
    }
    boost::intrusive_ptr<VSGObject> bop(op);
    active_objects[keypair] = bop;
    return true;
  }
  catch (const CFCannotMake& problem) {
    if (!allow_unknown) {
      W_MOD("VSGViewer: factory cannot create for " << data_class <<
            " encountered: " <<  problem.what());
      throw(problem);
    }
    W_MOD("VSGViewer: factory cannot create for " << data_class <<
          ", ignoring channel " << cname << " entry " << entry_id);
  }
  catch (const MapSpecificationError& problem) {
    if (!allow_unknown) {
      W_MOD("VSGViewer: not configured for " << data_class <<
            " encountered: " <<  problem.what());
      throw(problem);
    }
    W_MOD("VSGViewer: not configured for " << data_class <<
          ", ignoring channel " << cname << " entry " << entry_id);
  }
  catch (const std::exception& problem) {
    cerr << "VSGViewer: When trying to create for " << data_class
         << " encountered: " <<  problem.what() << endl;
  }
  return false;
}

bool VSGViewer::createStatic(const std::vector<std::string>& name)
{
  if (!name.size()) {
    E_MOD("Need to specify type of static object");
    return false;
  }

  auto obj = retrieveFactorySpec(name[0], "", static_objects.size(), true);
  if (obj.type.size() == 0) {
    E_MOD("Cannot find object type \"" << name[0] << "\" in the factory");
    return false;
  }

  if (name.size() >= 2) {
    obj.name = name[1];
  }
  return createStatic(obj);
}

bool VSGViewer::createStatic(const WorldDataSpec& obj)
{
  try {
    VSGObject* op =
      VSGObjectFactory::instance().create(obj.type, obj);
    if (root) { op->init(root, this); }
    boost::intrusive_ptr<VSGObject> bop(op);
    static_objects.push_back(bop);
    return true;
  }
  catch (const std::exception& problem) {
    W_MOD("VSGViewer: When trying to create " << obj.type
	  << " encountered: " <<  problem.what());
  }
  return false;
}
void VSGViewer::removeControllable(const NameSet& cname, uint32_t creation_id)
{
  active_objects[std::make_pair(cname.name, creation_id)].reset();
}

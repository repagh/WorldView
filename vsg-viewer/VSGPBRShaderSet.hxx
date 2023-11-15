/* ------------------------------------------------------------------   */
/*      item            : VSGPBRShaderSet.hxx
        made by         : Rene van Paassen
        date            : 230802
        category        : header file
        description     :
        changes         : 230802 first version
        language        : C++
        copyright       : (c) 23 TUDelft-AE-C&S
*/

#pragma once

#include <vsg/utils/ShaderSet.h>
#include <glm/glm.hpp>

namespace vsgviewer {

  /** Object to pass simple fog data to shaders */
  struct FogData {
    /// Color of the fog, RGBA
    glm::vec4 color;

    /// Density, determines depth.
    float density;

    /// Constructor to initialize
    FogData();
  };

  /** load and generate set of shaders

    @param opt   Generic vsg options
    @param data  Pointer to any shared data between shaders and application
  */
  vsg::ref_ptr<vsg::ShaderSet> vsgPBRShaderSet
    (vsg::ref_ptr<const vsg::Options> opt,
     vsg::ref_ptr<vsg::Data> data);
  vsg::ref_ptr<vsg::ShaderSet> vsgFlatShaderSet
    (vsg::ref_ptr<const vsg::Options> opt,
     vsg::ref_ptr<vsg::Data> data);
  vsg::ref_ptr<vsg::ShaderSet> vsgPhongShaderSet
    (vsg::ref_ptr<const vsg::Options> opt,
     vsg::ref_ptr<vsg::Data> data);

} // namespace vsgviewer

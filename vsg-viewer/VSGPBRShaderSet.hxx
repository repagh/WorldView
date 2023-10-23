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

  struct FogData {
    glm::vec4 color;
    float density;
  };

  // load and generate set of shaders
  vsg::ref_ptr<vsg::ShaderSet> vsgPBRShaderSet
  (vsg::ref_ptr<const vsg::Options>, vsg::ref_ptr<vsg::Data>);

} // namespace vsgviewer

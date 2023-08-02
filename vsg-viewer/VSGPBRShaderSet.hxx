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

namespace vsgviewer {

  // load and generate set of shaders
  vsg::ref_ptr<vsg::ShaderSet> vsgPBRShaderSet(vsg::ref_ptr<const vsg::Options>);

} // namespace vsgviewer

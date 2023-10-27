/* ------------------------------------------------------------------   */
/*      item            : VSGPBRShaderSet.cxx
        made by         : Rene' van Paassen
        date            : 230802
        category        : body file
        description     :
        changes         : 230802 first version
        language        : C++
        copyright       : (c) 23 TUDelft-AE-C&S
*/

#include "VSGPBRShaderSet.hxx"
#include <vsg/io/read.h>
#include <dueca/debug.h>
#include <vsg/state/material.h>

namespace vsgviewer {

  FogData::FogData() :
    color(0.0f),
    density(0.0f)
  { }
  
  vsg::ref_ptr<vsg::ShaderSet> vsgPBRShaderSet
  (vsg::ref_ptr<const vsg::Options> options,
   vsg::ref_ptr<vsg::Data> thefog)
  {
    auto pbr_vertexShader = vsg::read_cast<vsg::ShaderStage>
      ("shaders/standard.vert", options);
    auto pbr_fragmentShader = vsg::read_cast<vsg::ShaderStage>
      ("shaders/standard_pbr.frag", options);

    auto pbr = vsg::createPhysicsBasedRenderingShaderSet(options);

    // check these were found
    if (!pbr_vertexShader || !pbr_fragmentShader) {
        E_MOD("Could not load shaders.");
        return {};
    }

    pbr->stages.clear();
    pbr->stages.push_back(pbr_vertexShader);
    pbr->stages.push_back(pbr_fragmentShader);
    pbr->optionalDefines.insert("WORLDVIEW_SIMPLEFOG");
    pbr->defaultShaderHints = vsg::ShaderCompileSettings::create();
    pbr->defaultShaderHints->defines.insert("WORLDVIEW_SIMPLEFOG");

    pbr->variants.clear();


    // create the shader set
    //auto shaderSet = vsg::ShaderSet::create
    //  (vsg::ShaderStages{vertexShader, fragmentShader});

    // bindings?
    pbr->addAttributeBinding
      ("vsg_Vertex", "", 0,
       VK_FORMAT_R32G32B32_SFLOAT, vsg::vec3Array::create(1));
    pbr->addAttributeBinding
      ("vsg_Normal", "", 1,
       VK_FORMAT_R32G32B32_SFLOAT, vsg::vec3Array::create(1));
    pbr->addAttributeBinding
      ("vsg_TexCoord0", "", 2,
       VK_FORMAT_R32G32_SFLOAT, vsg::vec2Array::create(1));
    pbr->addAttributeBinding
      ("vsg_Color", "", 3,
       VK_FORMAT_R32G32B32A32_SFLOAT, vsg::vec4Array::create(1));

    pbr->addAttributeBinding
      ("vsg_position", "VSG_INSTANCE_POSITIONS", 4,
       VK_FORMAT_R32G32B32_SFLOAT, vsg::vec3Array::create(1));
    pbr->addAttributeBinding
      ("vsg_position_scaleDistance", "VSG_BILLBOARD", 4,
       VK_FORMAT_R32G32B32A32_SFLOAT, vsg::vec4Array::create(1));

    pbr->addDescriptorBinding
      ("displacementMap", "VSG_DISPLACEMENT_MAP", 0, 6,
       VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 1,
       VK_SHADER_STAGE_VERTEX_BIT,
       vsg::floatArray2D::create(1, 1, vsg::Data::Properties{VK_FORMAT_R32_SFLOAT}));
    pbr->addDescriptorBinding
      ("diffuseMap", "VSG_DIFFUSE_MAP", 0, 0,
       VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 1,
       VK_SHADER_STAGE_FRAGMENT_BIT,
       vsg::ubvec4Array2D::create(1, 1, vsg::Data::Properties{VK_FORMAT_R8G8B8A8_UNORM}));
    pbr->addDescriptorBinding
      ("mrMap", "VSG_METALLROUGHNESS_MAP", 0, 1,
       VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 1,
       VK_SHADER_STAGE_FRAGMENT_BIT,
       vsg::vec2Array2D::create(1, 1, vsg::Data::Properties{VK_FORMAT_R32G32_SFLOAT}));
    pbr->addDescriptorBinding
      ("normalMap", "VSG_NORMAL_MAP", 0, 2,
       VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 1,
       VK_SHADER_STAGE_FRAGMENT_BIT,
       vsg::vec3Array2D::create(1, 1, vsg::Data::Properties{VK_FORMAT_R32G32B32_SFLOAT}));
    pbr->addDescriptorBinding
      ("aoMap", "VSG_LIGHTMAP_MAP", 0, 3,
       VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 1,
       VK_SHADER_STAGE_FRAGMENT_BIT,
       vsg::floatArray2D::create(1, 1, vsg::Data::Properties{VK_FORMAT_R32_SFLOAT}));
    pbr->addDescriptorBinding
      ("emissiveMap", "VSG_EMISSIVE_MAP", 0, 4,
       VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 1,
       VK_SHADER_STAGE_FRAGMENT_BIT,
       vsg::ubvec4Array2D::create(1, 1, vsg::Data::Properties{VK_FORMAT_R8G8B8A8_UNORM}));
    pbr->addDescriptorBinding
      ("specularMap", "VSG_SPECULAR_MAP", 0, 5,
       VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 1,
       VK_SHADER_STAGE_FRAGMENT_BIT,
       vsg::ubvec4Array2D::create(1, 1, vsg::Data::Properties{VK_FORMAT_R8G8B8A8_UNORM}));
    pbr->addDescriptorBinding
      ("material", "", 0, 10,
      VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 1, VK_SHADER_STAGE_FRAGMENT_BIT,
      vsg::PbrMaterialValue::create());
    pbr->addDescriptorBinding
      ("lightData", "", 1, 0,
      VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 1,
      VK_SHADER_STAGE_FRAGMENT_BIT, vsg::vec4Array::create(64));

    // fog addition
    pbr->addDescriptorBinding
      ("gl_Fog", "WORLDVIEW_SIMPLEFOG", 0, 11,
       VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 1, VK_SHADER_STAGE_FRAGMENT_BIT,
       thefog);

    // additional defines
    pbr->optionalDefines =
      {"VSG_GREYSACLE_DIFFUSE_MAP", "VSG_TWO_SIDED_LIGHTING", "VSG_WORKFLOW_SPECGLOSS"};

    pbr->addPushConstantRange
      ("pc", "", VK_SHADER_STAGE_VERTEX_BIT, 0, 128);

    pbr->definesArrayStates.push_back
      (vsg::DefinesArrayState{{"VSG_INSTANCE_POSITIONS", "VSG_DISPLACEMENT_MAP"},
                                vsg::PositionAndDisplacementMapArrayState::create()});
    pbr->definesArrayStates.push_back
      (vsg::DefinesArrayState{{"VSG_INSTANCE_POSITIONS"},
                                vsg::PositionArrayState::create()});
    pbr->definesArrayStates.push_back
      (vsg::DefinesArrayState{{"VSG_DISPLACEMENT_MAP"},
       vsg::DisplacementMapArrayState::create()});
    pbr->definesArrayStates.push_back
      (vsg::DefinesArrayState{{"VSG_BILLBOARD"},
       vsg::BillboardArrayState::create()});

    pbr->customDescriptorSetBindings.push_back(vsg::ViewDependentStateBinding::create(1));

    return pbr;
  }

} // namespace vsgviewer

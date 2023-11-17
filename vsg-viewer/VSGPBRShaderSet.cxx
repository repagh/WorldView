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
#include <vsg/core/Array.h>
#include <vsg/core/Data.h>
#include <vsg/io/read.h>
#include <dueca/debug.h>
#include <vsg/state/material.h>


#define VIEW_DESCRIPTOR_SET 0
#define MATERIAL_DESCRIPTOR_SET 1

namespace vsgviewer {

  FogData::FogData() :
    color(0.5f),
    density(0.0f)
  { }

  vsg::ref_ptr<vsg::ShaderSet> vsgPBRShaderSet
  (vsg::ref_ptr<const vsg::Options> options,
   vsg::ref_ptr<vsg::Data> fog_data)
  {
    auto pbr_vertexShader = vsg::read_cast<vsg::ShaderStage>
      ("shaders/standard.vert", options);
    auto pbr_fragmentShader = vsg::read_cast<vsg::ShaderStage>
      ("shaders/fog_pbr.frag", options);

    // check these were found
    if (!pbr_vertexShader || !pbr_fragmentShader) {
        E_MOD("Could not load shaders.");
        return {};
    }

    auto pbr = vsg::ShaderSet::create(
      vsg::ShaderStages{pbr_vertexShader, pbr_fragmentShader});


    pbr->optionalDefines.insert("WORLDVIEW_SIMPLEFOG");
    pbr->defaultShaderHints = vsg::ShaderCompileSettings::create();
    pbr->defaultShaderHints->generateDebugInfo = true;
    pbr->defaultShaderHints->defines.insert("WORLDVIEW_SIMPLEFOG");

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
      ("displacementMap", "VSG_DISPLACEMENT_MAP", MATERIAL_DESCRIPTOR_SET, 6,
       VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 1,
       VK_SHADER_STAGE_VERTEX_BIT,
       vsg::floatArray2D::create(1, 1, vsg::Data::Properties{VK_FORMAT_R32_SFLOAT}));
    pbr->addDescriptorBinding
      ("diffuseMap", "VSG_DIFFUSE_MAP", MATERIAL_DESCRIPTOR_SET, 0,
       VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 1,
       VK_SHADER_STAGE_FRAGMENT_BIT,
       vsg::ubvec4Array2D::create(1, 1, vsg::Data::Properties{VK_FORMAT_R8G8B8A8_UNORM}));
    pbr->addDescriptorBinding
      ("mrMap", "VSG_METALLROUGHNESS_MAP", MATERIAL_DESCRIPTOR_SET, 1,
       VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 1,
       VK_SHADER_STAGE_FRAGMENT_BIT,
       vsg::vec2Array2D::create(1, 1, vsg::Data::Properties{VK_FORMAT_R32G32_SFLOAT}));
    pbr->addDescriptorBinding
      ("normalMap", "VSG_NORMAL_MAP", MATERIAL_DESCRIPTOR_SET, 2,
       VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 1,
       VK_SHADER_STAGE_FRAGMENT_BIT,
       vsg::vec3Array2D::create(1, 1, vsg::Data::Properties{VK_FORMAT_R32G32B32_SFLOAT}));
    pbr->addDescriptorBinding
      ("aoMap", "VSG_LIGHTMAP_MAP", MATERIAL_DESCRIPTOR_SET, 3,
       VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 1,
       VK_SHADER_STAGE_FRAGMENT_BIT,
       vsg::floatArray2D::create(1, 1, vsg::Data::Properties{VK_FORMAT_R32_SFLOAT}));
    pbr->addDescriptorBinding
      ("emissiveMap", "VSG_EMISSIVE_MAP", MATERIAL_DESCRIPTOR_SET, 4,
       VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 1,
       VK_SHADER_STAGE_FRAGMENT_BIT,
       vsg::ubvec4Array2D::create(1, 1, vsg::Data::Properties{VK_FORMAT_R8G8B8A8_UNORM}));
    pbr->addDescriptorBinding
      ("specularMap", "VSG_SPECULAR_MAP", MATERIAL_DESCRIPTOR_SET, 5,
       VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 1,
       VK_SHADER_STAGE_FRAGMENT_BIT,
       vsg::ubvec4Array2D::create(1, 1, vsg::Data::Properties{VK_FORMAT_R8G8B8A8_UNORM}));
    pbr->addDescriptorBinding
      ("material", "", MATERIAL_DESCRIPTOR_SET, 10,
      VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 1, VK_SHADER_STAGE_FRAGMENT_BIT,
      vsg::PbrMaterialValue::create());
    pbr->addDescriptorBinding
      ("lightData", "", VIEW_DESCRIPTOR_SET, 0,
      VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 1,
      VK_SHADER_STAGE_FRAGMENT_BIT, vsg::vec4Array::create(64));
    pbr->addDescriptorBinding("viewportData", "", VIEW_DESCRIPTOR_SET,
      1, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 1,
      VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT,
      vsg::vec4Value::create(0,0, 1280, 1024));
    pbr->addDescriptorBinding("shadowMaps", "", VIEW_DESCRIPTOR_SET, 2,
      VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 1, VK_SHADER_STAGE_FRAGMENT_BIT,
      vsg::floatArray3D::create(1, 1, 1, vsg::Data::Properties{VK_FORMAT_R32_SFLOAT}));

#if 1
    pbr->addDescriptorBinding
      ("gl_Fog", "WORLDVIEW_SIMPLEFOG", VIEW_DESCRIPTOR_SET, 11,
       VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 1, VK_SHADER_STAGE_FRAGMENT_BIT,
       fog_data);
#endif

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

vsg::ref_ptr<vsg::ShaderSet> vsgFlatShaderSet
  (vsg::ref_ptr<const vsg::Options> options,
   vsg::ref_ptr<vsg::Data> fog_data)
  {
    auto shaderHints = vsg::ShaderCompileSettings::create();


    auto flat_vertexShader = vsg::read_cast<vsg::ShaderStage>
      ("shaders/standard.vert", options);
    auto flat_fragmentShader = vsg::read_cast<vsg::ShaderStage>
      ("shaders/fog_flat_shaded.frag", options);

    // check these were found
    if (!flat_vertexShader || !flat_fragmentShader) {
        E_MOD("Could not load shaders.");
        return {};
    }

    auto flat = vsg::ShaderSet::create(
      vsg::ShaderStages{flat_vertexShader, flat_fragmentShader});

    flat->defaultShaderHints = vsg::ShaderCompileSettings::create();
    flat->defaultShaderHints->generateDebugInfo = true;
    flat->optionalDefines.insert("WORLDVIEW_SIMPLEFOG");
    flat->defaultShaderHints->defines.insert("WORLDVIEW_SIMPLEFOG");

    // bindings?
    flat->addAttributeBinding
      ("vsg_Vertex", "", 0,
       VK_FORMAT_R32G32B32_SFLOAT, vsg::vec3Array::create(1));
    flat->addAttributeBinding
      ("vsg_Normal", "", 1,
       VK_FORMAT_R32G32B32_SFLOAT, vsg::vec3Array::create(1));
    flat->addAttributeBinding
      ("vsg_TexCoord0", "", 2,
       VK_FORMAT_R32G32_SFLOAT, vsg::vec2Array::create(1));
    flat->addAttributeBinding
      ("vsg_Color", "", 3,
       VK_FORMAT_R32G32B32A32_SFLOAT, vsg::vec4Array::create(1));

    flat->addAttributeBinding
      ("vsg_position", "VSG_INSTANCE_POSITIONS", 4,
       VK_FORMAT_R32G32B32_SFLOAT, vsg::vec3Array::create(1));
    flat->addAttributeBinding
      ("vsg_position_scaleDistance", "VSG_BILLBOARD", 4,
       VK_FORMAT_R32G32B32A32_SFLOAT, vsg::vec4Array::create(1));

    flat->addDescriptorBinding
      ("displacementMap", "VSG_DISPLACEMENT_MAP", MATERIAL_DESCRIPTOR_SET, 6,
       VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 1,
       VK_SHADER_STAGE_VERTEX_BIT,
       vsg::floatArray2D::create(1, 1, vsg::Data::Properties{VK_FORMAT_R32_SFLOAT}));
    flat->addDescriptorBinding
      ("diffuseMap", "VSG_DIFFUSE_MAP", MATERIAL_DESCRIPTOR_SET, 0,
       VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 1,
       VK_SHADER_STAGE_FRAGMENT_BIT,
       vsg::ubvec4Array2D::create(1, 1, vsg::Data::Properties{VK_FORMAT_R8G8B8A8_UNORM}));

    flat->addDescriptorBinding
      ("material", "", MATERIAL_DESCRIPTOR_SET, 10,
      VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 1, VK_SHADER_STAGE_FRAGMENT_BIT,
      vsg::PhongMaterialValue::create());
    flat->addDescriptorBinding
      ("lightData", "", VIEW_DESCRIPTOR_SET, 0,
      VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 1,
      VK_SHADER_STAGE_FRAGMENT_BIT, vsg::vec4Array::create(64));
    flat->addDescriptorBinding("viewportData", "", VIEW_DESCRIPTOR_SET,
      1, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 1,
      VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT,
      vsg::vec4Value::create(0,0, 1280, 1024));
    flat->addDescriptorBinding("shadowMaps", "", VIEW_DESCRIPTOR_SET, 2,
      VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 1, VK_SHADER_STAGE_FRAGMENT_BIT,
      vsg::floatArray3D::create(1, 1, 1, vsg::Data::Properties{VK_FORMAT_R32_SFLOAT}));

    // fog addition
    flat->addDescriptorBinding
      ("gl_Fog", "WORLDVIEW_SIMPLEFOG", VIEW_DESCRIPTOR_SET, 11,
       VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 1, VK_SHADER_STAGE_FRAGMENT_BIT,
       fog_data);

    // additional defines
    flat->optionalDefines =
      {"VSG_POINT_SPRITE", "VSG_GREYSACLE_DIFFUSE_MAP"};

    flat->addPushConstantRange
      ("pc", "", VK_SHADER_STAGE_VERTEX_BIT, 0, 128);

    flat->definesArrayStates.push_back
      (vsg::DefinesArrayState{{"VSG_INSTANCE_POSITIONS", "VSG_DISPLACEMENT_MAP"},
                                vsg::PositionAndDisplacementMapArrayState::create()});
    flat->definesArrayStates.push_back
      (vsg::DefinesArrayState{{"VSG_INSTANCE_POSITIONS"},
                                vsg::PositionArrayState::create()});
    flat->definesArrayStates.push_back
      (vsg::DefinesArrayState{{"VSG_DISPLACEMENT_MAP"},
       vsg::DisplacementMapArrayState::create()});
    flat->definesArrayStates.push_back
      (vsg::DefinesArrayState{{"VSG_BILLBOARD"},
       vsg::BillboardArrayState::create()});

    return flat;
  }

vsg::ref_ptr<vsg::ShaderSet> vsgPhongShaderSet
  (vsg::ref_ptr<const vsg::Options> options,
   vsg::ref_ptr<vsg::Data> fog_data)
  {
    auto shaderHints = vsg::ShaderCompileSettings::create();


    auto phong_vertexShader = vsg::read_cast<vsg::ShaderStage>
      ("shaders/standard.vert", options);
    auto phong_fragmentShader = vsg::read_cast<vsg::ShaderStage>
      ("shaders/fog_phong.frag", options);

    // check these were found
    if (!phong_vertexShader || !phong_fragmentShader) {
        E_MOD("Could not load shaders.");
        return {};
    }

    auto phong = vsg::ShaderSet::create(
      vsg::ShaderStages{phong_vertexShader, phong_fragmentShader});

    phong->defaultShaderHints = vsg::ShaderCompileSettings::create();
    phong->defaultShaderHints->generateDebugInfo = true;
    phong->optionalDefines.insert("WORLDVIEW_SIMPLEFOG");
    phong->defaultShaderHints->defines.insert("WORLDVIEW_SIMPLEFOG");

    // bindings?
    phong->addAttributeBinding
      ("vsg_Vertex", "", 0,
       VK_FORMAT_R32G32B32_SFLOAT, vsg::vec3Array::create(1));
    phong->addAttributeBinding
      ("vsg_Normal", "", 1,
       VK_FORMAT_R32G32B32_SFLOAT, vsg::vec3Array::create(1));
    phong->addAttributeBinding
      ("vsg_TexCoord0", "", 2,
       VK_FORMAT_R32G32_SFLOAT, vsg::vec2Array::create(1));
    phong->addAttributeBinding
      ("vsg_Color", "", 3,
       VK_FORMAT_R32G32B32A32_SFLOAT, vsg::vec4Array::create(1));

    phong->addAttributeBinding
      ("vsg_position", "VSG_INSTANCE_POSITIONS", 4,
       VK_FORMAT_R32G32B32_SFLOAT, vsg::vec3Array::create(1));
    phong->addAttributeBinding
      ("vsg_position_scaleDistance", "VSG_BILLBOARD", 4,
       VK_FORMAT_R32G32B32A32_SFLOAT, vsg::vec4Array::create(1));

    phong->addDescriptorBinding
      ("displacementMap", "VSG_DISPLACEMENT_MAP", MATERIAL_DESCRIPTOR_SET, 6,
       VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 1,
       VK_SHADER_STAGE_VERTEX_BIT,
       vsg::floatArray2D::create(1, 1, vsg::Data::Properties{VK_FORMAT_R32_SFLOAT}));
    phong->addDescriptorBinding
      ("diffuseMap", "VSG_DIFFUSE_MAP", MATERIAL_DESCRIPTOR_SET, 0,
       VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 1,
       VK_SHADER_STAGE_FRAGMENT_BIT,
       vsg::ubvec4Array2D::create(1, 1, vsg::Data::Properties{VK_FORMAT_R8G8B8A8_UNORM}));
    phong->addDescriptorBinding("normalMap", "VSG_NORMAL_MAP",
      MATERIAL_DESCRIPTOR_SET, 2, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER,
      1, VK_SHADER_STAGE_FRAGMENT_BIT,
      vsg::vec3Array2D::create(1, 1, vsg::Data::Properties{VK_FORMAT_R32G32B32_SFLOAT}));
    phong->addDescriptorBinding("aoMap", "VSG_LIGHTMAP_MAP",
      MATERIAL_DESCRIPTOR_SET, 3, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 1, VK_SHADER_STAGE_FRAGMENT_BIT, vsg::vec4Array2D::create(1, 1, vsg::Data::Properties{VK_FORMAT_R32_SFLOAT}));
    phong->addDescriptorBinding("emissiveMap", "VSG_EMISSIVE_MAP",
      MATERIAL_DESCRIPTOR_SET, 4, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 1, VK_SHADER_STAGE_FRAGMENT_BIT, vsg::vec4Array2D::create(1, 1, vsg::Data::Properties{VK_FORMAT_R8G8B8A8_UNORM}));

    phong->addDescriptorBinding
      ("material", "", MATERIAL_DESCRIPTOR_SET, 10,
      VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 1, VK_SHADER_STAGE_FRAGMENT_BIT,
      vsg::PhongMaterialValue::create());
    phong->addDescriptorBinding
      ("lightData", "", VIEW_DESCRIPTOR_SET, 0,
      VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 1,
      VK_SHADER_STAGE_FRAGMENT_BIT, vsg::vec4Array::create(64));
    phong->addDescriptorBinding("viewportData", "", VIEW_DESCRIPTOR_SET,
      1, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 1,
      VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT,
      vsg::vec4Value::create(0,0, 1280, 1024));
    phong->addDescriptorBinding("shadowMaps", "", VIEW_DESCRIPTOR_SET, 2,
      VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 1, VK_SHADER_STAGE_FRAGMENT_BIT,
      vsg::floatArray3D::create(1, 1, 1, vsg::Data::Properties{VK_FORMAT_R32_SFLOAT}));

    // fog addition
    phong->addDescriptorBinding
      ("gl_Fog", "WORLDVIEW_SIMPLEFOG", VIEW_DESCRIPTOR_SET, 11,
       VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 1, VK_SHADER_STAGE_FRAGMENT_BIT,
       fog_data);

    // additional defines
    phong->optionalDefines =
      {"VSG_POINT_SPRITE", "VSG_TWO_SIDED_LIGHTING", "VSG_GREYSCALE_DIFFUSE_MAP"};

    phong->addPushConstantRange
      ("pc", "", VK_SHADER_STAGE_VERTEX_BIT, 0, 128);

    phong->definesArrayStates.push_back
      (vsg::DefinesArrayState{{"VSG_INSTANCE_POSITIONS", "VSG_DISPLACEMENT_MAP"},
                                vsg::PositionAndDisplacementMapArrayState::create()});
    phong->definesArrayStates.push_back
      (vsg::DefinesArrayState{{"VSG_INSTANCE_POSITIONS"},
                                vsg::PositionArrayState::create()});
    phong->definesArrayStates.push_back
      (vsg::DefinesArrayState{{"VSG_DISPLACEMENT_MAP"},
       vsg::DisplacementMapArrayState::create()});
    phong->definesArrayStates.push_back
      (vsg::DefinesArrayState{{"VSG_BILLBOARD"},
       vsg::BillboardArrayState::create()});
    phong->customDescriptorSetBindings.push_back(vsg::ViewDependentStateBinding::create(VIEW_DESCRIPTOR_SET));

    return phong;
  }


} // namespace vsgviewer

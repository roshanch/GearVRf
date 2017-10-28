/* Copyright 2015 Samsung Electronics Co., LTD
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

/***************************************************************************
 * Renders a scene, a screen.
 ***************************************************************************/

#include <vulkan/vulkan_index_buffer.h>
#include <vulkan/vulkan_vertex_buffer.h>
#include <vulkan/vk_cubemap_image.h>
#include <vulkan/vk_render_to_texture.h>
#include <vulkan/vk_render_target.h>
#include "renderer.h"
#include "glm/gtc/matrix_inverse.hpp"

#include "objects/scene.h"
#include "vulkan/vulkan_shader.h"
#include "vulkan_renderer.h"
#include "vulkan/vulkan_material.h"
#include "vulkan/vulkan_render_data.h"
#include "vulkan/vk_texture.h"
#include "vulkan/vk_bitmap_image.h"

namespace gvr {
ShaderData* VulkanRenderer::createMaterial(const char* uniform_desc, const char* texture_desc)
{
    return new VulkanMaterial(uniform_desc, texture_desc);
}
RenderTexture* VulkanRenderer::createRenderTexture(const RenderTextureInfo& renderTextureInfo) {
    return new VkRenderTexture(renderTextureInfo.fdboWidth, renderTextureInfo.fboHeight, 1);
}
    RenderData* VulkanRenderer::createRenderData()
{
    return new VulkanRenderData();
}
RenderTarget* VulkanRenderer::createRenderTarget(Scene* scene) {
    return new VkRenderTarget(scene);
}
RenderTarget* VulkanRenderer::createRenderTarget(RenderTexture* renderTexture, bool isMultiview){
    return new VkRenderTarget(renderTexture, isMultiview);
}
RenderTarget* VulkanRenderer::createRenderTarget(RenderTexture* renderTexture, const RenderTarget* renderTarget){
    return new VkRenderTarget(renderTexture, renderTarget);
}

RenderPass* VulkanRenderer::createRenderPass(){
    return new VulkanRenderPass();
}

UniformBlock* VulkanRenderer::createUniformBlock(const char* desc, int binding, const char* name, int maxelems)
{
   if (maxelems <= 1)
   {
       return new VulkanUniformBlock(desc, binding, name);
   }

    return new VulkanUniformBlock(desc, binding, name, maxelems);
}

Image* VulkanRenderer::createImage(int type, int format)
{
    switch (type)
    {
        case Image::ImageType::BITMAP: return new VkBitmapImage(format);
        case Image::ImageType::CUBEMAP: return new VkCubemapImage(format);
            //    case Image::ImageType::FLOAT_BITMAP: return new GLFloatImage();
    }
    return NULL;
}

Texture* VulkanRenderer::createTexture(int target)
{
    // TODO: where to send the target
    return new VkTexture(static_cast<int>(VK_IMAGE_TYPE_2D));
}

RenderTexture* VulkanRenderer::createRenderTexture(int width, int height, int sample_count,
                                                   int jcolor_format, int jdepth_format, bool resolve_depth,
                                                   const TextureParameters* texture_parameters, int number_views)
{
    return new VkRenderTexture(width, height, 1);
}

Shader* VulkanRenderer::createShader(int id, const char* signature,
                                     const char* uniformDescriptor, const char* textureDescriptor,
                                     const char* vertexDescriptor, const char* vertexShader,
                                     const char* fragmentShader)
{
    return new VulkanShader(id, signature, uniformDescriptor, textureDescriptor, vertexDescriptor, vertexShader, fragmentShader);
}

VertexBuffer* VulkanRenderer::createVertexBuffer(const char* desc, int vcount)
{
    return new VulkanVertexBuffer(desc, vcount);
}

IndexBuffer* VulkanRenderer::createIndexBuffer(int bytesPerIndex, int icount)
{
    return new VulkanIndexBuffer(bytesPerIndex, icount);
}

bool VulkanRenderer::renderWithShader(RenderState& rstate, Shader* shader, RenderData* rdata, ShaderData* shaderData,  int pass)
{

    VulkanRenderData* vkRdata = static_cast<VulkanRenderData*>(rdata);
    UniformBlock& transformUBO = vkRdata->getTransformUbo();
    VulkanMaterial* vkmtl = static_cast<VulkanMaterial*>(shaderData);

    if (shader->usesMatrixUniforms())
    {
        updateTransforms(rstate, &transformUBO, rdata);
    }
    rdata->updateGPU(this,shader);

    vulkanCore_->InitLayoutRenderData(*vkmtl, vkRdata, shader);

    if(vkRdata->isDirty(pass)) {
        vulkanCore_->InitDescriptorSetForRenderData(this, pass, shader, vkRdata);

        VkRenderPass render_pass = vulkanCore_->createVkRenderPass(NORMAL_RENDERPASS,1);
        std::string vkPipelineHashCode = vkRdata->getHashCode() + to_string(shader->getShaderID()) + rdata->mesh()->getVertexBuffer()->getDescriptor() ;

        VkPipeline pipeline = vulkanCore_->getPipeline(vkPipelineHashCode);
        if(pipeline == 0) {
            vkRdata->createPipeline(shader, this, pass, render_pass);
            vulkanCore_->addPipeline(vkPipelineHashCode, vkRdata->getVKPipeline(pass));
        }
        else{
            vkRdata->setPipeline(pipeline, pass);
            vkRdata->clearDirty();
        }
    }
    return true;
}


void VulkanRenderer::updatePostEffectMesh(Mesh* copy_mesh)
{
    float positions[] = { -1.0f, 1.0f,  1.0f,
                          -1.0f, -1.0f,  1.0f,
                          1.0f,  -1.0f,  1.0f,
                          1.0f,  1.0f,  1.0f,
                          -1.0f, 1.0f,  1.0f,
                          1.0f,  -1.0f,  1.0f};

    float uvs[] = { 0.0f, 1.0f,
                    0.0f, 0.0f,
                    1.0f, 0.0f,
                    1.0f, 1.0f,
                    0.0f, 1.0f,
                    1.0f, 0.0f};

    const int position_size = sizeof(positions)/ sizeof(positions[0]);
    const int uv_size = sizeof(uvs)/ sizeof(uvs[0]);

    copy_mesh->setVertices(positions, position_size);
    copy_mesh->setFloatVec("a_texcoord", uvs, uv_size);

}
void VulkanRenderer::renderRenderDataVector(RenderState& rstate,std::vector<RenderData*>& render_data_vector, std::vector<RenderData*>& render_data_list){
    for (auto rdata = render_data_vector.begin();
         rdata != render_data_vector.end();
         ++rdata)
    {
        if (!(rstate.render_mask & (*rdata)->render_mask()))
            continue;

        for(int curr_pass = 0; curr_pass < (*rdata)->pass_count(); curr_pass++) {
            ShaderData *curr_material = (*rdata)->material(curr_pass);
            Shader *shader = rstate.shader_manager->getShader((*rdata)->get_shader(rstate.is_multiview,curr_pass));
            if (shader == NULL)
            {
                LOGE("SHADER: shader not found");
                continue;
            }
            if (!renderWithShader(rstate, shader, (*rdata), curr_material, curr_pass))
                break;

            if(curr_pass == (*rdata)->pass_count()-1)
                render_data_list.push_back((*rdata));
        }
    }
}
void VulkanRenderer::renderRenderTarget(Scene* scene, RenderTarget* renderTarget, ShaderManager* shader_manager,
                                RenderTexture* post_effect_render_texture_a, RenderTexture* post_effect_render_texture_b){


    std::vector<RenderData*> render_data_list;
    Camera* camera = renderTarget->getCamera();
    RenderState rstate = renderTarget->getRenderState();
    RenderData* post_effects = camera->post_effect_data();
    rstate.scene = scene;
    rstate.shader_manager = shader_manager;
    rstate.uniforms.u_view = camera->getViewMatrix();
    rstate.uniforms.u_proj = camera->getProjectionMatrix();


    std::vector<RenderData*>* render_data_vector = renderTarget->getRenderDataVector();
    int postEffectCount = 0;

    if (!rstate.shadow_map) {
        rstate.render_mask = camera->render_mask();
        rstate.uniforms.u_right = rstate.render_mask & RenderData::RenderMaskBit::Right;
        rstate.material_override = NULL;
    }
    renderRenderDataVector(rstate,*render_data_vector,render_data_list);
    VkRenderTarget *vk_renderTarget = reinterpret_cast<VkRenderTarget *>(renderTarget);

    if ((post_effects != NULL) &&
        (post_effect_render_texture_a != nullptr) &&
        (post_effects->pass_count() >= 0)) {

        if (post_effects->isValid(this, rstate) < 0)
        {
            LOGE("Renderer::renderPostEffectData not ready");
            return;             // no shader available
        }
        vulkanCore_->InitPostEffectChain();
        VkRenderTexture* renderTexture = reinterpret_cast<VkRenderTexture*>(post_effect_render_texture_a);
        VkRenderTexture* input_texture = renderTexture;
        vulkanCore_->BuildCmdBufferForRenderData(render_data_list, camera, shader_manager,
                                                 nullptr, renderTexture, false);
        vulkanCore_->submitCmdBuffer(vulkanCore_->getPostEffectFence(), *vulkanCore_->getCurrentCmdBufferPE());
        vulkanCore_->waitForFence(vulkanCore_->getPostEffectFence());

        postEffectCount = post_effects->pass_count();
        // Call Post Effect
        for (int i = 0; i < postEffectCount-1; i++) {

            if (i % 2 == 0)
            {
                renderTexture = static_cast<VkRenderTexture*>(post_effect_render_texture_b);
            }
            else
            {
                renderTexture = static_cast<VkRenderTexture*>(post_effect_render_texture_a);
            }

            if(!renderPostEffectData(rstate,input_texture,post_effects,i))
                return;

            VkCommandBuffer cmdbuffer = *vulkanCore_->getCurrentCmdBufferPE();
            vulkanCore_->BuildCmdBufferForRenderDataPE(cmdbuffer, rstate.shader_manager,camera, post_effects, renderTexture, i);
            vulkanCore_->submitCmdBuffer(vulkanCore_->getPostEffectFence(),cmdbuffer);
            vulkanCore_->waitForFence(vulkanCore_->getPostEffectFence());
            input_texture = renderTexture;
        }
        render_data_list.clear();
        render_data_list.push_back(post_effects);

        if(!renderPostEffectData(rstate,input_texture,post_effects,postEffectCount - 1))
            return;

        vulkanCore_->BuildCmdBufferForRenderData(render_data_list, camera, shader_manager, renderTarget, nullptr, true);
        vulkanCore_->submitCmdBuffer(reinterpret_cast<VkRenderTexture *>(renderTarget->getTexture())->getFenceObject(),
                vk_renderTarget->getCommandBuffer());
    }
    else {
        vulkanCore_->BuildCmdBufferForRenderData(render_data_list, camera, shader_manager,
                                                 renderTarget, nullptr, false);
        vulkanCore_->submitCmdBuffer(
                reinterpret_cast<VkRenderTexture *>(renderTarget->getTexture())->getFenceObject(),
                vk_renderTarget->getCommandBuffer());
    }
}


}
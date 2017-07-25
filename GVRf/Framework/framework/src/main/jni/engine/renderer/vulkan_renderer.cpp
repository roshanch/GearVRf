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
#include "renderer.h"


#include "objects/scene.h"
#include "objects/textures/render_texture.h"
#include "vulkan/vulkan_shader.h"
#include "vulkan_renderer.h"
#include "vulkan/vulkan_material.h"
#include "vulkan/vulkan_render_data.h"
#include "vulkan/vk_texture.h"
#include "vulkan/vk_bitmap_image.h"
#include "vulkan/vk_render_to_texture.h"

namespace gvr {
    ShaderData* VulkanRenderer::createMaterial(const char* uniform_desc, const char* texture_desc)
    {
        return new VulkanMaterial(uniform_desc, texture_desc);
    }

    RenderData* VulkanRenderer::createRenderData()
    {
        return new VulkanRenderData();
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
        return new VulkanUniformBlock(desc, binding, name);
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
                                                 const TextureParameters* texture_parameters)
    {
        return NULL;
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
        Transform* const t = rdata->owner_object()->transform();

        int status = shaderData->updateGPU(this);
        if (status < 0)
        {
            LOGE("SHADER: textures not ready %s", rdata->owner_object()->name().c_str());
            return false;
        }

        VulkanRenderData* vkRdata = static_cast<VulkanRenderData*>(rdata);
        UniformBlock& transformUBO = vkRdata->getTransformUbo();
        VulkanMaterial* vkmtl = static_cast<VulkanMaterial*>(shaderData);

        if (shader->usesMatrixUniforms())
        {
            updateTransforms(rstate, &transformUBO, t);
        }
        rdata->updateGPU(this,shader);

        vulkanCore_->InitLayoutRenderData(*vkmtl, vkRdata, shader);

        if(vkRdata->isHashCodeDirty() || vkRdata->isDirty(0xFFFF) || vkRdata->isDescriptorSetNull(pass)) {

            vulkanCore_->InitDescriptorSetForRenderData(this, pass, shader, vkRdata);
            vkRdata->createPipeline(shader, this, pass);
        }
        return true;
    }
    void VulkanRenderer::renderMesh(RenderState& rstate, RenderData* render_data){
        for(int curr_pass =0; curr_pass< render_data->pass_count(); curr_pass++) {

            ShaderData *curr_material = render_data->material(curr_pass);
            Shader *shader = rstate.shader_manager->getShader(render_data->get_shader(curr_pass));
            if (shader == NULL)
            {
                LOGE("SHADER: shader not found");
                continue;
            }
            if (rstate.material_override != nullptr) {
                curr_material = rstate.material_override;
            }
            if (!renderWithShader(rstate, shader, render_data, curr_material, curr_pass))
                break;

            if(curr_pass == render_data->pass_count()-1)
                mRenderDataList.push_back(render_data);
        }
    }

// TODO : put vulkan equivalent code here
    void VulkanRenderer::cullAndRender(RenderTarget* renderTarget, Scene* scene,
                                   ShaderManager* shader_manager,
                                   PostEffectShaderManager* post_effect_shader_manager,
                                   RenderTexture* post_effect_render_texture_a,
                                   RenderTexture* post_effect_render_texture_b)
    {
        RenderState& rstate = renderTarget->getRenderState();
        Camera* camera = renderTarget->getCamera();
        const std::vector<ShaderData*>& post_effects = camera->post_effect_data();
        RenderTexture* saveRenderTexture = renderTarget->getTexture();
        mRenderDataList.clear();
        cullFromCamera(scene, camera, shader_manager);
        rstate.shader_manager = shader_manager;
        rstate.scene = scene;
        if (!rstate.shadow_map)
        {
            state_sort();
            GL(glEnable (GL_BLEND));
            GL(glBlendEquation (GL_FUNC_ADD));
            GL(glBlendFunc(GL_ONE, GL_ONE_MINUS_SRC_ALPHA));
        }

        if ((post_effects.size() == 0) ||
            (post_effect_render_texture_a == nullptr))
        {
            saveRenderTexture->useStencil(useStencilBuffer_);
            for (auto it = render_data_vector.begin();
                 it != render_data_vector.end();
                 ++it)
            {
                RenderData* rdata = *it;
                if (!rstate.shadow_map || rdata->cast_shadows())
                {
                    GL(renderRenderData(rstate, rdata));
                }
            }
            VkCommandBuffer cmdBuffer;
            vulkanCore_->createTransientCmdBuffer(cmdBuffer);
            vulkanCore_->beginCommandBuffer(&cmdBuffer);

            renderTarget->beginRendering(this);
            vulkanCore_->BuildCmdBufferForRenderData(mRenderDataList, camera, shader_manager, cmdBuffer);
            renderTarget->endRendering(this);

            vulkanCore_->endCommandBuffer(&cmdBuffer);
            VkFence fence = vulkanCore_->getFenceObject();
            vkResetFences(getDevice(), 1, &fence);

            vulkanCore_->submitCmdBuffer(&cmdBuffer, fence);
            vkWaitForFences(getDevice(), 1, &fence, VK_TRUE, 4294967295U);
        }
        else
        {
            RenderTexture* renderTexture = post_effect_render_texture_a;

            renderTexture->useStencil(useStencilBuffer_);
            renderTarget->setTexture(renderTexture);
            renderTarget->beginRendering(this);
            for (auto it = render_data_vector.begin();
                 it != render_data_vector.end();
                 ++it)
            {
                RenderData* rdata = *it;
                if (!rstate.shadow_map || rdata->cast_shadows())
                {
                    GL(renderRenderData(rstate, rdata));
                }
            }
            GL(glDisable(GL_DEPTH_TEST));
            GL(glDisable(GL_CULL_FACE));
            renderTarget->endRendering(this);
            for (int i = 0; i < post_effects.size() - 1; ++i)
            {
                if (i % 2 == 0)
                {
                    renderTexture = post_effect_render_texture_a;
                }
                else
                {
                    renderTexture = post_effect_render_texture_b;
                }
                renderTarget->setTexture(renderTexture);
                renderTarget->beginRendering(this);
                GL(renderPostEffectData(rstate, renderTexture, post_effects[i]));
                renderTarget->endRendering(this);
            }
            renderTarget->setTexture(saveRenderTexture);
            renderTarget->beginRendering(this);
            GL(renderPostEffectData(rstate, renderTexture, post_effects.back()));
            renderTarget->endRendering(this);
        }
        GL(glDisable(GL_DEPTH_TEST));
        GL(glDisable(GL_CULL_FACE));
        GL(glDisable(GL_BLEND));
    }
    void VulkanRenderer::renderCamera(Scene *scene, Camera *camera,
                                      ShaderManager *shader_manager,
                                      PostEffectShaderManager *post_effect_shader_manager,
                                      RenderTexture *post_effect_render_texture_a,
                                      RenderTexture *post_effect_render_texture_b) {


        if(!vulkanCore_->swapChainCreated())
            vulkanCore_->initVulkanCore();

        vulkanCore_->AcquireNextImage();
        RenderState rstate;
        rstate.shadow_map = false;
        rstate.material_override = NULL;
        rstate.shader_manager = shader_manager;
        rstate.scene = scene;
        rstate.render_mask = camera->render_mask();
        rstate.uniforms.u_right = rstate.render_mask & RenderData::RenderMaskBit::Right;
        rstate.uniforms.u_view = camera->getViewMatrix();
        rstate.uniforms.u_proj = camera->getProjectionMatrix();
        mRenderDataList.clear();

        renderRenderDataVector(rstate);

        VkCommandBuffer* cmdBuffer = vulkanCore_->getCurrentCmdBuffer();
        vulkanCore_->beginCommandBuffer(cmdBuffer);
        VkRenderTexture* renderTexture = vulkanCore_->getCurrentRenderTexture();
        renderTexture->setBackgroundColor(camera->background_color_r(), camera->background_color_g(),camera->background_color_b(), camera->background_color_a());
        renderTexture->beginRendering(Renderer::getInstance());

        vulkanCore_->BuildCmdBufferForRenderData(mRenderDataList, camera, shader_manager, *cmdBuffer);
        renderTexture->endRendering(Renderer::getInstance());
        vulkanCore_->endCommandBuffer(cmdBuffer);

        vulkanCore_->DrawFrameForRenderData();
    }


}
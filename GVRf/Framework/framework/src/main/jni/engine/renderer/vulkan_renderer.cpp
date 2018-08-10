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
#include <vulkan/vk_light.h>
#include "renderer.h"
#include "main_sorter.h"
#include "glm/gtc/matrix_inverse.hpp"

#include "objects/scene.h"
#include "vulkan/vulkan_shader.h"
#include "vulkan_renderer.h"
#include "vulkan/vulkan_material.h"
#include "vulkan/vulkan_render_data.h"
#include "vulkan/vk_texture.h"
#include "vulkan/vk_bitmap_image.h"
//#include <bits/stdc++.h>
#include <glslang/Include/Common.h>
#define VERTEX_BUFFER_BIND_ID 0
namespace gvr {
ShaderData* VulkanRenderer::createMaterial(const char* uniform_desc, const char* texture_desc)
{
    return new VulkanMaterial(uniform_desc, texture_desc);
}

RenderTexture* VulkanRenderer::createRenderTexture(const RenderTextureInfo& renderTextureInfo) {
    return new VkRenderTexture(renderTextureInfo.fboWidth, renderTextureInfo.fboHeight, DEPTH_IMAGE | COLOR_IMAGE, 1, renderTextureInfo.multisamples);
}

RenderTexture* VulkanRenderer::createRenderTexture(int width, int height, int sample_count, int layers, int jdepth_format) {
    return new VkRenderTexture(width, height, DEPTH_IMAGE | COLOR_IMAGE, layers, sample_count);
}

Light* VulkanRenderer::createLight(const char* uniformDescriptor, const char* textureDescriptor)
{
    return new VKLight(uniformDescriptor, textureDescriptor);
}

RenderData* VulkanRenderer::createRenderData()
{
    return new VulkanRenderData();
}

RenderData* VulkanRenderer::createRenderData(RenderData* data)
{
    return new VulkanRenderData(*data);
}

UniformBlock* VulkanRenderer::createTransformBlock(int numMatrices)
{
    return VulkanRenderer::createUniformBlock("mat4 u_matrices", TRANSFORM_UBO_INDEX, "Transform_ubo", numMatrices);
}

RenderTarget* VulkanRenderer::createRenderTarget(Scene* scene, bool stereo)
{
    VkRenderTarget* renderTarget = new VkRenderTarget(scene, stereo);
    RenderSorter* sorter = new MainSceneSorter(*this, 0, true);
    static_cast<MainSceneSorter*>(sorter)->setSortOptions({ MainSceneSorter::SortOption::RENDER_ORDER, MainSceneSorter::SortOption::DISTANCE,
                                                            MainSceneSorter::SortOption::PIPELINE, MainSceneSorter::SortOption::MESH });
    renderTarget->setRenderSorter(sorter);
    return renderTarget;
}

RenderTarget* VulkanRenderer::createRenderTarget(RenderTexture* renderTexture, bool isMultiview, bool isStereo)
{
    VkRenderTarget* renderTarget = new VkRenderTarget(renderTexture, isMultiview, isStereo);
    RenderSorter* sorter = new MainSceneSorter(*this, 0, true);
    static_cast<MainSceneSorter*>(sorter)->setSortOptions({ MainSceneSorter::SortOption::RENDER_ORDER, MainSceneSorter::SortOption::DISTANCE,
                                                            MainSceneSorter::SortOption::PIPELINE, MainSceneSorter::SortOption::MESH });
    renderTarget->setRenderSorter(sorter);
    return renderTarget;
}

RenderTarget* VulkanRenderer::createRenderTarget(RenderTexture* renderTexture, const RenderTarget* renderTarget)
{
    VkRenderTarget* vkTarget = new VkRenderTarget(renderTexture, renderTarget);
    vkTarget->setRenderSorter(renderTarget->getRenderSorter());
    return vkTarget;
}

RenderPass* VulkanRenderer::createRenderPass()
{
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
//<<<<<<< HEAD
    return new VkRenderTexture(width, height, DEPTH_IMAGE | COLOR_IMAGE, 1, sample_count);
//=======
//    if (monoscopic)
//        return new VkRenderTextureOnScreen(width, height, sample_count);
//    return new VkRenderTextureOffScreen(width, height, sample_count);
//>>>>>>> bba42476... validate function for vulkan
}

Shader* VulkanRenderer::createShader(int id, const char* signature,
                                     const char* uniformDescriptor, const char* textureDescriptor,
                                     const char* vertexDescriptor, const char* vertexShader,
                                     const char* fragmentShader, const char* matrixCalc)
{
    Shader* shader = new VulkanShader(id, signature, uniformDescriptor, textureDescriptor, vertexDescriptor, vertexShader, fragmentShader, matrixCalc);
    int shader_id = shader->getShaderID();
    PipelineHashing& pipelineHashing = vulkanCore_->getPipelineHash();
    shader->setShaderIndex(pipelineHashing.getShaderIndex(shader_id));
    return shader;
}

VertexBuffer* VulkanRenderer::createVertexBuffer(const char* desc, int vcount)
{
    VertexBuffer* vertexBuffer = new VulkanVertexBuffer(desc, vcount);
    std::string descriptor = vertexBuffer->getTinyDescriptor();
    PipelineHashing& pipelineHashing = vulkanCore_->getPipelineHash();
    int index = pipelineHashing.addDescriptor(descriptor);
    vertexBuffer->setDescriptorIndex(index);
    return vertexBuffer;
}

IndexBuffer* VulkanRenderer::createIndexBuffer(int bytesPerIndex, int icount)
{
    return new VulkanIndexBuffer(bytesPerIndex, icount);
}

void VulkanRenderer::updatePostEffectMesh(Mesh* copy_mesh)
{
    float positions[] = { -1.0f, +1.0f, 1.0f,
                          +1.0f, -1.0f, 1.0f,
                          -1.0f, -1.0f, 1.0f,

                          +1.0f, +1.0f, 1.0f,
                          +1.0f, -1.0f, 1.0f,
                          -1.0f, +1.0f, 1.0f,
    };

    float uvs[] = { 0.0f, 1.0f,
                    1.0f, 0.0f,
                    0.0f, 0.0f,

                    1.0f, 1.0f,
                    1.0f, 0.0f,
                    0.0f, 1.0f,
    };

    const int position_size = sizeof(positions)/ sizeof(positions[0]);
    const int uv_size = sizeof(uvs)/ sizeof(uvs[0]);

    copy_mesh->setVertices(positions, position_size);
    copy_mesh->setFloatVec("a_texcoord", uvs, uv_size);
}
//<<<<<<< HEAD
//=======
inline bool isLayoutNeeded(RenderSorter::Renderable& r, LightList& lights) {
    const DataDescriptor &textureDescriptor = r.shader->getTextureDescriptor();
    DataDescriptor &uniformDescriptor = r.shader->getUniformDescriptor();
    bool transformUboPresent = r.shader->usesMatrixUniforms();
    VulkanMaterial *vkmtl = static_cast<VulkanMaterial *>(r.renderPass->material());

    if (textureDescriptor.getNumEntries() == 0 && uniformDescriptor.getNumEntries() == 0 &&
        !transformUboPresent && !lights.getLightCount())
        return false;

    return true;
}
 //todo : check matdata dirty
inline bool isDescriptorSetDirty(RenderSorter::Renderable& r){
    return r.renderModes.isDirty() || r.material->isDirty(ShaderData::DIRTY_BITS::NEW_TEXTURE | ShaderData::DIRTY_BITS::MAT_DATA)  ||
            static_cast<VulkanRenderPass*>(r.renderPass)->mDescriptorSets.size() == 0;
}
void VulkanRenderer::createVkResources(RenderSorter::Renderable& r, RenderState& rstate){

    VulkanRenderData* vkRdata = static_cast<VulkanRenderData*>(r.renderData);

    vkRdata->updateGPU(this, r.shader);
    LightList& lights = rstate.scene->getLights();
    bool need_layouts = isLayoutNeeded(r, lights);
    if(r.shader->isShaderDirty() && need_layouts)
        vulkanCore_->InitLayoutRenderData(r, lights,rstate);


    //todo: better logic for pipeline creation check
    bool createPipeline = false;
    if(isDescriptorSetDirty(r)) {
       if (isLayoutNeeded(r, lights))
           vulkanCore_->InitDescriptorSetForRenderData(r, lights,rstate);
       createPipeline = true;
    }

    vulkanCore_->updateTransformDescriptors(r);

    if(createPipeline) {
        VkRenderPass render_pass = vulkanCore_->createVkRenderPass(NORMAL_RENDERPASS,
                                                                   rstate.sampleCount);
        PipelineHashing& pipelineHashing = vulkanCore_->getPipelineHash();

       //todo: check for pipeline derivatives
        pipelineHashing.createPipeline(this,r,rstate,render_pass);
        VulkanRenderPass* vk_renderPass = static_cast<VulkanRenderPass*>(r.renderPass);
        vk_renderPass->render_modes().clearDirty();
    }
}
void VulkanRenderer::validate(RenderSorter::Renderable& r, RenderState& rstate)
{
    r.material->updateGPU(this);
    createVkResources(r,rstate);
}



void VulkanRenderer::render(const RenderState& rstate, const RenderSorter::Renderable& r)
{
    VulkanRenderPass* vkRenderPass = static_cast<VulkanRenderPass*>(r.renderPass);
    VkPipeline item_pipeline = vkRenderPass->m_pipeline;
    VkPipeline curr_pipeline = mCurrentState.renderPass ? static_cast<VulkanRenderPass*>(r.renderPass) ->m_pipeline : VK_NULL_HANDLE;
    VkCommandBuffer cmdBuffer = mCurrentCmdBuffer;

    if(item_pipeline != curr_pipeline){
        vkCmdBindPipeline(cmdBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS,
                          item_pipeline);
        mCurrentState.renderPass = r.renderPass;
    }

    VulkanShader *Vkshader = reinterpret_cast<VulkanShader *>(r.shader);

    //bind out descriptor set, which handles our uniforms and samplers
    if (vkRenderPass->mDescriptorSets.size())
    {
        udata u;
        u.u_proj_offset = rstate.camera->getProjectionMatrix()[0][0] * CameraRig::default_camera_separation_distance();
        u.u_matrix_offset = r.matrixOffset; u.u_right = rstate.u_right; u.u_render_mask = rstate.u_render_mask;
                                                 vkCmdPushConstants (cmdBuffer, Vkshader->getPipelineLayout(), VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT,
                            0, sizeof(udata), &u);

        vkCmdBindDescriptorSets(cmdBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS,
                                Vkshader->getPipelineLayout(), 0, 2,
                                vkRenderPass->mDescriptorSets.data(), 0, NULL);
    }
    const VulkanIndexBuffer *ibuf = reinterpret_cast<const VulkanIndexBuffer *>(r.mesh->getIndexBuffer());

    //if(mCurrentState.mesh != r.mesh) {
    mCurrentState.mesh = r.mesh;
    // Bind our vertex buffer, with a 0 offset.
    VkDeviceSize offsets[1] = {0};
    VulkanVertexBuffer *vbuf = reinterpret_cast< VulkanVertexBuffer *>(r.mesh->getVertexBuffer());
    const GVR_VK_Vertices *vert = (vbuf->getVKVertices(r.shader));

    vkCmdBindVertexBuffers(cmdBuffer, VERTEX_BUFFER_BIND_ID, 1, &(vert->buf), offsets);

    if (ibuf && ibuf->getIndexCount()) {
        const GVR_VK_Indices &ind = ibuf->getVKIndices();
        VkIndexType indexType = (ibuf->getIndexSize() == 2) ? VK_INDEX_TYPE_UINT16
                                                            : VK_INDEX_TYPE_UINT32;
        vkCmdBindIndexBuffer(cmdBuffer, ind.buffer, 0, indexType);
    }

    //}
    if (ibuf && ibuf->getIndexCount())
        vkCmdDrawIndexed(cmdBuffer, ibuf->getVKIndices().count, 1, 0, 0, 1);
    else
       vkCmdDraw(cmdBuffer, r.mesh->getVertexCount(), 1, 0, 1);

}

void VulkanRenderer::renderRenderTarget(Scene* scene, jobject javaSceneObject, RenderTarget* renderTarget, ShaderManager* shader_manager,
                                RenderTexture* post_effect_render_texture_a, RenderTexture* post_effect_render_texture_b){
#if 0
//>>>>>>> 9a1c2d25... cleanup
    std::vector<RenderData*> render_data_list;
    Camera* camera = renderTarget->getCamera();
    RenderState rstate = renderTarget->getRenderState();
    RenderData* post_effects = camera->post_effect_data();
    rstate.scene = scene;
    rstate.shader_manager = shader_manager;
    rstate.u_matrices[VIEW] = camera->getViewMatrix();
    rstate.u_matrices[PROJECTION] = camera->getProjectionMatrix();
    rstate.javaSceneObject = javaSceneObject;

    if(vulkanCore_->isSwapChainPresent())
        rstate.u_matrices[PROJECTION] = glm::mat4(1,0,0,0,  0,-1,0,0, 0,0,0.5,0, 0,0,0.5,1) * rstate.u_matrices[PROJECTION];
    int postEffectCount = 0;

    if (!rstate.is_shadow)
    {
        rstate.u_render_mask = camera->render_mask();
        rstate.u_right = rstate.u_render_mask & RenderData::RenderMaskBit::Right;
    }

    renderTarget->beginRendering();
    renderTarget->render();
    renderTarget->endRendering();
    VkRenderTarget *vk_renderTarget = static_cast<VkRenderTarget *>(renderTarget);

    if ((post_effects != NULL) &&
        (post_effect_render_texture_a != nullptr) &&
        (post_effects->pass_count() >= 0))
    {

        VkRenderTexture* renderTexture = static_cast<VkRenderTexture*>(post_effect_render_texture_a);
        VkRenderTexture* input_texture = renderTexture;
        vulkanCore_->BuildCmdBufferForRenderData(render_data_list, camera, shader_manager,
                                                 nullptr, renderTexture, false, rstate.is_shadow);

        vulkanCore_->submitCmdBuffer(renderTexture->getFenceObject(), renderTexture->getCommandBuffer());
        vulkanCore_->waitForFence(renderTexture->getFenceObject());

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

            if (!renderPostEffectData(rstate,input_texture,post_effects,i))
                return;

            VkCommandBuffer cmdbuffer = renderTexture->getCommandBuffer();
            vulkanCore_->BuildCmdBufferForRenderDataPE(cmdbuffer, rstate.shader_manager,camera, post_effects, renderTexture, i);
            vulkanCore_->submitCmdBuffer(renderTexture->getFenceObject(),cmdbuffer);
            vulkanCore_->waitForFence(renderTexture->getFenceObject());
            input_texture = renderTexture;
        }
        if (!renderPostEffectData(rstate, input_texture, post_effects, postEffectCount - 1))
            return;
        vulkanCore_->BuildCmdBufferForRenderData(render_data_list, camera, shader_manager, renderTarget, nullptr, true, rstate.is_shadow);
        vulkanCore_->submitCmdBuffer(
                static_cast<VkRenderTexture *>(renderTarget->getTexture())->getFenceObject(),
                vk_renderTarget->getCommandBuffer());
    }
    else
    {
        vulkanCore_->BuildCmdBufferForRenderData(render_data_list, camera, shader_manager,
                                                 renderTarget, nullptr, false, rstate.is_shadow);
        vulkanCore_->submitCmdBuffer(
                static_cast<VkRenderTexture *>(renderTarget->getTexture())->getFenceObject(),
                vk_renderTarget->getCommandBuffer());

        // Wait for shadowmap to be rendered
        if(rstate.is_shadow){
            int success = 0;
            while(success != 1){
                success = vulkanCore_->waitForFence(static_cast<VkRenderTexture *>(renderTarget->getTexture())->getFenceObject());
            }
        }

    }

    // Presenting image to swapchain
    if(vulkanCore_->isSwapChainPresent()) {
        vulkanCore_->waitForFence(static_cast<VkRenderTexture *>(renderTarget->getTexture())->getFenceObject());
        vulkanCore_->PresentBackBuffer();
    }
#endif
    mCurrentState.reset();
    renderTarget = static_cast<VkRenderTarget *> (renderTarget);


    //todo: bug here
    RenderState& rstate = renderTarget->getRenderState();
    Camera* camera = rstate.camera;

    rstate.javaSceneObject = javaSceneObject;
    rstate.scene = scene;
    rstate.shader_manager = shader_manager;
    if (rstate.is_multiview)
    {
        rstate.u_render_mask = RenderData::RenderMaskBit::Right | RenderData::RenderMaskBit::Left;
        rstate.u_right = 1;
    }
    else
    {
        rstate.u_render_mask = camera->render_mask();
        rstate.u_right = 0;
        if (((rstate.u_render_mask & RenderData::RenderMaskBit::Right) != 0) && rstate.is_stereo)
        {
            rstate.u_right = 1;
        }
    }

    int postEffectCount = 0;
    RenderData* post_effects = camera->post_effect_data();

    if ((post_effects != NULL) &&
        (post_effect_render_texture_a != nullptr) &&
        (post_effects->pass_count() > 0)) {

        VkResult err;
        //render everything on post_effect_render_texture_a
        VkRenderTexture* renderTexture = static_cast<VkRenderTexture*>(post_effect_render_texture_a);
        VkRenderTexture* input_texture = renderTexture;

        renderTexture->setBackgroundColor(camera->background_color_r(), camera->background_color_g(),
                                          camera->background_color_b(), camera->background_color_a());
        renderTexture->useStencil(this->useStencilBuffer_);
        renderTexture->beginRendering(this);
        setCurrentCommandBuffer(renderTexture->getCommandBuffer());
        renderTarget->render();
        renderTexture->endRendering(this);

        vulkanCore_->submitCmdBuffer( renderTexture->getFenceObject(), getCurrentCommandBuffer());
        err = vkWaitForFences(vulkanCore_->getDevice(), 1, &(renderTexture->getFenceObject()), VK_TRUE, 4294967295U);
        GVR_VK_CHECK(!err);

        postEffectCount = post_effects->pass_count();

        for (int i = 0; i < postEffectCount-1; i++) {

            if (i % 2 == 0)
            {
                renderTexture = static_cast<VkRenderTexture*>(post_effect_render_texture_b);
            }
            else
            {
                renderTexture = static_cast<VkRenderTexture*>(post_effect_render_texture_a);
            }

            renderTexture->beginRendering(this);
            setCurrentCommandBuffer(renderTexture->getCommandBuffer());
            if(!renderPostEffectData(rstate,input_texture,post_effects,i))
                return;
            renderTexture->endRendering(this);

            vulkanCore_->submitCmdBuffer( renderTexture->getFenceObject(), getCurrentCommandBuffer());
            err = vkWaitForFences(vulkanCore_->getDevice(), 1, &(renderTexture->getFenceObject()), VK_TRUE, 4294967295U);
            GVR_VK_CHECK(!err);

            input_texture = renderTexture;

        }

        renderTarget->beginRendering();
        setCurrentCommandBuffer(static_cast<VkRenderTexture *>(renderTarget->getTexture())->getCommandBuffer());
        if(!renderPostEffectData(rstate, input_texture, post_effects, postEffectCount - 1))
            return;
        renderTarget->endRendering();

        vulkanCore_->submitCmdBuffer(
                static_cast<VkRenderTexture *>(renderTarget->getTexture())->getFenceObject(),
                getCurrentCommandBuffer());

        VkFence  fence = static_cast<VkRenderTexture *>(renderTarget->getTexture())->getFenceObject();
        err = vkWaitForFences(vulkanCore_->getDevice(), 1, &fence, VK_TRUE, 4294967295U);
        GVR_VK_CHECK(!err);


    } else {

        renderTarget->beginRendering();
        setCurrentCommandBuffer(static_cast<VkRenderTexture *>(renderTarget->getTexture())->getCommandBuffer());
        renderTarget->render();
        renderTarget->endRendering();

        vulkanCore_->submitCmdBuffer(
                static_cast<VkRenderTexture *>(renderTarget->getTexture())->getFenceObject(),
                getCurrentCommandBuffer());

        VkFence fence = static_cast<VkRenderTexture *>(renderTarget->getTexture())->getFenceObject();
        VkResult err;
        err = vkWaitForFences(vulkanCore_->getDevice(), 1, &fence, VK_TRUE, 4294967295U);
        GVR_VK_CHECK(!err);
    }

    rstate.javaSceneObject = nullptr;
}

    /**
     * Generate shadow maps for all the lights that cast shadows.
     * The scene is rendered from the viewpoint of the light using a
     * special depth shader (GVRDepthShader) to create the shadow map.
     * @see Renderer::renderShadowMap Light::makeShadowMap
     */
    void VulkanRenderer::makeShadowMaps(Scene* scene, jobject javaSceneObject, ShaderManager* shader_manager)
    {
        scene->getLights().makeShadowMaps(scene, javaSceneObject, shader_manager);
    }

}
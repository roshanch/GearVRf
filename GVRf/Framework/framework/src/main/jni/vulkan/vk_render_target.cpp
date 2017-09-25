//
// Created by roshan on 9/22/17.
//
#include "../engine/renderer/vulkan_renderer.h"
#include "../vulkan/vk_render_target.h"
#include "../vulkan/vk_render_to_texture.h"


namespace gvr{
 void VkRenderTarget::beginRendering(Renderer* renderer){

 }
 void VkRenderTarget::endRendering(Renderer* renderer){

 }
VkRenderTarget::VkRenderTarget(RenderTexture* renderTexture, bool is_multiview): RenderTarget(renderTexture, is_multiview){
    initVkData();
}

void VkRenderTarget::createFenceObject(VkDevice device){
    VkResult ret = VK_SUCCESS;
    ret = vkCreateFence(device, gvr::FenceCreateInfo(), nullptr, &mWaitFence);
    GVR_VK_CHECK(!ret);
}
void VkRenderTarget::createCmdBuffer(VkDevice device, VkCommandPool commandPool){
    VkResult ret = VK_SUCCESS;
    ret = vkAllocateCommandBuffers(device, gvr::CmdBufferCreateInfo(VK_COMMAND_BUFFER_LEVEL_PRIMARY, commandPool),
                                   &mCmdBuffer
    );
    GVR_VK_CHECK(!ret);
}
void VkRenderTarget::initVkData() {
    VulkanRenderer* renderer = reinterpret_cast<VulkanRenderer*>(Renderer::getInstance());
    VkDevice device = renderer->getDevice();
    VkCommandPool commandPool = renderer->getCore()->getCommandPool();
    createCmdBuffer(device,commandPool);
    createFenceObject(device);
    mPostEffectTexture = new VkRenderTexture(mRenderTexture->width(), mRenderTexture->height());
}
VkRenderTarget::VkRenderTarget(Scene* scene): RenderTarget(scene){
    initVkData();
}
VkRenderTarget::VkRenderTarget(RenderTexture* renderTexture, const RenderTarget* source): RenderTarget(renderTexture, source){
    initVkData();
}
}

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


#ifndef FRAMEWORK_VULKANCORE_H
#define FRAMEWORK_VULKANCORE_H

#include "util/gvr_log.h"
#include <android/native_window_jni.h>	// for native window JNI
#include <string>
#include <unordered_map>
#include "objects/components/camera.h"
#include "vk_texture.h"
#include "vulkan_flags.h"
//<<<<<<< HEAD
//=======
#include "engine/renderer/render_sorter.h"
//>>>>>>> 68dd0213... validate all vulkan resources

#define GVR_VK_CHECK(X) if (!(X)) { FAIL("VK_CHECK Failure"); }
#define GVR_VK_VERTEX_BUFFER_BIND_ID 0
#define GVR_VK_SAMPLE_NAME "GVR Vulkan"
#define VK_KHR_ANDROID_SURFACE_EXTENSION_NAME "VK_KHR_android_surface"

namespace gvr {
class VulkanUniformBlock;

extern  void setImageLayout(VkImageMemoryBarrier imageMemoryBarrier, VkCommandBuffer cmdBuffer, VkImage image, VkImageAspectFlags aspectMask, VkImageLayout oldImageLayout, VkImageLayout newImageLayout, VkImageSubresourceRange subresourceRange,
                            VkPipelineStageFlags srcStageFlags = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT, VkPipelineStageFlags destStageFlags = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT);
enum ShaderType{
    VERTEX_SHADER,
    FRAGMENT_SHADER
};
enum RenderPassType{
    // Shadow RenderPass will have index from 1 to 16
    // Whereas Normal Renderpass will have index 16 + sampleCount
    SHADOW_RENDERPASS = 0, NORMAL_RENDERPASS = 16
};
extern std::vector<uint64_t> samplers;
extern VkSampler getSampler(uint64_t index);
VkDescriptorSetLayoutBinding createLayoutBinding(int binding_index, int stageFlags, bool sampler = false);
extern VkSampleCountFlagBits getVKSampleBit(int sampleCount);
extern VkRenderPass* createVkRenderPass(RenderPassType);


class Scene;
class ShaderManager;
class RenderData;
class VulkanRenderData;
class Camera;
class VulkanData;
class VulkanMaterial;
class VulkanRenderer;
class UniformBlock;
class Shader;
class VKFramebuffer;
extern uint8_t *oculusTexData;
class VkRenderTexture;
class VulkanShader;
class VkRenderTarget;
class RenderTarget;
class LightList;
//<<<<<<< HEAD
class VKDeviceComponent;
//=======
class PipelineHashing{
public:
    int addDescriptor(const std::string& desc);
    int getShaderIndex(int shader_id);
    VkPipeline createPipeline(VulkanRenderer* renderer, RenderSorter::Renderable& r, RenderState& rstate, VkRenderPass renderPass);
//>>>>>>> 4ee0e2c6... pipeline hashing for vulkan

private:
    std::vector<int> shader_ids;
    std::vector<std::string> mesh_descriptors;
    std::vector<int> hash_keys;
    struct renderModePipelineHash{
        renderModePipelineHash(RenderModes render_mode, VkPipeline pipeline,
                               int next_index): mRenderMode(render_mode), mPipeline(pipeline), mNextindex(next_index){}
        RenderModes mRenderMode;
        VkPipeline  mPipeline;
        int         mNextindex;
    };
    std::vector<renderModePipelineHash> pipelineHash={};
};
class VulkanCore final {

public:
    // Return NULL if Vulkan inititialisation failed. NULL denotes no Vulkan support for this device.
    static VulkanCore *getInstance(ANativeWindow *newNativeWindow = nullptr, int vulkanPropValue = 0) {
        if (!theInstance) {

            theInstance = new VulkanCore(newNativeWindow, vulkanPropValue);
            theInstance->initVulkanCore();
        }
        if (theInstance->m_Vulkan_Initialised)
            return theInstance;
        return NULL;
    }


    //check if Vulkan has been initialised.
    static bool isInstancePresent(){
        if(theInstance == NULL || !theInstance->m_Vulkan_Initialised)
            return false;
        else
            return true;
    }


    void releaseInstance(){
        if(theInstance) {
            delete theInstance;
            theInstance = nullptr;
        }
    }

    void recreateSwapChain(ANativeWindow *);

    ~VulkanCore();

//<<<<<<< HEAD
//    void InitLayoutRenderData(VulkanMaterial * vkMtl, VulkanRenderData* vkdata, Shader*, LightList& lights);
//
//    void initCmdBuffer(VkCommandBufferLevel level,VkCommandBuffer& cmdBuffer);
//
//    bool InitDescriptorSetForRenderData(VulkanRenderer* renderer, int pass, Shader*, VulkanRenderData* vkData, LightList *lights, VulkanMaterial* vkmtl);
//=======
    void InitLayoutRenderData(RenderSorter::Renderable& r, LightList& lights, RenderState& rstate);

    void initCmdBuffer(VkCommandBufferLevel level,VkCommandBuffer& cmdBuffer);

    bool InitDescriptorSetForRenderData(RenderSorter::Renderable&r, LightList& lights, RenderState& rstate);

    void updateTransformDescriptors(RenderSorter::Renderable&r);
//>>>>>>> 68dd0213... validate all vulkan resources
    void beginCmdBuffer(VkCommandBuffer cmdBuffer);
    void BuildCmdBufferForRenderData(std::vector<RenderData *> &render_data_vector, Camera*, ShaderManager*,RenderTarget*,VkRenderTexture*, bool, bool);
    void BuildCmdBufferForRenderDataPE(VkCommandBuffer &cmdBuffer, ShaderManager*, Camera*, RenderData* rdata, VkRenderTexture*, int);

    int waitForFence(VkFence fence);

    VkFence createFenceObject();
    VkCommandBuffer createCommandBuffer(VkCommandBufferLevel level);
    void InitPipelineForRenderData(RenderSorter::Renderable&r,RenderState& rstate, VkRenderPass renderpass);
    void submitCmdBuffer(VkFence fence, VkCommandBuffer cmdBuffer);

    bool GetMemoryTypeFromProperties(uint32_t typeBits, VkFlags requirements_mask,
                                     uint32_t *typeIndex);

    VkDevice &getDevice() {
        return m_device;
    }

    VkPhysicalDevice& getPhysicalDevice(){
        return m_physicalDevice;
    }

    VkQueue &getVkQueue() {
        return m_queue;
    }

    void createTransientCmdBuffer(VkCommandBuffer&);

    VkCommandPool &getTransientCmdPool() {
        return m_commandPoolTrans;
    }

    void initVulkanCore();

    VkRenderPass createVkRenderPass(RenderPassType render_pass_type, int sample_count = 1);

    VkPipeline getPipeline(const std::string& key){
        auto it = pipelineHashMap.find(key);
        if(it != pipelineHashMap.end())
            return it->second;

        return 0;
    }

    void addPipeline(std::string key, VkPipeline pipeline){
        pipelineHashMap[key] = pipeline;
    }
    void InitCommandPools();
    VkCommandPool getCommandPool(){
        return m_commandPool;
    }

    void renderToOculus(RenderTarget* renderTarget);
    void unmapRenderToOculus(RenderTarget* renderTarget);
    void InitSwapChain();

    VkImage getSwapChainImage(){
        return mSwapchainBuffers[swapChainImageIndex].image;
    }

    VkImageView getSwapChainView(){
        return mSwapchainBuffers[swapChainImageIndex++].view;
    }

    bool isSwapChainCreationFinished()
    {
        return swapChainImageIndex == mSwapchainImageCount;
    }

    bool isSwapChainPresent(){
        return swapChainFlag;
    }
    int getSwapChainIndexToRender(){
        return mSwapchainCurrentIdx;
    }
    void SetNextBackBuffer();
    void PresentBackBuffer();
//<<<<<<< HEAD

    void addDeviceComponent(VKDeviceComponent*);
    void removeDeviceComponent(VKDeviceComponent *);


//=======
     PipelineHashing& getPipelineHash(){
        return mPipelineHash;
    }
//>>>>>>> 4ee0e2c6... pipeline hashing for vulkan
private:
    PipelineHashing mPipelineHash;

    static VulkanCore *theInstance;
    std::unordered_map<std::string, VkPipeline> pipelineHashMap;

    explicit VulkanCore(ANativeWindow *newNativeWindow,  int vulkanPropValue = 0) : m_pPhysicalDevices(NULL){
        m_Vulkan_Initialised = false;
        validationLayers = (vulkanPropValue == 2);
        initVulkanDevice(newNativeWindow);
    }

    bool CreateInstance();
    bool GetPhysicalDevices();


    void initVulkanDevice(ANativeWindow *newNativeWindow);

    bool InitDevice();

    void InitSurface();

    void InitSync();

    void createPipelineCache();

    bool m_Vulkan_Initialised;

    std::vector <uint32_t> CompileShader(const std::string &shaderName,
                                         ShaderType shaderTypeID,
                                         const std::string &shaderContents);
    void InitShaders(VkPipelineShaderStageCreateInfo shaderStages[],
                     std::vector<uint32_t>& result_vert, std::vector<uint32_t>& result_frag);

    void GetDescriptorPool(VkDescriptorPool& descriptorPool);
    VkCullModeFlagBits getVulkanCullFace(int);

    ANativeWindow *m_androidWindow;
    VkInstance m_instance;
    VkPhysicalDevice *m_pPhysicalDevices;
    VkPhysicalDevice m_physicalDevice;
    VkPhysicalDeviceProperties m_physicalDeviceProperties;
    VkPhysicalDeviceMemoryProperties m_physicalDeviceMemoryProperties;
    VkDevice m_device;
    uint32_t m_physicalDeviceCount;
    uint32_t m_queueFamilyIndex;
    VkQueue m_queue;

    VkSurfaceKHR m_surface = VK_NULL_HANDLE;
    VkSurfaceFormatKHR mSurfaceFormat;
    VkSwapchainKHR mSwapchain;
    struct SwapchainBuffer
    {
        VkImage image;
        VkImageView view;
    };

    int swapChainImageIndex = 0;
    SwapchainBuffer* mSwapchainBuffers;
    bool swapChainFlag = false;
    // Vulkan Synchronization objects
    VkSemaphore mBackBufferSemaphore;
    VkSemaphore mRenderCompleteSemaphore;

    uint32_t mSwapchainCurrentIdx = 0;
    uint32_t mSwapchainImageCount;

    VkCommandPool m_commandPool;
    VkCommandPool m_commandPoolTrans;

    VkPipelineCache m_pipelineCache;
    std::unordered_map<int, VkRenderPass> mRenderPassMap;

    std::vector<VKDeviceComponent * > mDeviceComponents;

        bool validationLayers = true;
            std::vector<const char*> getInstanceLayers();
            std::vector<const char*> getInstanceExtensions();
            void CreateValidationCallbacks();
            PFN_vkCreateDebugReportCallbackEXT  mCreateDebugReportCallbackEXT;
            PFN_vkDestroyDebugReportCallbackEXT mDestroyDebugReportCallbackEXT;
            VkDebugReportCallbackEXT            mDebugReportCallback;
};
}
#endif //FRAMEWORK_VULKANCORE_H
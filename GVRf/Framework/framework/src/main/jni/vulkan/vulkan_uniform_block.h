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

#ifndef VULKAN_UNIFORMBLOCK_H_
#define VULKAN_UNIFORMBLOCK_H_

#include "objects/uniform_block.h"

namespace gvr {
    class VulkanUniformBlock;
    class VulkanCore;
    /**
     * Manages a Uniform Block containing data parameters to pass to
     * Vulkan vertex and fragment shaders.
     *
     * The UniformBlock may be updated by the application. If it has changed,
     * GearVRf resends the entire data block to Vulkan.
     */
    class VulkanUniformBlock : public UniformBlock
    {
        int getPaddingSize(short &totaSize, int padSize);
        void uboPadding();
    public:
        void createBuffer(Renderer* renderer);
        explicit VulkanUniformBlock(const char* descriptor, int bindingPoint,const char* blockName);
        explicit VulkanUniformBlock(const char* descriptor, int bindingPoint,const char* blockName, int maxelems);
        virtual ~VulkanUniformBlock() {}
        bool bindBuffer(Shader*, Renderer*, int locationOffset = 0) { return true; }
        virtual bool updateGPU(Renderer*, int start = 0, int len = 0);
        virtual std::string makeShaderLayout();
        void createDescriptorWriteInfo(int binding_index,int stageFlags, bool sampler=false);
        GVR_Uniform& getBuffer() { return m_bufferInfo; }

        GVR_Uniform m_bufferInfo;
        const VkWriteDescriptorSet& getWriteDescriptorSet();
        void setDescriptorSet(VkDescriptorSet descriptorSet){
            writeDescriptorSet.dstSet = descriptorSet;
        }
        VkDescriptorSetLayoutBinding getLayoutBinding(){
            return  layout_binding;
        }
        VkDescriptorSet getDescriptorSet(){
            return writeDescriptorSet.dstSet;
        }
        void setDescriptorLayout(VkDescriptorSetLayout descriptorSetLayout){
            this->descriptorSetLayout = descriptorSetLayout;
        }
        VkDescriptorSetLayout getDescriptorSetLayout(){
            return descriptorSetLayout;
        }
        char * getUniformData() { return mUniformData; }
        virtual bool setFloatVec(const char *name, const float *val, int n);
        virtual bool setIntVec(const char *name, const int *val, int n);
    protected:

        void updateBuffer(VulkanCore* vk, int start, int len);
        VkWriteDescriptorSet writeDescriptorSet = {};
        VkDescriptorSetLayout descriptorSetLayout = 0;
        VkDescriptorSetLayoutBinding layout_binding;
    };
}
#endif

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
 * A shader which an user can add in run-time.
 ***************************************************************************/

#include "custom_shader.h"
#include "engine/renderer/renderer.h"
#include "gl/gl_program.h"
#include "objects/material.h"
#include "objects/scene.h"
#include "objects/mesh.h"
#include "objects/textures/texture.h"
#include "objects/components/render_data.h"
#include "util/gvr_gl.h"

#include <sys/time.h>

GLubyte* material_transform_data =nullptr;
#define BATCH_SIZE 60
namespace gvr {
CustomShader::CustomShader(const std::string& vertex_shader, const std::string& fragment_shader)
    : vertexShader_(vertex_shader), fragmentShader_(fragment_shader) {

}
void CustomShader::initializeOnDemand(RenderState* rstate) {
    if (nullptr == program_)
    {
        size_t size = sizeof(glm::vec4)*4 + sizeof(float) + sizeof(glm::mat4) * 3 + sizeof(glm::mat4) * (do_batching ? BATCH_SIZE : 1);
        material_transform_data= (GLubyte*)malloc(size);

        program_ = new GLProgram(vertexShader_.c_str(), fragmentShader_.c_str());
        if(rstate->use_multiview && !(strstr(vertexShader_.c_str(),"gl_ViewID_OVR")
                && strstr(vertexShader_.c_str(),"GL_OVR_multiview2")
                && strstr(vertexShader_.c_str(),"GL_OVR_multiview2"))){
            std::string error = "Your shaders are not multiview";
            LOGE("Your shaders are not multiview");
            throw error;
        }

        u_right_ = glGetUniformLocation(program_->id(), "u_right");

        vertexShader_.clear();
        fragmentShader_.clear();
        LOGE("Custom shader added program %d", program_->id());
    }
   if (textureVariablesDirty_) {
        std::lock_guard<std::mutex> lock(textureVariablesLock_);
        for (auto it = textureVariables_.begin(); it != textureVariables_.end(); ++it) {
            if (-1 == it->location) {
                it->location = it->variableType.f_getLocation(program_->id());
                LOGV("CustomShader::texture:location: variable: %s location: %d", it->variable.c_str(),
                        it->location);
            }
        }
        textureVariablesDirty_ = false;
    }

    if (uniformVariablesDirty_) {
        std::lock_guard<std::mutex> lock(uniformVariablesLock_);
        for (auto it = uniformVariables_.begin(); it != uniformVariables_.end(); ++it) {
            LOGV("CustomShader::uniform:location: variable: %s", it->variable.c_str());
            if (-1 == it->location) {
                it->location = it->variableType.f_getLocation(program_->id());
                LOGV("CustomShader::uniform:location: location: %d", it->location);
            }
        }
        uniformVariablesDirty_ = false;
    }

    if (attributeVariablesDirty_) {
        std::lock_guard <std::mutex> lock(attributeVariablesLock_);
        for (auto it = attributeVariables_.begin(); it != attributeVariables_.end(); ++it) {
            if (-1 == it->location) {
                it->location = it->variableType.f_getLocation(program_->id());
                LOGV("CustomShader::attribute:location: variable: %s location: %d", it->variable.c_str(),
                        it->location);
            }
        }
        attributeVariablesDirty_ = false;
    }
}


	

CustomShader::~CustomShader() {
    free(material_transform_data);
    material_transform_data = nullptr;
    delete program_;
}
GLuint CustomShader::getProgramId(){
	return program_->id();
}
void CustomShader::addTextureKey(const std::string& variable_name, const std::string& key) {
    LOGV("CustomShader::texture:add variable: %s key: %s", variable_name.c_str(), key.c_str());
    Descriptor<TextureVariable> d(variable_name, key);

    d.variableType.f_getLocation = [variable_name] (GLuint programId) {
        return glGetUniformLocation(programId, variable_name.c_str());
    };

    d.variableType.f_bind = [key] (int& textureIndex, const Material& material, GLuint location) {
        glActiveTexture(GL_TEXTURE0 + textureIndex);
        Texture* texture = material.getTextureNoError(key);
        if (nullptr != texture) {
            glBindTexture(texture->getTarget(), texture->getId());
            glUniform1i(location, textureIndex++);
        }
    };

    std::lock_guard<std::mutex> lock(textureVariablesLock_);
    textureVariables_.insert(d);
    textureVariablesDirty_ = true;
}


void CustomShader::addAttributeFloatKey(const std::string& variable_name,
        const std::string& key) {
    AttributeVariableBind f =
            [key] (Mesh& mesh, GLuint location) {
                mesh.setVertexAttribLocF(location, key);
            };
    addAttributeKey(variable_name, key, f);
}


void CustomShader::addAttributeVec2Key(const std::string& variable_name,
        const std::string& key) {
    AttributeVariableBind f =
            [key] (Mesh& mesh, GLuint location) {
                mesh.setVertexAttribLocV2(location, key);
            };
    addAttributeKey(variable_name, key, f);
}
void CustomShader::addAttributeKey(const std::string& variable_name,
        const std::string& key, AttributeVariableBind f) {
    Descriptor<AttributeVariable> d(variable_name, key);

    d.variableType.f_getLocation = [variable_name] (GLuint programId) {
        return glGetAttribLocation(programId, variable_name.c_str());
    };
    d.variableType.f_bind = f;

    std::lock_guard <std::mutex> lock(attributeVariablesLock_);
    attributeVariables_.insert(d);
    attributeVariablesDirty_ = true;
}


void CustomShader::addAttributeVec3Key(const std::string& variable_name,
        const std::string& key) {
    AttributeVariableBind f =
            [key] (Mesh& mesh, GLuint location) {
                mesh.setVertexAttribLocV3(location, key);
            };
    addAttributeKey(variable_name, key, f);
}


void CustomShader::addAttributeVec4Key(const std::string& variable_name,
        const std::string& key) {
    AttributeVariableBind f =
            [key] (Mesh& mesh, GLuint location) {
                mesh.setVertexAttribLocV4(location, key);
            };
    addAttributeKey(variable_name, key, f);
}
void CustomShader::addUniformKey(const std::string& variable_name,
        const std::string& key, UniformVariableBind f) {
    LOGV("CustomShader::uniform:add variable: %s key: %s", variable_name.c_str(), key.c_str());
    Descriptor<UniformVariable> d(variable_name, key);

    d.variableType.f_getLocation = [variable_name] (GLuint programId) {
        return glGetUniformLocation(programId, variable_name.c_str());
    };
    d.variableType.f_bind = f;

    std::lock_guard <std::mutex> lock(uniformVariablesLock_);
    uniformVariables_.insert(d);
    uniformVariablesDirty_ = true;
}


void CustomShader::addUniformFloatKey(const std::string& variable_name,
        const std::string& key) {
    UniformVariableBind f =
            [key] (Material& material, GLuint location) {
                glUniform1f(location, material.getFloat(key));
            };
    addUniformKey(variable_name, key, f);
}


void CustomShader::addUniformVec2Key(const std::string& variable_name,
        const std::string& key) {
    UniformVariableBind f =
            [key] (Material& material, GLuint location) {
                glm::vec2 v = material.getVec2(key);
                glUniform2f(location, v.x, v.y);
            };
    addUniformKey(variable_name, key, f);
}


void CustomShader::addUniformVec3Key(const std::string& variable_name,
        const std::string& key) {
    UniformVariableBind f =
            [key] (Material& material, GLuint location) {
                glm::vec3 v = material.getVec3(key);
                glUniform3f(location, v.x, v.y, v.z);
            };
    addUniformKey(variable_name, key, f);
}


void CustomShader::addUniformVec4Key(const std::string& variable_name,
        const std::string& key) {
    UniformVariableBind f =
            [key] (Material& material, GLuint location) {
                glm::vec4 v = material.getVec4(key);
                glUniform4f(location, v.x, v.y, v.z, v.w);
            };
    addUniformKey(variable_name, key, f);
}

void CustomShader::addUniformMat4Key(const std::string& variable_name,
        const std::string& key) {
    UniformVariableBind f =
            [key] (Material& material, GLuint location) {
                glm::mat4 m = material.getMat4(key);
                glUniformMatrix4fv(location, 1, GL_FALSE, glm::value_ptr(m));
            };
    addUniformKey(variable_name, key, f);
}

void CustomShader::updateUbos(Material* material, RenderState* rstate, const std::vector<glm::mat4>& model_matrices, int draw_count){
    glm::vec4 material_ambient_color  = material->getVec4("ambient_color");
    glm::vec4 material_diffuse_color  = material->getVec4("diffuse_color");
    glm::vec4 material_specular_color = material->getVec4("specular_color");
    glm::vec4 emissive_color          = material->getVec4("emissive_color");
    float material_specular_exponent  = material->getFloat("specular_exponent");

    int offset = 0;

    memcpy(material_transform_data+ offset,glm::value_ptr(model_matrices[0]), sizeof(glm::mat4)*draw_count );

    offset += (do_batching ? BATCH_SIZE : 1) * sizeof(glm::mat4);
    memcpy(material_transform_data+ offset,glm::value_ptr(rstate->uniforms.u_proj), sizeof(glm::mat4));

    offset +=sizeof(glm::mat4);
    memcpy(material_transform_data+ offset,glm::value_ptr(rstate->uniforms.u_view_[0]), sizeof(glm::mat4)*2 );

    offset +=2*sizeof(glm::mat4);
    memcpy(material_transform_data + offset, glm::value_ptr(material_ambient_color), sizeof(material_ambient_color));

    offset += sizeof(material_ambient_color);
    memcpy(material_transform_data + offset, glm::value_ptr(material_diffuse_color), sizeof(material_diffuse_color));

    offset += sizeof(material_diffuse_color);
    memcpy(material_transform_data + offset, glm::value_ptr(material_specular_color), sizeof(material_specular_color));

    offset += sizeof(material_specular_color);
    memcpy(material_transform_data + offset, glm::value_ptr(emissive_color), sizeof(emissive_color));

    offset += sizeof(emissive_color);
    memcpy(material_transform_data + offset, &material_specular_exponent, sizeof(material_specular_exponent));

    size_t size = sizeof(glm::vec4)*4 + sizeof(float) + sizeof(glm::mat4) * 3 + sizeof(glm::mat4) * (do_batching ? BATCH_SIZE : 1);
    program_->updateMaterialTransformUBO(size,material_transform_data);

}

void CustomShader::render_batch(const std::vector<glm::mat4>& model_matrix,
        RenderData* render_data,  RenderState& rstate, unsigned int indexCount, int drawcount){
    Material* material = rstate.material_override;
    initializeOnDemand(&rstate);
    {
        std::lock_guard<std::mutex> lock(textureVariablesLock_);
        for (auto it = textureVariables_.begin(); it != textureVariables_.end(); ++it) {
            Texture* texture = material->getTextureNoError(it->key);
            if (texture == NULL) {
                LOGE(" texture is null for %s", render_data->owner_object()->name().c_str());
                return;
            }
            // If any texture is not ready, do not render the material at all
            if (!texture->isReady()) {
                LOGE(" texture is not ready for %s", render_data->owner_object()->name().c_str());
                return;
            }
        }
    }

    if(!rstate.use_multiview){
        rstate.uniforms.u_view_[0] = rstate.uniforms.u_view;
        rstate.uniforms.u_mv_[0] = rstate.uniforms.u_mv;
        rstate.uniforms.u_mv_it_[0] = rstate.uniforms.u_mv_it;
        rstate.uniforms.u_mvp_[0] = rstate.uniforms.u_mvp;
    }
    Mesh* mesh = render_data->mesh();
    GLuint programId = program_->id();
    glUseProgram(programId);

    updateUbos(material,&rstate,model_matrix, drawcount);
    /*
     * Update the bone matrices
     */
    int a_bone_indices = glGetAttribLocation(program_->id(), "a_bone_indices");
    int a_bone_weights = glGetAttribLocation(program_->id(), "a_bone_weights");

    if ((a_bone_indices >= 0) ||
        (a_bone_weights >= 0)){
        glm::mat4 finalTransform;
        mesh->setBoneLoc(a_bone_indices, a_bone_weights);
        mesh->generateBoneArrayBuffers(program_->id());
        int nBones = mesh->getVertexBoneData().getNumBones();
        if (nBones > MAX_BONES)
            nBones = MAX_BONES;
        const std::vector<glm::mat4>& bone_matrices = mesh->getVertexBoneData().getBoneMatrices();
        GLubyte* boneData = (GLubyte*)&bone_matrices[0][0][0];
       // LOGE(" size of bones is %d no of bones are %d", sizeof(bone_matrices), nBones);
        program_->updateBonesUBO(sizeof(glm::mat4)* nBones, boneData);
        checkGlError("CustomShader after bones");
    }
    /*
     * Bind textures
     */
    int texture_index = 0;
    {
        std::lock_guard<std::mutex> lock(textureVariablesLock_);
        for (auto it = textureVariables_.begin(); it != textureVariables_.end(); ++it) {
            auto d = *it;
            d.variableType.f_bind(texture_index, *material, d.location);
            texture_index++;
        }
    }
    /*
     * Update the uniforms for the lights
     */
    const std::vector<Light*>& lightlist = rstate.scene->getLightList();
    bool castShadow = false;
    for (auto it = lightlist.begin();
         it != lightlist.end();
         ++it) {
        Light* light = (*it);
         if (light != NULL) {
            light->render(program_->id(), texture_index);
            if (light->castShadow())
                castShadow = true;
         }
    }
    if (castShadow){
        Light::bindShadowMap(program_->id(), texture_index);
    }
    glBindVertexArray(render_data->mesh()->getVAOId(programId));

    if(rstate.use_multiview)
        glDrawElementsInstanced(render_data->draw_mode(),indexCount, GL_UNSIGNED_SHORT, NULL, 2 );
    else
        glDrawElements(render_data->draw_mode(), indexCount, GL_UNSIGNED_SHORT, 0);

    GL(glBindVertexArray(0));
    checkGlError(" TextureShader::render_batch");

}

void CustomShader::render(RenderState* rstate, RenderData* render_data, Material* material) {

	initializeOnDemand(rstate);
    {
        std::lock_guard<std::mutex> lock(textureVariablesLock_);
        for (auto it = textureVariables_.begin(); it != textureVariables_.end(); ++it) {
            Texture* texture = material->getTextureNoError(it->key);
            if (texture == NULL) {
            	LOGE(" texture is null for %s", render_data->owner_object()->name().c_str());
            	return;
            }
            // If any texture is not ready, do not render the material at all
            if (!texture->isReady()) {
            	LOGE(" texture is not ready for %s", render_data->owner_object()->name().c_str());
                return;
            }
        }
    }
    if(!rstate->use_multiview){
        rstate->uniforms.u_view_[0] = rstate->uniforms.u_view;
        rstate->uniforms.u_mv_[0] = rstate->uniforms.u_mv;
        rstate->uniforms.u_mv_it_[0] = rstate->uniforms.u_mv_it;
        rstate->uniforms.u_mvp_[0] = rstate->uniforms.u_mvp;
    }
    Mesh* mesh = render_data->mesh();
    glUseProgram(program_->id());

    std::vector<glm::mat4> model_matrix;
    model_matrix.push_back(rstate->uniforms.u_model);
    updateUbos(material,rstate,model_matrix, 1);

    /*
     * Update the bone matrices
     */
    int a_bone_indices = glGetAttribLocation(program_->id(), "a_bone_indices");
    int a_bone_weights = glGetAttribLocation(program_->id(), "a_bone_weights");

    if ((a_bone_indices >= 0) ||
        (a_bone_weights >= 0)){
        glm::mat4 finalTransform;
        mesh->setBoneLoc(a_bone_indices, a_bone_weights);
        mesh->generateBoneArrayBuffers(program_->id());
        int nBones = mesh->getVertexBoneData().getNumBones();
        if (nBones > MAX_BONES)
            nBones = MAX_BONES;
        const std::vector<glm::mat4>& bone_matrices = mesh->getVertexBoneData().getBoneMatrices();
        GLubyte* boneData = (GLubyte*)&bone_matrices[0][0][0];
       // LOGE(" size of bones is %d no of bones are %d", sizeof(bone_matrices), nBones);
        program_->updateBonesUBO(sizeof(glm::mat4)* nBones, boneData);
        checkGlError("CustomShader after bones");
    }
    /*
     * Bind textures
     */
    int texture_index = 0;
    {
        std::lock_guard<std::mutex> lock(textureVariablesLock_);
        for (auto it = textureVariables_.begin(); it != textureVariables_.end(); ++it) {
            auto d = *it;
            d.variableType.f_bind(texture_index, *material, d.location);
            texture_index++;
        }
    }
    /*
     * Update the uniforms for the lights
     */
    const std::vector<Light*>& lightlist = rstate->scene->getLightList();
    bool castShadow = false;
    for (auto it = lightlist.begin();
         it != lightlist.end();
         ++it) {
        Light* light = (*it);
         if (light != NULL) {
            light->render(program_->id(), texture_index);
            if (light->castShadow())
                castShadow = true;
         }
    }
    if (castShadow){
    	Light::bindShadowMap(program_->id(), texture_index);
    }

    checkGlError("CustomShader::render");
}

} /* namespace gvr */

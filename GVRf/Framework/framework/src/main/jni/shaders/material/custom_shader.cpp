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

GLubyte* mat_data =nullptr;
GLubyte* transforms_data =nullptr;
namespace gvr {
CustomShader::CustomShader(const std::string& vertex_shader, const std::string& fragment_shader)
    : vertexShader_(vertex_shader), fragmentShader_(fragment_shader) {

    size_t size = sizeof(glm::vec4)*4 + sizeof(float);
    mat_data= (GLubyte*)malloc(size);
    size = sizeof(glm::mat4) * 5;
    transforms_data =(GLubyte*)malloc(sizeof(glm::mat4)*5);
}


void CustomShader::initializeOnDemand() {
    if (nullptr == program_)
    {
        program_ = new GLProgram(vertexShader_.c_str(), fragmentShader_.c_str());
        vertexShader_.clear();
        fragmentShader_.clear();
        u_mvp_ = glGetUniformLocation(program_->id(), "u_mvp1");
        u_view_ = glGetUniformLocation(program_->id(), "u_view1");
        u_mv_ = glGetUniformLocation(program_->id(), "u_mv1");
        u_mv_it_ = glGetUniformLocation(program_->id(), "u_mv_it1");
        u_right_ = glGetUniformLocation(program_->id(), "u_right");
        u_model_ = glGetUniformLocation(program_->id(), "u_model1");
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
    free(mat_data);
    mat_data = nullptr;
    delete program_;
    free(transforms_data);
    transforms_data = nullptr;
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
        glActiveTexture(getGLTexture(textureIndex));
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


void CustomShader::render(RenderState* rstate, RenderData* render_data, Material* material) {

	initializeOnDemand();
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
   // LOGE("rendering %s with program %d", render_data->owner_object()->name().c_str(), program_->id());

    Mesh* mesh = render_data->mesh();
    glUseProgram(program_->id());


    glm::vec4 material_ambient_color  = material->getVec4("ambient_color");
    glm::vec4 material_diffuse_color  = material->getVec4("diffuse_color");
    glm::vec4 material_specular_color = material->getVec4("specular_color");
    glm::vec4 emissive_color          = material->getVec4("emissive_color");
    float material_specular_exponent  = material->getFloat("specular_exponent");

    memcpy(mat_data, glm::value_ptr(material_ambient_color), sizeof(material_ambient_color));
    int offset = sizeof(material_ambient_color);
    memcpy(mat_data + offset, glm::value_ptr(material_diffuse_color), sizeof(material_diffuse_color));

    offset += sizeof(material_diffuse_color);
    memcpy(mat_data + offset, glm::value_ptr(material_specular_color), sizeof(material_specular_color));

    offset += sizeof(material_specular_color);
    memcpy(mat_data + offset, glm::value_ptr(emissive_color), sizeof(emissive_color));
    offset += sizeof(emissive_color);

    memcpy(mat_data + offset, &material_specular_exponent, sizeof(material_specular_exponent));
    size_t size = sizeof(glm::vec4)*4 + 1*sizeof(float);
    program_->updateMateialUBO(size,mat_data);



    // Fill the data for transformations
    offset = 0;
    memcpy(transforms_data+ offset,glm::value_ptr(rstate->uniforms.u_model), sizeof(glm::mat4) );

    offset +=sizeof(glm::mat4);
    memcpy(transforms_data+ offset,glm::value_ptr(rstate->uniforms.u_mvp), sizeof(glm::mat4) );

    offset +=sizeof(glm::mat4);
    memcpy(transforms_data+ offset,glm::value_ptr(rstate->uniforms.u_view), sizeof(glm::mat4) );

    offset +=sizeof(glm::mat4);
    memcpy(transforms_data+ offset,glm::value_ptr(rstate->uniforms.u_mv), sizeof(glm::mat4) );

    offset +=sizeof(glm::mat4);
    memcpy(transforms_data+ offset,glm::value_ptr(rstate->uniforms.u_mv_it), sizeof(glm::mat4) );

    size = sizeof(glm::mat4) * 5;
    program_->updateTransformsUBO(size, transforms_data);
  //  LOGE("passed this point");


    /*
     * Update the bone matrices
     */
    int a_bone_indices = glGetAttribLocation(program_->id(), "a_bone_indices");
    int a_bone_weights = glGetAttribLocation(program_->id(), "a_bone_weights");
    int u_bone_matrices = glGetUniformLocation(program_->id(), "u_bone_matrix[0]");
    if ((a_bone_indices >= 0) ||
        (a_bone_weights >= 0) ||
        (u_bone_matrices >= 0)) {
        glm::mat4 finalTransform;
        mesh->setBoneLoc(a_bone_indices, a_bone_weights);
        mesh->generateBoneArrayBuffers(program_->id());
        int nBones = mesh->getVertexBoneData().getNumBones();
        if (nBones > MAX_BONES)
            nBones = MAX_BONES;
        for (int i = 0; i < nBones; ++i) {
            finalTransform = mesh->getVertexBoneData().getFinalBoneTransform(i);
            glUniformMatrix4fv(u_bone_matrices + i, 1, GL_FALSE, glm::value_ptr(finalTransform));
        }
        checkGlError("CustomShader after bones");
    }
    /*
     * Update values of uniform variables
     */
/*    {
        std::lock_guard<std::mutex> lock(uniformVariablesLock_);
        for (auto it = uniformVariables_.begin(); it != uniformVariables_.end(); ++it) {
            auto d = *it;
            try {
                    d.variableType.f_bind(*material, d.location);
            } catch(const std::string& exc) {

                //the keys defined for this shader might not have been used by the material yet
            }
        }
    }
*/ /*   if (u_model_ != -1){
    	glUniformMatrix4fv(u_model_, 1, GL_FALSE, glm::value_ptr(rstate->uniforms.u_model));
    }
    if (u_mvp_ != -1) {
        glUniformMatrix4fv(u_mvp_, 1, GL_FALSE, glm::value_ptr(rstate->uniforms.u_mvp));
    }
    if (u_view_ != -1) {
        glUniformMatrix4fv(u_view_, 1, GL_FALSE, glm::value_ptr(rstate->uniforms.u_view));
    }
    if (u_mv_ != -1) {
        glUniformMatrix4fv(u_mv_, 1, GL_FALSE, glm::value_ptr(rstate->uniforms.u_mv));
    }
    if (u_mv_it_ != -1) {
        glUniformMatrix4fv(u_mv_it_, 1, GL_FALSE, glm::value_ptr(rstate->uniforms.u_mv_it));
    }
    if (u_right_ != 0) {
        glUniform1i(u_right_, rstate->uniforms.u_right ? 1 : 0);
    }
  */  /*
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

int CustomShader::getGLTexture(int n) {
    switch (n) {
    case 0:
        return GL_TEXTURE0;
    case 1:
        return GL_TEXTURE1;
    case 2:
        return GL_TEXTURE2;
    case 3:
        return GL_TEXTURE3;
    case 4:
        return GL_TEXTURE4;
    case 5:
        return GL_TEXTURE5;
    case 6:
        return GL_TEXTURE6;
    case 7:
        return GL_TEXTURE7;
    case 8:
        return GL_TEXTURE8;
    case 9:
        return GL_TEXTURE9;
    case 10:
        return GL_TEXTURE10;
    default:
        return GL_TEXTURE0;
    }
}

} /* namespace gvr */

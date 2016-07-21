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
 * RAII class for GL programs.
 ***************************************************************************/

#ifndef GL_PROGRAM_H_
#define GL_PROGRAM_H_

#ifndef GL_ES_VERSION_3_0
#include "GLES3/gl3.h"
#endif

#include "engine/memory/gl_delete.h"

#include "util/gvr_log.h"
#include "util/gvr_gl.h"
#define MATERIAL_UBO_INDEX 0
#define LIGHT_UBO_INDEX 1
#define TRANSFORM_UBO_INDEX 2

namespace gvr {
class UBO{
private:
    GLuint bind_index_;
    GLuint buffer_id_;
    GLuint block_index_ ;
public:
    void createBuffer(size_t size, GLuint program_id, const char* block_name, int bind_index){
        glGenBuffers(1,&buffer_id_);
        glBindBuffer(GL_UNIFORM_BUFFER, buffer_id_);
        glBufferData(GL_UNIFORM_BUFFER, size, NULL, GL_DYNAMIC_DRAW);
        block_index_ = glGetUniformBlockIndex(program_id,
                block_name);

        bind_index_ = bind_index;
        glUniformBlockBinding(program_id, block_index_, bind_index_);
        glBindBufferBase(GL_UNIFORM_BUFFER, bind_index_, buffer_id_);
    }
    void updateBuffer(size_t size, unsigned char* data){

        glBindBuffer(GL_UNIFORM_BUFFER, buffer_id_);
        glBufferData(GL_UNIFORM_BUFFER, size, data, GL_DYNAMIC_DRAW);
    }
    GLuint getId(){
        return buffer_id_;
    }
    GLuint getBlockIndex(){
        return block_index_;
    }
};
class GLProgram {
    UBO* material_ubo_;
    UBO* light_ubo_;
    UBO* transform_ubo_;

public:
    void updateMateialUBO(size_t size, unsigned char* data){
        if(material_ubo_ == nullptr){
            material_ubo_ = new UBO();
            material_ubo_->createBuffer(size, id_, "Material_UBO", MATERIAL_UBO_INDEX);
        }
        material_ubo_->updateBuffer(size, data);
    }
    void updateTransformsUBO(size_t size, unsigned char* data){
        if(transform_ubo_ == nullptr){
            transform_ubo_ = new UBO();
            transform_ubo_->createBuffer(size, id_, "Transforms_UBO", TRANSFORM_UBO_INDEX);
        }
        transform_ubo_->updateBuffer(size, data);
    }
/*    void updateLightUBO(size_t size, unsigned char* data){
        if(light_ubo_ == nullptr){
            light_ubo_ = new UBO();
            light_ubo_->createBuffer(size, id_, "light_UBO", TRANSFORM_UBO_INDEX);
        }
        light_ubo_->updateBuffer(size, data);
    }
*/
    GLProgram(const char* pVertexSourceStrings,
            const char* pFragmentSourceStrings): material_ubo_(nullptr), light_ubo_(nullptr), transform_ubo_(nullptr) {
        deleter_ = getDeleterForThisThread();
        GLint vertex_shader_string_lengths[1] = { (GLint) strlen(
                pVertexSourceStrings) };
        GLint fragment_shader_string_lengths[1] = { (GLint) strlen(
                pFragmentSourceStrings) };

        id_ = createProgram(1, &pVertexSourceStrings,
                vertex_shader_string_lengths, &pFragmentSourceStrings,
                fragment_shader_string_lengths);
    }

    GLProgram(const char** pVertexSourceStrings,
            const GLint* pVertexSourceStringLengths,
            const char** pFragmentSourceStrings,
            const GLint* pFragmentSourceStringLengths, int count)
            : material_ubo_(nullptr), light_ubo_(nullptr), transform_ubo_(nullptr),
            id_(
                    createProgram(count, pVertexSourceStrings,
                            pVertexSourceStringLengths, pFragmentSourceStrings,
                            pFragmentSourceStringLengths)) {
        deleter_ = getDeleterForThisThread();
    }

    ~GLProgram() {
        deleter_->queueProgram(id_);
    }

    GLuint id() const {
        return id_;
    }

    static void checkGlError(const char* op) {
        for (GLint error = glGetError(); error; error = glGetError()) {
            LOGI("after %s() glError (0x%x)\n", op, error);
        }
    }

    GLuint loadShader(GLenum shaderType, int strLength, const char** pSourceStrings,
            const GLint*pSourceStringLengths) {
        GLuint shader = glCreateShader(shaderType);
        if (shader) {
            glShaderSource(shader, strLength, pSourceStrings, pSourceStringLengths);
            glCompileShader(shader);
            GLint compiled = 0;
            glGetShaderiv(shader, GL_COMPILE_STATUS, &compiled);
            if (!compiled) {
                GLint infoLen = 0;
                glGetShaderiv(shader, GL_INFO_LOG_LENGTH, &infoLen);
                if (infoLen) {
                    char* buf = (char*) malloc(infoLen);
                    if (buf) {
                        glGetShaderInfoLog(shader, infoLen, NULL, buf);
                        LOGE("Could not compile shader %d:\n%s\n", shaderType,
                                buf);
                        free(buf);
                    }
                    deleter_->queueShader(shader);
                    shader = 0;
                }
            }
        }
        return shader;
    }

    GLuint createProgram(int strLength,
            const char** pVertexSourceStrings,
            const GLint* pVertexSourceStringLengths,
            const char** pFragmentSourceStrings,
            const GLint* pFragmentSourceStringLengths) {
        GLuint vertexShader = loadShader(GL_VERTEX_SHADER, strLength,
                pVertexSourceStrings, pVertexSourceStringLengths);
        if (!vertexShader) {
            return 0;
        }

        GLuint pixelShader = loadShader(GL_FRAGMENT_SHADER, strLength,
                pFragmentSourceStrings, pFragmentSourceStringLengths);
        if (!pixelShader) {
            return 0;
        }

        GLuint program = glCreateProgram();
        if (program) {
            glAttachShader(program, vertexShader);
            checkGlError("glAttachShader");
            glAttachShader(program, pixelShader);
            checkGlError("glAttachShader");
            bindCommonAttributes(program);
            glLinkProgram(program);
            GLint linkStatus = GL_FALSE;
            glGetProgramiv(program, GL_LINK_STATUS, &linkStatus);
            if (linkStatus != GL_TRUE) {
                GLint bufLength = 0;
                glGetProgramiv(program, GL_INFO_LOG_LENGTH, &bufLength);
                if (bufLength) {
                    char* buf = (char*) malloc(bufLength);
                    if (buf) {
                        glGetProgramInfoLog(program, bufLength, NULL, buf);
                        LOGE("Could not link program:\n%s\n", buf);
                        free(buf);
                    }
                }
                deleter_->queueProgram(program);
                program = 0;
            }
        }
        return program;
    }

//    enum attributeBindLocation {
//        POSITION_ATTRIBUTE_LOCATION = 0,
//        TEXCOORD_ATTRIBUT_LOCATION = 1,
//        NORMAL_ATTRIBUTE_LOCATION = 2
//    };

private:
    GLuint id_;
    GlDelete* deleter_;

    static void bindCommonAttributes(GLuint id) {
//        glBindAttribLocation(id, POSITION_ATTRIBUTE_LOCATION, "a_position");
//        glBindAttribLocation(id, TEXCOORD_ATTRIBUT_LOCATION, "a_tex_coord");
//        glBindAttribLocation(id, NORMAL_ATTRIBUTE_LOCATION, "a_normal");
    }

};
}
#endif

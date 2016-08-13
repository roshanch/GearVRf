//
// Created by roshan.c on 8/12/16.
//

#ifndef FRAMEWORK_UNIFORM_BUFFER_H
#define FRAMEWORK_UNIFORM_BUFFER_H

#ifndef GL_ES_VERSION_3_0
#include "GLES3/gl3.h"
#endif

#include "engine/memory/gl_delete.h"

#include "util/gvr_log.h"
#include "util/gvr_gl.h"
#define MATERIAL_UBO_INDEX 0
#define LIGHT_UBO_INDEX 1
#define TRANSFORM_UBO_INDEX 2
#define BONE_UBO_INDEX  3

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
        checkGLError("after create buffer");
    }
    void updateBuffer(size_t size, unsigned char* data){

        glBindBuffer(GL_UNIFORM_BUFFER, buffer_id_);
        glBufferData(GL_UNIFORM_BUFFER, size, data, GL_DYNAMIC_DRAW);
        checkGLError("after data supplied");
    }
    GLuint getId(){
        return buffer_id_;
    }
    GLuint getBlockIndex(){
        return block_index_;
    }
};
}
#endif //FRAMEWORK_UNIFORM_BUFFER_H

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
 * A frame buffer object.
 ***************************************************************************/

#ifndef GL_RENDER_TEXTURE_H_
#define GL_RENDER_TEXTURE_H_

#include "objects/textures/render_texture.h"
#include "gl/gl_render_buffer.h"
#include "gl/gl_frame_buffer.h"
#include "gl/gl_image.h"
#include "eglextension/msaa/msaa.h"
#include "util/gvr_gl.h"

namespace gvr {
class GLRenderImage;
class Renderer;
class GLRenderTexture : public RenderTexture
{
public:
    explicit GLRenderTexture(int width, int height, int sample_count, int layers, int depth_format);
    explicit GLRenderTexture(int width, int height, int sample_count,
            int jcolor_format, int jdepth_format, bool resolve_depth,
            const TextureParameters* texture_parameters);

    virtual ~GLRenderTexture();

    virtual int getId() { return mImage->getId(); }
    virtual int width() const { return mImage->getWidth(); }
    virtual int height() const { return mImage->getHeight(); }

    virtual unsigned int getFrameBufferId() const {
        return renderTexture_gl_frame_buffer_->id();
    }

    virtual void bind() {
        glBindFramebuffer(GL_FRAMEBUFFER, renderTexture_gl_frame_buffer_->id());
    }

    virtual void beginRendering(Renderer*);
    virtual void endRendering(Renderer*);
    virtual bool isReady();

    // Start to read back texture in the background. It can be optionally called before
    // readRenderResult() to read pixels asynchronously. This function returns immediately.
    virtual  void startReadBack();

    // Copy data in pixel buffer to client memory. This function is synchronous. When
    // it returns, the pixels have been copied to PBO and then to the client memory.
    virtual bool readRenderResult(unsigned int *readback_buffer, long capacity);

    bool bindTexture(int gl_location, int texIndex);

private:
    GLRenderTexture(const GLRenderTexture&);
    GLRenderTexture(GLRenderTexture&&);
    GLRenderTexture& operator=(const GLRenderTexture&);
    GLRenderTexture& operator=(GLRenderTexture&&);


    void invalidateFrameBuffer(GLenum target, bool is_fbo, const bool color_buffer, const bool depth_buffer);


protected:
    void initialize();
    void generateRenderTextureNoMultiSampling(int jdepth_format,GLenum depth_format, int width, int height);
    void generateRenderTextureEXT(int sample_count,int jdepth_format,GLenum depth_format, int width, int height);
    void generateRenderTexture(int sample_count, int jdepth_format, GLenum depth_format, int width,
                               int height, int jcolor_format);
    GLenum depth_format_;
    GLRenderBuffer* renderTexture_gl_render_buffer_ = nullptr;// This is actually depth buffer.
    GLFrameBuffer* renderTexture_gl_frame_buffer_ = nullptr;
    GLFrameBuffer* renderTexture_gl_resolve_buffer_ = nullptr;
    GLRenderBuffer* renderTexture_gl_color_buffer_ = nullptr;// This is only for multisampling case
                                     // when resolveDepth is on.
    GLuint renderTexture_gl_pbo_ = 0;
};
class GLMultiviewRenderTexture: public GLRenderTexture
{
public:
    int mLayers_;
    GLuint frameBufferDepthTextureId;
    GLuint render_texture_gl_texture_ = 0;
    GLFrameBuffer* renderTexture_gl_read_buffer = nullptr;
    GLuint getReadBufferId(){
        if(renderTexture_gl_read_buffer == NULL)
            renderTexture_gl_read_buffer = new GLFrameBuffer();

        return  renderTexture_gl_read_buffer->id();
    }
    void startReadBack(int layer);
    explicit GLMultiviewRenderTexture(int width, int height, int sample_count, int layers, int depth_format): GLRenderTexture(width, height, sample_count, layers, depth_format),
                                                                                                              mLayers_(layers){}
    explicit GLMultiviewRenderTexture(int width, int height, int sample_count,
                                         int jcolor_format, int jdepth_format, bool resolve_depth,
                                         const TextureParameters* texture_parameters, int layers);
    bool readRenderResult(uint32_t *readback_buffer, long capacity, int layer);
    virtual ~GLMultiviewRenderTexture(){

    }
    virtual bool isReady() {
        return GLRenderTexture::isReady();
    }
    virtual void beginRendering(Renderer* renderer){
        if (!isReady())
        {
            return;
        }

        bind();
        GLRenderTexture::beginRendering(renderer);
    }

};

class GLNonMultiviewRenderTexture: public GLRenderTexture
{
public:
    int layer_index_;
    explicit GLNonMultiviewRenderTexture(int width, int height, int sample_count, int layers, int depth_format): GLRenderTexture(width, height, sample_count, layers, depth_format) {}
    explicit GLNonMultiviewRenderTexture(int width, int height, int sample_count,
                             int jcolor_format, int jdepth_format, bool resolve_depth,
                             const TextureParameters* texture_parameters);
    void generateRenderTextureLayer(GLenum depth_format, int width, int height);
    void bindFrameBufferToLayer(int layerIndex);
    bool readRenderResult(uint32_t *readback_buffer, long capacity);
    virtual ~GLNonMultiviewRenderTexture(){

    }
    void startReadBack(int layer);
    virtual bool isReady(){
        bool status = GLRenderTexture::isReady();
        if (renderTexture_gl_frame_buffer_ == NULL)
        {
            renderTexture_gl_frame_buffer_ = new GLFrameBuffer();
            generateRenderTextureLayer(depth_format_, width(), height());
            checkGLError("RenderTexture::isReady generateRenderTextureLayer");
        }
        return status;
    }
    virtual void beginRendering(Renderer* renderer){
        if (!isReady())
        {
            return;
        }

        bind();
        if (mImage->getDepth() > 1)
        {
            LOGV("GLRenderTexture::beginRendering layer=%d", layer_index_);
            glFramebufferTextureLayer(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, mImage->getId(), 0, layer_index_);
        }
        GLRenderTexture::beginRendering(renderer);

    }
};
}
#endif

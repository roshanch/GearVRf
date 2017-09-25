//
// Created by roshan on 9/22/17.
//


#include "gl_render_target.h"
#include "../objects/textures/render_texture.h"
#include "../engine/renderer/renderer.h"
#include "../util/gvr_log.h"

namespace gvr{
void  GLRenderTarget::beginRendering(Renderer* renderer){
    mRenderTexture->useStencil(renderer->useStencilBuffer());
    mRenderState.viewportWidth = mRenderTexture->width();
    mRenderState.viewportHeight = mRenderTexture->height();
    if (-1 != mRenderState.camera->background_color_r())
    {
        mRenderTexture->setBackgroundColor(mRenderState.camera->background_color_r(),
                                           mRenderState.camera->background_color_g(),
                                           mRenderState.camera->background_color_b(), mRenderState.camera->background_color_a());
    }
    mRenderTexture->beginRendering(renderer);
    checkGLError("RenderTarget::beginRendering");
}
void  GLRenderTarget::endRendering(Renderer* renderer){
    mRenderTexture->endRendering(renderer);
    checkGLError("RenderTarget::endRendering");
}
}
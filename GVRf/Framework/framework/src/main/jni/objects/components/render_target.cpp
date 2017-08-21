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

#include "render_target.h"
#include "component.inl"
#include "objects/textures/render_texture.h"
namespace gvr {

/**
 * Constructs a render target component which renders to a designated texture.
 * The scene will be rendered from the viewpoint of the scene object
 * the RenderTarget is attached to. Nothing will be rendered if
 * the render target is not attached to a scene object or
 * if it does not have a texture.
 *
 * If a RenderTarget is actually a ShadowMap, it is rendered
 * automatically by the lighting code. Otherwise, the
 * Java application is responsible for initiating rendering.
 *
 * @param texture RenderTexture to render to
 */
RenderTarget::RenderTarget(RenderTexture* tex, bool is_multiview)
: Component(RenderTarget::getComponentType()),
  mRenderTexture(tex),mRenderDataVector(std::make_shared< std::vector<RenderData*>>())
  //mCamera(nullptr)
{
 //   mRenderState.viewportY = 0;
//    mRenderState.viewportX = 0;
    mRenderState.shadow_map = false;
    LOGE("Roshan constructor %d", is_multiview);
    mRenderState.material_override = NULL;
    mRenderState.is_multiview = is_multiview;

}
RenderTarget::RenderTarget(Scene* scene)
: Component(RenderTarget::getComponentType()), mRenderTexture(nullptr),mRenderDataVector(std::make_shared< std::vector<RenderData*>>()){
    mRenderState.shadow_map = false;
    mRenderState.material_override = NULL;
    mRenderState.is_multiview = false;
    mRenderState.scene = scene;

}
RenderTarget::RenderTarget(RenderTexture* tex, const RenderTarget* source)
        : Component(RenderTarget::getComponentType()),
          mRenderTexture(tex), mRenderDataVector(source->mRenderDataVector)
{
    mRenderState.shadow_map = false;
    mRenderState.material_override = NULL;
    mRenderState.is_multiview = false;
}
/**
 * Constructs an empty render target without a render texture.
 * This component will not render anything until a RenderTexture
 * is provided.
 */
RenderTarget::RenderTarget()
:   Component(RenderTarget::getComponentType()),
    mRenderTexture(nullptr), mRenderDataVector(std::make_shared< std::vector<RenderData*>>())
   // mCamera(nullptr)
{
    //mRenderState.viewportY = 0;
 //   mRenderState.viewportX = 0;
    mRenderState.is_multiview = false;
    mRenderState.shadow_map = false;
    mRenderState.material_override = NULL;
}
 void RenderTarget::cullFromCamera(Scene* scene, Camera* camera, Renderer* renderer, ShaderManager* shader_manager){
    if(camera == nullptr)
        LOGE("camera is NULL");
     renderer->cullFromCamera(scene, camera,shader_manager, mRenderDataVector.get(),mRenderState.is_multiview);
     renderer->state_sort(mRenderDataVector.get());
}


RenderTarget::~RenderTarget()
{
    if (mRenderTexture)
    {
        delete mRenderTexture;
    }
}

/**
 * Designates the RenderTexture this RenderTarget should render to.
 * @param RenderTexture to render to
 * @see #getTexture()
 * @see RenderTexture
 */
void RenderTarget::setTexture(RenderTexture* texture)
{
    mRenderTexture = texture;
}

/**
 * Setup to start rendering to this render target.
 * You should not call this function if there is
 * no RenderTexture.
 */
void  RenderTarget::beginRendering(Renderer* renderer )
{
 //   mRenderState.render_mask = mRenderState.camera->render_mask();
 //   mRenderState.uniforms.u_right = mRenderState.render_mask & RenderData::RenderMaskBit::Right;
 //   mRenderState.uniforms.u_proj = mRenderState.camera->getProjectionMatrix();
//    mRenderState.uniforms.u_view = mRenderState.camera->getViewMatrix();
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

/**
 * Clean up after rendering to this render target.
 * You should not call this function if there is
 * no RenderTexture.
 */
void RenderTarget::endRendering(Renderer* renderer)
{
    mRenderTexture->endRendering(renderer);
    checkGLError("RenderTarget::endRendering");
}

}

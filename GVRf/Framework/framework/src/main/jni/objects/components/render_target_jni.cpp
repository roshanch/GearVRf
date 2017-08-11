/***************************************************************************
 * JNI
 ***************************************************************************/

#include "objects/components/render_target.h"

#include "util/gvr_jni.h"
#include "util/gvr_log.h"
#include "glm/gtc/type_ptr.hpp"

namespace gvr {
    extern "C" {
    JNIEXPORT jlong JNICALL
    Java_org_gearvrf_NativeRenderTarget_ctorMultiview(JNIEnv *env, jobject obj, jobject jtexture, jboolean isMultiview);

    JNIEXPORT jlong JNICALL
    Java_org_gearvrf_NativeRenderTarget_ctor(JNIEnv *env, jobject obj, jobject jtexture,  jlong ptr);

    JNIEXPORT jlong JNICALL
    Java_org_gearvrf_NativeRenderTarget_getComponentType(JNIEnv *env, jobject obj);

    JNIEXPORT void JNICALL
    Java_org_gearvrf_NativeRenderTarget_setTexture(JNIEnv *env, jobject obj, jlong ptr, jobject texture);

    JNIEXPORT void JNICALL
    Java_org_gearvrf_NativeRenderTarget_setMainScene(JNIEnv *env, jobject obj, jlong ptr, jlong Sceneptr);

    JNIEXPORT void JNICALL
    Java_org_gearvrf_NativeRenderTarget_setCamera(JNIEnv *env, jobject obj, jlong ptr, jlong camera);

    JNIEXPORT void JNICALL
    Java_org_gearvrf_NativeRenderTarget_beginRendering(JNIEnv *env, jobject obj, jlong ptr);

    JNIEXPORT void JNICALL
    Java_org_gearvrf_NativeRenderTarget_endRendering(JNIEnv *env, jobject obj, jlong ptr);

    JNIEXPORT jlong JNICALL
    Java_org_gearvrf_NativeRenderTarget_defaultCtr(JNIEnv *env, jobject obj, jlong jscene);

    JNIEXPORT void JNICALL
    Java_org_gearvrf_NativeRenderTarget_cullFromCamera(JNIEnv *env, jobject obj, jlong ptr, jlong jcamera, jlong jshaderManager);
    JNIEXPORT void JNICALL
    Java_org_gearvrf_NativeRenderTarget_render(JNIEnv *env, jobject obj, jlong renderTarget, jlong camera, jlong shader_manager, jlong posteffectrenderTextureA, jlong posteffectRenderTextureB);
    };

JNIEXPORT void JNICALL
Java_org_gearvrf_NativeRenderTarget_render(JNIEnv *env, jobject obj, jlong renderTarget, jlong camera, jlong shader_manager, jlong posteffectrenderTextureA, jlong posteffectRenderTextureB){
    RenderTarget* target = reinterpret_cast<RenderTarget*>(renderTarget);
    target->setCamera(reinterpret_cast<Camera*>(camera));
    gRenderer->getInstance()->renderRenderTarget(target, reinterpret_cast<ShaderManager*>(shader_manager),
                                                 reinterpret_cast<RenderTexture*>(posteffectrenderTextureA), reinterpret_cast<RenderTexture*>(posteffectRenderTextureB));
}

JNIEXPORT jlong JNICALL
Java_org_gearvrf_NativeRenderTarget_defaultCtr(JNIEnv *env, jobject obj, jlong jscene){
    Scene* scene = reinterpret_cast<Scene*>(jscene);
    return reinterpret_cast<jlong>(new RenderTarget(scene));

}
JNIEXPORT jlong JNICALL
Java_org_gearvrf_NativeRenderTarget_ctorMultiview(JNIEnv *env, jobject obj, jobject jtexture, jboolean isMultiview)
{
    RenderTexture* texture = reinterpret_cast<RenderTexture*>(jtexture);
    return reinterpret_cast<jlong>(new RenderTarget(texture, isMultiview));
}
JNIEXPORT jlong JNICALL
Java_org_gearvrf_NativeRenderTarget_ctor(JNIEnv *env, jobject obj, jobject jtexture, jlong ptr)
{
    RenderTexture* texture = reinterpret_cast<RenderTexture*>(jtexture);
    RenderTarget* sourceRenderTarget = reinterpret_cast<RenderTarget*>(ptr);
    return reinterpret_cast<jlong>(new RenderTarget(texture, sourceRenderTarget));
}
JNIEXPORT void JNICALL
Java_org_gearvrf_NativeRenderTarget_setMainScene(JNIEnv *env, jobject obj, jlong ptr, jlong Sceneptr){
    RenderTarget* target = reinterpret_cast<RenderTarget*>(ptr);
    Scene* scene = reinterpret_cast<Scene*>(Sceneptr);
    target->setMainScene(scene);
}

JNIEXPORT void JNICALL
Java_org_gearvrf_NativeRenderTarget_beginRendering(JNIEnv *env, jobject obj, jlong ptr){
    RenderTarget* target = reinterpret_cast<RenderTarget*>(ptr);
    target->beginRendering(gRenderer->getInstance());
}

JNIEXPORT void JNICALL
Java_org_gearvrf_NativeRenderTarget_endRendering(JNIEnv *env, jobject obj, jlong ptr){
    RenderTarget* target = reinterpret_cast<RenderTarget*>(ptr);
    target->endRendering(gRenderer->getInstance());
}


JNIEXPORT void JNICALL
Java_org_gearvrf_NativeRenderTarget_cullFromCamera(JNIEnv *env, jobject obj, jlong ptr, jlong jcamera, jlong jshaderManager){

    RenderTarget* target = reinterpret_cast<RenderTarget*>(ptr);
    Camera* camera = reinterpret_cast<Camera*>(jcamera);
    ShaderManager* shaderManager = reinterpret_cast<ShaderManager*> (jshaderManager);
    target->cullFromCamera(camera,gRenderer->getInstance(), shaderManager);
}

JNIEXPORT void JNICALL
Java_org_gearvrf_NativeRenderTarget_setTexture(JNIEnv *env, jobject obj, jlong ptr, jobject jtexture)
{
    RenderTarget* target = reinterpret_cast<RenderTarget*>(ptr);
    RenderTexture* texture = reinterpret_cast<RenderTexture*>(jtexture);
    target->setTexture(texture);
}

JNIEXPORT void JNICALL
Java_org_gearvrf_NativeRenderTarget_setCamera(JNIEnv *env, jobject obj, jlong ptr, jlong jcamera)
{
    RenderTarget* target = reinterpret_cast<RenderTarget*>(ptr);
    Camera* camera = reinterpret_cast<Camera*>(jcamera);
    target->setCamera(camera);
}

JNIEXPORT jlong JNICALL
Java_org_gearvrf_NativeRenderTarget_getComponentType(JNIEnv * env, jobject obj)
{
    return RenderTarget::getComponentType();
}

}
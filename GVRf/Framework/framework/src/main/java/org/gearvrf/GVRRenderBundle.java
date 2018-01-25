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
package org.gearvrf;

import android.transition.Scene;

import org.gearvrf.utility.VrAppSettings;

/** A container for various services and pieces of data required for rendering. */
final class GVRRenderBundle implements IRenderBundle {
    private  GVRContext mGVRContext;
    private  GVRMaterialShaderManager mMaterialShaderManager;
    private  GVRRenderTexture mPostEffectRenderTextureA = null;
    private  GVRRenderTexture mPostEffectRenderTextureB = null;
    private  GVRRenderTarget mEyeCaptureRenderTarget = null;

    private int  mSampleCount;
    private int mWidth, mHeight;
    private GVRRenderTarget[] mLeftEyeRenderTarget = new GVRRenderTarget[3];
    private GVRRenderTarget [] mRightEyeRenderTarget = new GVRRenderTarget[3];
    private GVRRenderTarget [] mMultiviewRenderTarget = new GVRRenderTarget[3];


    private GVRRenderTexture mEyeCapturePostEffectRenderTextureA = null;
    private GVRRenderTexture mEyeCapturePostEffectRenderTextureB = null;

    GVRRenderBundle(GVRContext gvrContext, final int width, final int height) {
        mGVRContext = gvrContext;
        mMaterialShaderManager = new GVRMaterialShaderManager(gvrContext);

        final VrAppSettings appSettings = mGVRContext.getActivity().getAppSettings();
        mSampleCount = appSettings.getEyeBufferParams().getMultiSamples() < 0 ? 0
                : appSettings.getEyeBufferParams().getMultiSamples();
        if (mSampleCount > 1) {
            int maxSampleCount = GVRMSAA.getMaxSampleCount();
            if (mSampleCount > maxSampleCount && maxSampleCount > 1) {
                mSampleCount = maxSampleCount;
            }
        }
        mWidth = mGVRContext.getActivity().getAppSettings().getEyeBufferParams().getResolutionWidth();
        mHeight = mGVRContext.getActivity().getAppSettings().getEyeBufferParams().getResolutionHeight();
    }
    public void createRenderTarget(int index, GVRViewManager.EYE eye, long renderTextureInfo){

        if(eye == GVRViewManager.EYE.MULTIVIEW)
            mMultiviewRenderTarget[index] = new GVRRenderTarget(new GVRRenderTexture(mGVRContext,  mWidth, mHeight,
                getRenderTexture(renderTextureInfo)), mGVRContext.getMainScene(), true);

        else if(eye == GVRViewManager.EYE.LEFT)
            mLeftEyeRenderTarget[index] = new GVRRenderTarget(new GVRRenderTexture(mGVRContext, mWidth, mHeight,
                    getRenderTexture(renderTextureInfo)), mGVRContext.getMainScene());
        else
            mRightEyeRenderTarget[index] = new GVRRenderTarget(new GVRRenderTexture(mGVRContext, mWidth, mHeight,
                    getRenderTexture(renderTextureInfo)), mGVRContext.getMainScene(),mLeftEyeRenderTarget[index]);

    }


    public void createRenderTargetChain(boolean use_multiview){

        for(int i=0; i< 3; i++){
            if(use_multiview){
                addRenderTarget(mMultiviewRenderTarget[i].getNative(), GVRViewManager.EYE.MULTIVIEW.ordinal(), i);
            }
            else {
                addRenderTarget(mLeftEyeRenderTarget[i].getNative(), GVRViewManager.EYE.LEFT.ordinal(), i);
                addRenderTarget(mRightEyeRenderTarget[i].getNative(), GVRViewManager.EYE.RIGHT.ordinal(), i);
            }

        }
        for(int i=0; i< 3; i++){
            int index = (i+1) % 3;
            if(use_multiview)
                mMultiviewRenderTarget[i].attachRenderTarget(mMultiviewRenderTarget[index]);
            else {
                mLeftEyeRenderTarget[i].attachRenderTarget(mLeftEyeRenderTarget[index]);
                mRightEyeRenderTarget[i].attachRenderTarget(mRightEyeRenderTarget[index]);
            }
        }
    }
    public GVRRenderTarget getEyeCaptureRenderTarget() {
        if(mEyeCaptureRenderTarget == null){
            mEyeCaptureRenderTarget  = new GVRRenderTarget(new GVRRenderTexture(mGVRContext, mWidth, mHeight, mSampleCount), mGVRContext.getMainScene());
            mEyeCaptureRenderTarget.setCamera(mGVRContext.getMainScene().getMainCameraRig().getCenterCamera());
        }
        return  mEyeCaptureRenderTarget;
    }

    public GVRRenderTarget getLeftEyeRenderTarget(int index){
        return mLeftEyeRenderTarget[index];
    }

    public GVRRenderTarget getRightEyeRenderTarget(int index){
       return mRightEyeRenderTarget[index];
    }

    public GVRRenderTarget getMultiviewRenderTarget(int index){
        return mMultiviewRenderTarget[index];
    }

    public GVRRenderTarget getRenderTarget(GVRViewManager.EYE eye, int index){
        if(eye == GVRViewManager.EYE.LEFT)
            return getLeftEyeRenderTarget(index);
        if(eye == GVRViewManager.EYE.RIGHT)
            return getRightEyeRenderTarget(index);

        return getMultiviewRenderTarget(index);
    }
    public GVRRenderTexture getEyeCapturePostEffectRenderTextureA(){
        if(mEyeCapturePostEffectRenderTextureA == null)
            mEyeCapturePostEffectRenderTextureA  = new GVRRenderTexture(mGVRContext, mWidth , mHeight, mSampleCount, 1);
        return mEyeCapturePostEffectRenderTextureA;
    }

    public GVRRenderTexture getEyeCapturePostEffectRenderTextureB(){
        if(mEyeCapturePostEffectRenderTextureB == null)
            mEyeCapturePostEffectRenderTextureB = new GVRRenderTexture(mGVRContext, mWidth , mHeight, mSampleCount, 1);

        return mEyeCapturePostEffectRenderTextureB;
    }
    public GVRMaterialShaderManager getMaterialShaderManager() {
        return mMaterialShaderManager;
    }

    public GVRPostEffectShaderManager getPostEffectShaderManager() {
        return null;
    }

    public GVRRenderTexture getPostEffectRenderTextureA() {
        if(mPostEffectRenderTextureA == null)
            mPostEffectRenderTextureA = new GVRRenderTexture(mGVRContext, mWidth , mHeight, mSampleCount, mGVRContext.getActivity().getAppSettings().isMultiviewSet() ? 2 : 1);
        return mPostEffectRenderTextureA;
    }

    public GVRRenderTexture getPostEffectRenderTextureB() {
        if(mPostEffectRenderTextureB == null)
            mPostEffectRenderTextureB = new GVRRenderTexture(mGVRContext, mWidth , mHeight, mSampleCount, mGVRContext.getActivity().getAppSettings().isMultiviewSet() ? 2 : 1);

        return mPostEffectRenderTextureB;
    }
    protected native long getRenderTexture(long ptr);
    protected native void addRenderTarget(long renderTarget, int eye, int index);
}

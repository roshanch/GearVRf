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
    private  GVRRenderTexture mPostEffectRenderTextureA;
    private  GVRRenderTexture mPostEffectRenderTextureB;
    private  GVRRenderTarget mEyeCaptureRenderTarget = null;
    private  GVRRenderTarget mDaydreamRenderTarget = null;
    private int  mSampleCount;

    private GVRRenderTarget[] mLeftEyeRenderTarget = new GVRRenderTarget[3];
    private GVRRenderTarget [] mRightEyeRenderTarget = new GVRRenderTarget[3];
    private GVRRenderTarget [] mMultiviewRenderTarget = new GVRRenderTarget[3];
    GVRRenderBundle(GVRContext gvrContext, final int width, final int height) {
        mGVRContext = gvrContext;
        mMaterialShaderManager = new GVRMaterialShaderManager(gvrContext);

        final VrAppSettings appSettings = mGVRContext.getActivity().getAppSettings();
        mSampleCount = appSettings.getEyeBufferParams().getMultiSamples() < 0 ? 0
                : appSettings.getEyeBufferParams().getMultiSamples();
        if (mSampleCount > 1) {
            int maxSampleCount = GVRMSAA.getMaxSampleCount();
            if (mSampleCount > maxSampleCount) {
                mSampleCount = maxSampleCount;
            }
        }

        if (mSampleCount <= 1) {
            mPostEffectRenderTextureA = new GVRRenderTexture(mGVRContext, width, height);
            mPostEffectRenderTextureB = new GVRRenderTexture(mGVRContext, width, height);
        } else {
            mPostEffectRenderTextureA = new GVRRenderTexture(mGVRContext, width, height, mSampleCount);
            mPostEffectRenderTextureB = new GVRRenderTexture(mGVRContext, width, height, mSampleCount);
        }
        boolean isMultiviewSet = mGVRContext.getActivity().getAppSettings().isMultiviewSet();
        for (int i = 0; i < 3; i++) {
            if(isMultiviewSet) {
                    GVRRenderTexture renderTexture = new GVRRenderTexture(mGVRContext, width, height, getRenderTexture(mGVRContext.getActivity().getNative(), GVRViewManager.EYE.MULTIVIEW.ordinal(), i));
                    mMultiviewRenderTarget[i] = new GVRRenderTarget(renderTexture,mGVRContext.getMainScene(),true);
            }
            else {
                mLeftEyeRenderTarget[i] = new GVRRenderTarget(new GVRRenderTexture(mGVRContext, width, height,
                        getRenderTexture(mGVRContext.getActivity().getNative(), GVRViewManager.EYE.LEFT.ordinal(), i)), mGVRContext.getMainScene());
                mRightEyeRenderTarget[i] = new GVRRenderTarget(new GVRRenderTexture(mGVRContext, width, height,
                        getRenderTexture(mGVRContext.getActivity().getNative(), GVRViewManager.EYE.RIGHT.ordinal(), i)), mGVRContext.getMainScene(),mLeftEyeRenderTarget[i] );
            }
        }
    }
    public GVRRenderTarget getDaydreamRenderTarget(){
        if(null == mDaydreamRenderTarget){
            mDaydreamRenderTarget = new GVRRenderTarget( mGVRContext);
        }
        return mDaydreamRenderTarget;
    }
    public GVRRenderTarget getEyeCaptureRenderTarget() {
        if(mEyeCaptureRenderTarget == null){
            int width = mGVRContext.getActivity().getAppSettings().getEyeBufferParams().getResolutionWidth();
            int height = mGVRContext.getActivity().getAppSettings().getEyeBufferParams().getResolutionHeight();
            // we will be using this render-target when multiview is enabled, so we can share the render-list of multiview-rendertarget with this
            mEyeCaptureRenderTarget  = new GVRRenderTarget(new GVRRenderTexture(mGVRContext, width, height, mSampleCount, 1), mGVRContext.getMainScene(), mMultiviewRenderTarget[0]);
        }
        return  mEyeCaptureRenderTarget;
    }
    public GVRRenderTarget getRenderTarget(GVRViewManager.EYE eye, int index){
        if(eye == GVRViewManager.EYE.LEFT)
            return mLeftEyeRenderTarget[index];
        if(eye == GVRViewManager.EYE.RIGHT)
            return mRightEyeRenderTarget[index];

        return mMultiviewRenderTarget[index];
    }
    public void updateMainScene(GVRScene scene){

        boolean isMultiviewSet = mGVRContext.getActivity().getAppSettings().isMultiviewSet();
        for (int i = 0; i < 3; i++) {
            if(isMultiviewSet) {
                mMultiviewRenderTarget[i].setMainScene(scene);
            }
            else {
                mLeftEyeRenderTarget[i].setMainScene(scene);
                mRightEyeRenderTarget[i].setMainScene(scene);
            }
        }
    }


    public GVRMaterialShaderManager getMaterialShaderManager() {
        return mMaterialShaderManager;
    }

    public GVRPostEffectShaderManager getPostEffectShaderManager() {
        return null;
    }

    public GVRRenderTexture getPostEffectRenderTextureA() {
        return mPostEffectRenderTextureA;
    }

    public GVRRenderTexture getPostEffectRenderTextureB() {
        return mPostEffectRenderTextureB;
    }
    protected native long getRenderTexture(long activity, int eye, int index);
}

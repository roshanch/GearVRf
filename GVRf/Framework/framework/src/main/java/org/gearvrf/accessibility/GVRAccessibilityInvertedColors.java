package org.gearvrf.accessibility;

/*
 * Copyright 2015 Samsung Electronics Co., LTD
 * 
 * Licensed under the Apache License, Version 2.0 (the "License"); you may not use this file except in compliance with the License. You may obtain a copy of the License at
 * 
 * http://www.apache.org/licenses/LICENSE-2.0
 * 
 * Unless required by applicable law or agreed to in writing, software distributed under the License is distributed on an "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied. See the License for the specific
 * language governing permissions and limitations under the License.
 */

import org.gearvrf.GVRContext;
import org.gearvrf.GVRMaterial;
import org.gearvrf.GVRShaderData;
import org.gearvrf.GVRScene;
import org.gearvrf.GVRShaderData;
import org.gearvrf.GVRShaderId;

public class GVRAccessibilityInvertedColors {

    private GVRMaterial postEffect;
    private boolean mInverted;
    private GVRContext mGvrContext;

    /**
     * Initialize {@link GVRShaderData}
     *
     * @param gvrContext
     */
    public GVRAccessibilityInvertedColors(final GVRContext gvrContext) {
        mGvrContext = gvrContext;
        GVRShaderId shaderId = gvrContext.getMaterialShaderManager().getShaderType(GVRAccessibilityPostEffectShader.class);
        postEffect = new GVRMaterial(gvrContext, shaderId);
    }

    public void turnOn(final GVRScene... scene) {
        mInverted = true;
        mGvrContext.runOnGlThread(new Runnable() {

            @Override
            public void run() {
                for (GVRScene gvrScene : scene) {
                    gvrScene.getMainCameraRig().getLeftCamera()
                            .addPostEffect(postEffect);
                    gvrScene.getMainCameraRig().getRightCamera()
                            .addPostEffect(postEffect);
                }
            }

        });

    }

    public void turnOff(final GVRScene... scene) {
        mInverted = false;
        mGvrContext.runOnGlThread(new Runnable() {

            @Override
            public void run() {
                for (GVRScene gvrScene : scene) {
                    gvrScene.getMainCameraRig().getLeftCamera()
                            .removePostEffect(postEffect);
                    gvrScene.getMainCameraRig().getRightCamera()
                            .removePostEffect(postEffect);
                }
            }

        });

    }

    public boolean isInverted() {
        return mInverted;
    }

}

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

import android.graphics.Bitmap;
import android.opengl.GLES30;
import android.opengl.GLUtils;

import org.gearvrf.GVRCompressedTexture;
import org.gearvrf.utility.Log;

import java.nio.ByteBuffer;
import java.util.List;
import java.util.concurrent.CountDownLatch;
import java.util.concurrent.ExecutionException;
import java.util.concurrent.Future;
import java.util.concurrent.TimeUnit;
import java.util.concurrent.TimeoutException;
import java.util.concurrent.locks.Condition;
import java.util.concurrent.locks.ReentrantLock;

import static android.opengl.GLES20.GL_LINEAR_MIPMAP_NEAREST;
import static android.opengl.GLES20.GL_NO_ERROR;
import static android.opengl.GLES20.GL_TEXTURE_2D;
import static android.opengl.GLES20.GL_TEXTURE_MIN_FILTER;
import static android.opengl.GLES20.glBindTexture;
import static android.opengl.GLES20.glGenerateMipmap;
import static android.opengl.GLES20.glGetError;
import static android.opengl.GLES20.glTexParameteri;

/** Wrapper for a GL texture. */
public class GVRTexture extends GVRHybridObject implements GVRAndroidResource.TextureCallback
{
    protected static final String TAG = "GVRTexture";
    protected GVRImage  mImage;
    protected GVRTextureParameters mTextureParams;
    private volatile int mTextureId;
    private final ReentrantLock mLock;
    public String mTexCoordAttr;
    public String mShaderVar;

    public GVRTexture(GVRContext gvrContext)
    {
        super(gvrContext, NativeTexture.constructor());
        mImage = null;
        mLock = new ReentrantLock();
        mTextureParams = null;
        mTextureId = 0;
    }

    protected GVRTexture(GVRContext gvrContext, long ptr)
    {
        super(gvrContext, ptr);
        mImage = null;
        mLock = new ReentrantLock();
        mTextureParams = null;
        mTextureId = 0;
    }

    public GVRTexture(GVRContext gvrContext, GVRTextureParameters texparams)
    {
        super(gvrContext, NativeTexture.constructor());
        mImage = null;
        mTextureId = 0;
        mLock = new ReentrantLock();
        updateTextureParameters(texparams);
    }

    public boolean stillWanted(GVRAndroidResource r)
    {
        return true;
    }

    public void loaded(GVRImage image, GVRAndroidResource resource)
    {
        String fname = image.getFileName();
        if ((fname == null) || fname.isEmpty() && (resource != null))
        {
            image.setFileName(resource.getResourceFilename());
        }
        setImage(image);
    }

    public void failed(Throwable ex, GVRAndroidResource resource) { }

    public int getId()
    {
        if (mTextureId != 0)
        {
            return mTextureId;
        }

        final CountDownLatch cdl = new CountDownLatch(1);
        getGVRContext().runOnGlThread(new Runnable() {
            @Override
            public void run() {
                NativeTexture.isReady(getNative());
                mTextureId = NativeTexture.getId(getNative());
                cdl.countDown();
            }
        });
        try
        {
            cdl.await();
        }
        catch (final Exception exc)
        {
            throw new IllegalStateException("Exception waiting for texture ready");
        }
        return mTextureId;
    }


    /**
     * Update the texture parameters {@link GVRTextureParameters} after the
     * texture has been created.
     */
    public void updateTextureParameters(GVRTextureParameters textureParameters)
    {
        mTextureParams = textureParameters;
        long nativePtr = getNative();
        if (nativePtr != 0)
        {
            NativeTexture.updateTextureParameters(nativePtr, textureParameters.getCurrentValuesArray());
        }
    }

    /**
     * Returns the list of atlas information necessary to map
     * the texture atlas to each scene object.
     *
     * @return List of atlas information.
     */
    public List<GVRAtlasInformation> getAtlasInformation()
    {
        if ((mImage != null) && (mImage instanceof GVRImageAtlas))
        {
            return ((GVRImageAtlas) mImage).getAtlasInformation();
        }
        return null;
    }

    /**
     * Set the list of {@link GVRAtlasInformation} to map the texture atlas
     * to each object of the scene.
     *
     * @param atlasInformation Atlas information to map the texture atlas to each
     *        scene object.
     */
    public void setAtlasInformation(List<GVRAtlasInformation> atlasInformation)
    {
        if ((mImage != null) && (mImage instanceof GVRImageAtlas))
        {
            ((GVRImageAtlas) mImage).setAtlasInformation(atlasInformation);
        }
     }

    /**
     * Inform if the texture is a large image containing "atlas" of sub-images
     * with a list of {@link GVRAtlasInformation} necessary to map it to the
     * scene objects.
     *
     * @return True if the texture is a large image containing "atlas",
     *         otherwise it returns false.
     */
    public boolean isAtlasedTexture()
    {
        return (mImage != null) &&
                (mImage instanceof GVRImageAtlas) &&
                ((GVRImageAtlas) mImage).isAtlasedTexture();
    }

    /**
     * Changes the image data associated with a GVRTexture.
     * This can be a simple bitmap, a compressed bitmap,
     * a cubemap or a compressed cubemap.
     * @param imageData data for the texture as a GVRImate
     */
    public void setImage(final GVRImage imageData)
    {
        mImage = imageData;
        if (imageData != null)
            NativeTexture.setImage(getNative(), imageData, imageData.getNative());
        else
            NativeTexture.setImage(getNative(), null, 0);
    }

    public GVRImage getImage()
    {
        return mImage;
    }


    /**
     * Designate the vertex attribute and shader variable for the texture coordinates
     * associated with this texture.
     *
     * @param texCoordAttr  name of vertex attribute with texture coordinates.
     * @param shaderVarName name of shader variable to get texture coordinates.
     */
    public void setTexCoord(String texCoordAttr, String shaderVarName)
    {
        mTexCoordAttr = texCoordAttr;
        mShaderVar = shaderVarName;
    }

    /**
     * Gets the name of the vertex attribute containing the texture
     * coordinates for this texture.
     *
     * @return name of texture coordinate vertex attribute
     */
    public String getTexCoordAttr()
    {
        return mTexCoordAttr;
    }


    /**
     * Gets the name of the shader variable to get the texture
     * coordinates for this texture.
     *
     * @return name of shader variable
     */
    public String getTexCoordShaderVar()
    {
        return mShaderVar;
    }

}

class NativeTexture {
    static native long constructor();
    static native int getId(long texture);
    static native boolean isReady(long texture);
    static native void updateTextureParameters(long texture, int[] textureParametersValues);
    static native void setImage(long texPointer, GVRImage javeImage, long nativeImage);
}

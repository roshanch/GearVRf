/***************************************************************************
 * installable external renderer
 ***************************************************************************/

#ifndef EXTERNAL_RENDERER_SHADER_H_
#define EXTERNAL_RENDERER_SHADER_H_

typedef void (*GVRF_ExternalRenderer)(long data,
                                      const float* bounding_volume, int vcount,
                                      const float* projection, int pcount,
                                      const float* texCoords, int tcount,
                                      float opacity);
extern "C" void GVRF_installExternalRenderer(GVRF_ExternalRenderer renderer);

namespace gvr {
class RenderState;
class RenderData;
class ShaderData;
/*
 * TODO: Figure out how this fits in now. It is not really a shader.
 * It doesn't have a program ID. It has logic none of the other shaders have.
 */
class ExternalRendererShader
{
public:
    explicit ExternalRendererShader(long id) {}
    virtual ~ExternalRendererShader() {}
    virtual void render(RenderState* rstate, RenderData* render_data, ShaderData* material);

private:
    ExternalRendererShader(
            const ExternalRendererShader& shader) = delete;
    ExternalRendererShader(
            ExternalRendererShader&& shader) = delete;
    ExternalRendererShader& operator=(
            const ExternalRendererShader& shader) = delete;
    ExternalRendererShader& operator=(
            ExternalRendererShader&& shader) = delete;

    float scratchBuffer[6];
};

}

#endif

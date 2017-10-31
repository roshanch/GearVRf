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
 * Camera for scene rendering.
 ***************************************************************************/

#ifndef CAMERA_H_
#define CAMERA_H_

#include <vector>

#include "glm/glm.hpp"

#include "objects/components/component.h"

namespace gvr {
class ShaderData;
class RenderData;

class Camera: public Component {
public:
    Camera();
    virtual ~Camera();

    float background_color_r() const {
        return background_color_r_;
    }

    void set_background_color_r(float r) {
        background_color_r_ = r;
    }

    float background_color_g() const {
        return background_color_g_;
    }

    void set_background_color_g(float g) {
        background_color_g_ = g;
    }

    float background_color_b() const {
        return background_color_b_;
    }

    void set_background_color_b(float b) {
        background_color_b_ = b;
    }

    float background_color_a() const {
        return background_color_a_;
    }

    void set_background_color_a(float a) {
        background_color_a_ = a;
    }

    int render_mask() const {
        return render_mask_;
    }

    void set_render_mask(int render_mask) {
        render_mask_ = render_mask;
    }

    RenderData* post_effect_data() const {
        return post_effect_data_;
    }

    void setPostEffect(RenderData* post_effects);
    virtual glm::mat4 getProjectionMatrix() const = 0;
    virtual const glm::mat4& getViewMatrix();
    virtual void setViewMatrix(const glm::mat4& viewMtx);
    static long long getComponentType() {
        return COMPONENT_TYPE_CAMERA;
    }
private:
    Camera(const Camera& camera) = delete;
    Camera(Camera&& camera) = delete;
    Camera& operator=(const Camera& camera) = delete;
    Camera& operator=(Camera&& camera) = delete;

private:
    float background_color_r_;
    float background_color_g_;
    float background_color_b_;
    float background_color_a_;
    int render_mask_;
    RenderData* post_effect_data_;
    glm::mat4 view_matrix_;
};

}
#endif

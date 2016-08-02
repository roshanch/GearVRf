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
#include "batch.h"
#include "objects/scene_object.h"
#include "objects/components/camera.h"
#include "objects/components/render_data.h"
#include "objects/light.h"

#define BATCH_SIZE 60

namespace gvr {
Batch::Batch(int no_vertices, int no_indices) :
        draw_count_(0), vertex_count_(0), index_count_(0), vertex_limit_(no_vertices),
        indices_limit_(no_indices), renderdata_(nullptr),mesh_init_(false),
        index_offset_(0), not_batched_(false) {

    vertices_.reserve(no_vertices);
    tex_coords_.reserve(no_vertices);
    indices_.reserve(no_indices);
    normals_.reserve(no_vertices);
    matrix_indices_.reserve(no_vertices);
}

Batch::~Batch() {
    vertices_.clear();
    normals_.clear();
    tex_coords_.clear();
    indices_.clear();
    matrices_.clear();
    matrix_indices_.clear();
    delete renderdata_;

}
bool Batch::updateMesh(Mesh* render_mesh){
    const std::vector<unsigned short>& indices = render_mesh->indices();
        const std::vector<glm::vec3>& vertices = render_mesh->vertices();
        const std::vector<glm::vec3>& normals = render_mesh->normals();
        const std::vector<glm::vec2>& tex_cords = render_mesh->tex_coords(int(material_->getFloat("uvIndex")));
        std::map<std::string, std::vector<float>>& float_vectors     = render_mesh->getFloatVectors();
        std::map<std::string, std::vector<glm::vec2>>& vec2_vectors = render_mesh->getVec2Vectors();
        std::map<std::string, std::vector<glm::vec3>>& vec3_vectors = render_mesh->getVec3Vectors();
        std::map<std::string, std::vector<glm::vec4>>& vec4_vectors = render_mesh->getVec4Vectors();
        LOGE("size of v: %d , T %d N %d",vertices.size(),tex_cords.size(),normals.size());
        int size = 0;
        size = vertices.size();

        for(int i=0;i<size;i++){
            vertices_.push_back(vertices[i]);
            matrix_indices_.push_back(draw_count_);
        }


        // Check if models has normals and texCords
        if(normals.size() > 0){
            for(int i=0;i<size;i++)
                normals_.push_back(normals[i]);
        }

        if(tex_cords.size() > 0){
            for(int i=0;i<size;i++)
                tex_coords_.push_back(tex_cords[i]);
        }

        for(auto& it: float_vectors){
            auto it1 = float_vectors_.find(it.first);
            if(it1 !=float_vectors_.end()){
                std::vector<float>& float_vector = float_vectors_[it.first];
                int vec_size = it.second.size();
                for(int i =0 ;i<vec_size; i++){
                    float_vector.push_back((it.second)[i]);
                }
            }
            else {
                auto& float_vector = it.second;
                std::vector<float> new_float_vector;
                int vec_size = it.second.size();
                for(int i =0 ;i<vec_size; i++){
                    new_float_vector.push_back(float_vector[i]);
                }
               float_vectors_[it.first] = new_float_vector;
            }
        }

        for(auto& it: vec2_vectors){
            auto it2 = vec2_vectors_.find(it.first);
            if(it2 !=vec2_vectors_.end()){
                std::vector<glm::vec2>& vec2_vector = vec2_vectors_[it.first];
                int vec_size = it.second.size();
                for(int i =0 ;i<vec_size; i++){
                    vec2_vector.push_back((it.second)[i]);
                }
            }
            else {
                auto& vec2_vector = it.second;
                std::vector<glm::vec2> new_vec2_vector;
                int vec_size = it.second.size();
                for(int i =0 ;i<vec_size; i++){
                    new_vec2_vector.push_back(vec2_vector[i]);
                }
               vec2_vectors_[it.first] = new_vec2_vector;
            }
        }

        for(auto& it: vec3_vectors){
            auto it2 = vec3_vectors_.find(it.first);
            if(it2 !=vec3_vectors_.end()){
                std::vector<glm::vec3>& vec3_vector = vec3_vectors_[it.first];
                int vec_size = it.second.size();
                for(int i =0 ;i<vec_size; i++){
                    vec3_vector.push_back((it.second)[i]);
                }
            }
            else {
                auto& vec3_vector = it.second;
                std::vector<glm::vec3> new_vec3_vector;
                int vec_size = it.second.size();
                for(int i =0 ;i<vec_size; i++){
                    new_vec3_vector.push_back(vec3_vector[i]);
                }
               vec3_vectors_[it.first] = new_vec3_vector;
            }
        }

        for(auto& it: vec4_vectors){
            auto it2 = vec4_vectors_.find(it.first);
            if(it2 !=vec4_vectors_.end()){
                std::vector<glm::vec4>& vec4_vector = vec4_vectors_[it.first];
                int vec_size = it.second.size();
                for(int i =0 ;i<vec_size; i++){
                    vec4_vector.push_back((it.second)[i]);
                }
            }
            else {
                auto& vec4_vector = it.second;
                std::vector<glm::vec4> new_vec4_vector;
                int vec_size = it.second.size();
                for(int i =0 ;i<vec_size; i++){
                    new_vec4_vector.push_back(vec4_vector[i]);
                }
               vec4_vectors_[it.first] = new_vec4_vector;
            }
        }
        size = indices.size();
        index_count_+=size;
        for (int i = 0; i < size; i++) {
            unsigned short index = indices[i];
            index += index_offset_;
            indices_.push_back(index);
        }
    LOGE("added to batch");
        // update all VBO data
        vertex_count_ += vertices.size();
        index_offset_ += vertices.size();

}
/*
 * Add renderdata of scene object into mesh, add vertices, texcoords, normals, model matrices
 */
bool Batch::add(RenderData *render_data) {
    material_ = render_data->pass(0)->material();
    Mesh *render_mesh = render_data->mesh();
    const std::vector<unsigned short>& indices = render_mesh->indices();

    Transform* const t = render_data->owner_object()->transform();
    glm::mat4 model_matrix;
    if (t != NULL) {
        model_matrix = glm::mat4(t->getModelMatrix());
    }

    // Store the model matrix and its index into map for update
    matrix_index_map_[render_data] = draw_count_;
    matrices_.push_back(model_matrix);
    render_data->owner_object()->setTransformUnDirty();

    // if it is not texture shader, dont add into batch, render in normal way
    if (material_->shader_type() != Material::ShaderType::TEXTURE_SHADER && !isCustomShader(material_)) {
        render_data_set_.insert(render_data);
        LOGE("not a valid shader");
        render_mesh->setMeshModified(false); // mark mesh clean
        return true;
    }

    // if mesh is large, render in normal way
    if (indices.size() + index_count_ > indices_limit_) {
        if (draw_count_ > 0) {
            return false;
        } else {
            LOGE("mesh is large %d", indices.size());
            render_data_set_.insert(render_data);
            not_batched_ = true;
            render_mesh->setMeshModified(false); // mark mesh clean
            return true;
        }
    }
    // Copy all renderData properties
    if (draw_count_ == 0) {
        if (!renderdata_) {
            renderdata_ = new RenderData(*render_data);
            renderdata_->set_batching(true);
       }

    }

    render_data_set_.insert(render_data); // store all the renderdata which are in batch
    render_mesh->setMeshModified(false); // mark mesh clean

    updateMesh(render_mesh);
    draw_count_++;
    mesh_init_ = false;

    return true;
}

void Batch::setupMesh(Material* mat){
    if(!mesh_init_){
        mesh_init_ = true;

        mesh_.set_vertices(vertices_);
        if(normals_.size() > 0)
            mesh_.set_normals(normals_);

        mesh_.setFloatVecMap(float_vectors_);
        mesh_.setVec2Map(vec2_vectors_);
        mesh_.setVec3Map(vec3_vectors_);
        mesh_.setVec4Map(vec4_vectors_);

        int uvIndex = int(mat->getFloat("uvIndex"));
        if(tex_coords_.size() > 0)
            mesh_.set_tex_coords(tex_coords_,uvIndex);

        mesh_.set_indices(indices_);
        mesh_.setFloatVector("a_matrix_index", matrix_indices_);
        mesh_.setCurrentUVIndex(uvIndex);
        renderdata_->set_mesh(&mesh_);

    }
}
/*
 *  Check if any of the meshes in batch are modified
 */
bool Batch::isBatchDirty() {
    for (auto it = render_data_set_.begin(); it != render_data_set_.end();
            it++) {
        if ((*it)->mesh()->isMeshModified())
            return true;
    }
    return false;
}

/*
 * Set batch in render data to null. can be use to mark dirty.
 */
void Batch::setMeshesDirty() {
    for (auto it = render_data_set_.begin(); it != render_data_set_.end();
            it++) {
        (*it)->setBatchNull();
    }
}

}

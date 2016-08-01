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
#ifndef BATCH_MANAGER_H
#define BATCH_MANAGER_H

#include "objects/mesh.h"
#include <map>
#include "objects/components/render_data.h"
#include "renderer.h"
#include <unordered_map>
#include <memory>
#include <vector>
#include <string>
#include "batch.h"
#include <set>
#include<unordered_set>
namespace gvr{
class RenderData;
class Material;
class Mesh;
class Batch;
struct RenderState;
class BatchManager{

public:
    BatchManager(int batch_size, int max_indices){
        batch_size_ = batch_size;
        max_indices_ = max_indices;
    }
    void batchSetup(std::vector<RenderData*>& render_data_vector);
    void renderBatches(RenderState& rstate);

private:
/*
 * batch_set stores all the batches, batch_map stores the indices of batches and the respective map
 * to get constant look up time
 */
    std::vector<Batch*> batch_set_;
    std::unordered_map<Batch*, int> batch_map_;
    int max_indices_;
    int batch_size_;
    /*
     * batch indices stores the indices in render_vector where batches are split
     */
    std::vector<int> batch_indices_;
    struct comparator {
        std::string renderdata_properties;
        Material* mat;
        Material::ShaderType shader_type;
        bool mesh_dynamic;
    };
    void clearBatchSet(){
         batch_set_.clear(); // Clear batch vector
         batch_map_.clear();
    }
    void getNewBatch(RenderData* rdata, Batch** existing_batch);
    void createBatch(int start, int end, std::vector<RenderData*>& render_data_vector);

};
}
#endif // BATCH_MANAGER_H

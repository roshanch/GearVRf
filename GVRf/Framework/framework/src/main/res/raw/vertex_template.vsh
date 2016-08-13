
#define HAS_BATCHING
#ifdef HAS_MULTIVIEW
#extension GL_OVR_multiview2 : enable
layout(num_views = 2) in;
flat out int view_id;
#endif

layout (std140) uniform Transform_UBO {
#ifdef HAS_BATCHING
	mat4 u_model[60];
#else
	mat4 u_model;
#endif
	 mat4 u_proj;
	 mat4 u_view[2];
};

layout (std140) uniform Material_UBO {
	 vec4 ambient_color;
	 vec4 diffuse_color;
	 vec4 specular_color;
	 vec4 emissive_color;
	 float specular_exponent;

};

layout (std140) uniform Bones_UBO 
{
	mat4 u_bone_matrix[60];

};

in vec3 a_position;
in vec2 a_texcoord;
in vec3 a_normal;

#ifdef HAS_BATCHING
in float a_matrix_index;
#endif

#ifdef HAS_VertexSkinShader
in vec4 a_bone_weights;
in ivec4 a_bone_indices;
#endif

#ifdef HAS_VertexNormalShader
in vec3 a_tangent;
in vec3 a_bitangent;
#endif

out vec2 diffuse_coord;
out vec3 view_direction;
out vec3 viewspace_position;
out vec3 viewspace_normal;
out vec4 local_position;

struct Vertex
{
	vec4 local_position;
	vec4 local_normal;
	vec3 viewspace_position;
	vec3 viewspace_normal;
	vec3 view_direction;
};

#ifdef HAS_LIGHTSOURCES
	@LIGHTSOURCES
#endif
	
void main() {
	Vertex vertex;

	vertex.local_position = vec4(a_position.xyz, 1.0);
	vertex.local_normal = vec4(0.0, 0.0, 1.0, 0.0);
	mat4 model_matrix;

mat4 view_matrix;
#ifdef HAS_MULTIVIEW
	view_id = int(gl_ViewID_OVR);
    view_matrix = u_view[gl_ViewID_OVR];
#else
    view_matrix = u_view[0];    
#endif

#ifdef HAS_BATCHING
	int index =int(a_matrix_index);
    model_matrix = u_model[index];
#else
	 model_matrix = u_model;
#endif
mat4 mv = view_matrix * model_matrix;

	@VertexShader
#ifdef HAS_VertexSkinShader
	@VertexSkinShader
#endif
#ifdef HAS_VertexNormalShader
	@VertexNormalShader
#endif
#ifdef HAS_LIGHTSOURCES
	LightVertex(vertex);
#endif

	viewspace_position = vertex.viewspace_position;
	viewspace_normal = vertex.viewspace_normal;
	view_direction = vertex.view_direction;

	gl_Position = u_proj * mv * vertex.local_position;

}
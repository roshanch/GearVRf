
layout (std140) uniform Material_UBO {
	 mat4 u_model;
	 mat4 u_mvp[2];
	 mat4 u_view[2];
	 mat4 u_mv[2];
	 mat4 u_mv_it[2];
	 vec4 ambient_color;
	 vec4 diffuse_color;
	 vec4 specular_color;
	 vec4 emissive_color;
	 float specular_exponent;

};
uniform mat4 shadow_matrix;

#ifdef HAS_MULTIVIEW
#extension GL_OVR_multiview2 : enable
layout(num_views = 2) in;
flat out int view_id;
#endif


in vec3 a_position;
in vec4 a_bone_weights;
in ivec4 a_bone_indices;
out vec4 local_position;
out vec4 proj_position;
struct Vertex
{
	vec4 local_position;
};

void main() {
	Vertex vertex;

	vertex.local_position = vec4(a_position.xyz, 1.0);
#ifdef HAS_MULTIVIEW
	proj_position = u_mvp[gl_ViewID_OVR] * vertex.local_position;
	view_id = int(gl_ViewID_OVR);
#else
	proj_position = u_mvp[0] * vertex.local_position;
#endif	

	gl_Position = proj_position;
}
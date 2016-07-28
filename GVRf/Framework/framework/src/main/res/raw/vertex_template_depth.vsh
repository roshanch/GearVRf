
layout (std140) uniform Material_UBO {
#ifdef HAS_BATCHING
	 mat4 u_model[60];
#else
	 mat4 u_model;
#ifdef	 
	 mat4 u_proj;
	 mat4 u_view[2];
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

#ifdef HAS_BATCHING
in float a_matrix_index;
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

	
	gl_Position = u_proj* mv * vertex.local_position;
}
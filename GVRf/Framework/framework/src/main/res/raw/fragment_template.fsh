
#define HAS_BATCHING
#ifdef HAS_MULTIVIEW
flat in int view_id;
#endif

precision highp float;
precision highp sampler2DArray;

layout (std140) uniform Material_UBO {
#ifdef HAS_BATCHING
	mat4 u_model[60]; 
#else
	mat4 u_model; 
#endif	
	 mat4 u_proj;
	 mat4 u_view[2];
	 vec4 ambient_color;
	 vec4 diffuse_color;
	 vec4 specular_color;
	 vec4 emissive_color;
	 float specular_exponent;

};

in vec3 viewspace_position;
in vec3 viewspace_normal;
in vec4 local_position;
in vec4 proj_position;
in vec2 diffuse_coord;
in vec3 view_direction;
out vec4 fragColor;

#ifdef HAS_SHADOWS
uniform sampler2DArray u_shadow_maps;
#endif

struct Radiance
{
   vec3 ambient_intensity;
   vec3 diffuse_intensity;
   vec3 specular_intensity;
   vec3 direction; // view space direction from light to surface
   float attenuation;
};

@FragmentSurface

@FragmentAddLight

@LIGHTSOURCES

void main()
{
	Surface s = @ShaderName();
#if defined(HAS_LIGHTSOURCES)
	vec4 color = LightPixel(s);
	color = clamp(color, vec4(0), vec4(1));
	fragColor = color;
#else
	fragColor = s.diffuse;
	//fragColor = vec4(1,0,0,1);
#endif
}

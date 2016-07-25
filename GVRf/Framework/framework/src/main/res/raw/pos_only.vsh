#ifdef HAS_MULTIVIEW
vec4 pos = u_mv[gl_ViewID_OVR] * vertex.local_position;
#else
vec4 pos = u_mv[0] * vertex.local_position;
#endif

vertex.viewspace_position = pos.xyz / pos.w;
vertex.view_direction = normalize(-vertex.viewspace_position);

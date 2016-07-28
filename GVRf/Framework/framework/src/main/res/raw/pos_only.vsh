
vec4 pos =  mv * vertex.local_position;	
vertex.viewspace_position = pos.xyz / pos.w;
vertex.view_direction = normalize(-vertex.viewspace_position);

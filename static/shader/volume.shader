<Layout>
	version = 420
	pushsize = 128
	input = [vec3,vec3,vec2]
	topology = triangles
</Layout>
<FragmentShader>
#import surface

layout(binding=0) uniform sampler3D tex3d;

#ifdef vulkan
layout(push_constant) uniform User {
	vec4 map_colors[4];
	float map_values[4];
	int map_count;
};
#else
uniform vec4 map_colors[4];
uniform float map_values[4];
uniform int map_count;
#endif

vec4 color_map(float f) {
	for (int i=0; i<map_count-1; i++)
		if (f >= map_values[i] && f <= map_values[i+1])
			return mix(map_colors[i], map_colors[i+1], (f - map_values[i]) / (map_values[i+1] - map_values[i]));
	if (f < map_values[0])
		return map_colors[0];
	return map_colors[map_count-1];
}

void main() {
	// Mesh
	vec3 tmp1 = in_pos.xyz / in_pos.w;
	vec3 tmp2 = normalize(in_normal);
	vec2 tmp3 = in_uv;
	// Material
	vec4 tmp4 = material.albedo;
	float tmp5 = material.roughness;
	float tmp6 = material.metal;
	vec4 tmp7 = material.emission;
	
	vec3 cam_pos0 = (inverse(matrix.view * matrix.model) * vec4(0,0,0,1)).xyz;
	vec3 p0 = (inverse(matrix.view * matrix.model) * vec4(tmp1,1)).xyz;
	vec3 dir0 = normalize(p0 - cam_pos0);
	
	vec4 acc = vec4(0,0,0,0);
	
	float dt = 0.03;
	for (int i=0; i<30; i++) {
		// scalar field value
		float f = texture(tex3d, p0).r;
		vec4 c = color_map(f);
		acc.xyz += (1-acc.a) * c.xyz * dt * 5;
		acc.a += (1-acc.a) * c.a * dt * 5;
		
	
		p0 += dir0 * dt;
		if (p0.x<0 || p0.y<0 || p0.z<0 || p0.x>1 || p0.y>1 || p0.z>1 || acc.a>0.95)
			break;
	}
	
	// SurfaceOutput
	//surface_out(tmp2, tmp9, tmp7, tmp6, tmp5);
	out_color = acc;
}
</FragmentShader>

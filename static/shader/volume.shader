<Layout>
	version = 420
	pushsize = 0
	input = [vec3,vec3,vec2]
	topology = triangles
</Layout>
<VertexShader>

// custom vertex shader for forwarding "normalized" model-space coordinates

struct Matrices {
	mat4 model;
	mat4 view;
	mat4 project;
};

#ifdef vulkan
layout(binding = 8) uniform Parameters {
	Matrices matrix;
};
#else
/*layout(binding = 0)*/ uniform Matrices matrix;
#endif

layout(location = 0) in vec3 in_position;
layout(location = 1) in vec3 in_normal;
layout(location = 2) in vec2 in_uv;

layout(location = 0) out vec4 out_pos; // view space
layout(location = 1) out vec3 out_normal;
layout(location = 2) out vec2 out_uv;
layout(location = 3) out vec4 out_color; // optional
layout(location = 4) out vec3 out_pos0; // model space

void main() {
	gl_Position = matrix.project * matrix.view * matrix.model * vec4(in_position, 1);
	out_normal = (matrix.view * matrix.model * vec4(in_normal, 0)).xyz;
	//out_normal = (matrix.model * vec4(in_normal, 0)).xyz;
	out_uv = in_uv;
	out_pos = matrix.view * matrix.model * vec4(in_position, 1);
	//out_pos = matrix.model * vec4(in_position, 1);
	out_color = vec4(1);
	out_pos0 = in_position;
}

</VertexShader>
<FragmentShader>
#import surface

layout(location=4) in vec3 in_pos0; // model space
layout(binding=0) uniform sampler3D tex3d;

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
	
	// scalar field value
	float f = texture(tex3d, in_pos0).r;
	
	vec4 tmp9 = vec4(0,0,0,1);
	if (f < 0)
		tmp9 = mix(vec4(0,0,1,1), vec4(1,1,1,1), f+1);
	else
		tmp9 = mix(vec4(1,1,1,1), vec4(1,0,0,1), f);
	
	// SurfaceOutput
	surface_out(tmp2, tmp9, tmp7, tmp6, tmp5);
	//out_color = vec4(in_uv,0,1);
}
</FragmentShader>

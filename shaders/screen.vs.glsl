#version 330

in vec3 in_position;
uniform mat3 transform;

out vec2 texcoord;

void main()
{
	vec3 pos = transform * vec3(in_position.xy, 1.0);
	//vec3 pos =  vec3(in_position.xy, 1.0);
    gl_Position = vec4(pos.xy, 0, 1.0);
	texcoord = (in_position.xy + 1) / 2.f;
}

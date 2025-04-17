#version 330

// Input attributes
in vec3 in_position;
in vec2 in_texcoord;
in mat2 basis;
in vec2 offset;
in mat2 section; // section[0][0] is left, sections[0][1] is top, sections[1][0] is right, sctions [1][1] is bottom
in vec4 modulate;

// Passed to fragment shader
out vec2 texcoord;
out vec4 fmodulate;

// Application data
uniform mat3 projection;
uniform vec2 textureSize; 
uniform vec2 anchor;
uniform mat3 overallTransform;


#ifdef OUTLINE

out vec2 ftextureSize;

#endif

void main()
{
	vec2 texcoord_scale = vec2(section[1][0] - section[0][0], section[1][1] - section[0][1]);
	vec2 texcoord_offset = vec2(section[0][0], section[0][1]);
	texcoord = (in_texcoord * texcoord_scale + texcoord_offset);

	vec2 scale = textureSize * texcoord_scale;
	mat2 scaleBasis = mat2(scale.x, 0, 0, scale.y);
	
	mat2 finalBasis = basis * scaleBasis;
	vec3 pos = projection * overallTransform * vec3(finalBasis * (in_position.xy - anchor) + offset, 1.0);
	gl_Position = vec4(pos.xy, in_position.z, 1.0);

	fmodulate = modulate;

	#ifdef OUTLINE

	ftextureSize = textureSize;

	#endif
}


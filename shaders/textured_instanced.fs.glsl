#version 330

// From vertex shader
in vec2 texcoord;
in vec4 fmodulate;

// Application data
uniform sampler2D sampler0;
uniform vec4 overallModulate;

// Output color
layout(location = 0) out  vec4 color;


#ifdef OUTLINE

uniform float outlineSize;
uniform vec4 outlineColor;
in vec2 ftextureSize;

#endif

void main()
{
	color = overallModulate * fmodulate * texture(sampler0, vec2(texcoord.x, texcoord.y));

	#ifdef OUTLINE

	float outlineStrength = 0.0f;
	vec2 pixelDx = vec2(1.0f, 1.0f) / ftextureSize;

	

	for (float dx = -ceil(outlineSize); dx <= ceil(outlineSize); dx += 1.0f) {
	for (float dy = -ceil(outlineSize); dy <= ceil(outlineSize); dy += 1.0f) {
		if (dx * dx + dy * dy > outlineSize * outlineSize) continue;
		if (texture(sampler0, vec2(dx, dy) * pixelDx + texcoord).a == 0.0f) continue;
		outlineStrength = 1.0f;	
		
	}}


	outlineStrength *= (1.0f - color.a);

	color = mix(color, outlineColor, outlineStrength);

	#endif
}

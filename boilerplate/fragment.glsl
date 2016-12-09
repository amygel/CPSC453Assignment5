// ==========================================================================
// Vertex program for barebones GLFW boilerplate
//
// Author:  Sonny Chan, University of Calgary
// Date:    December 2015
// ==========================================================================
#version 410

// interpolated colour received from vertex stage
in vec3 Normal;

// first output is mapped to the framebuffer's colour index by default
out vec4 FragmentColour;

uniform sampler2D tex;
float PI = 3.1459;

void main(void)
{
	float x = atan(Normal[0], Normal[2]) / (2.0f * PI) + 0.5f;
	float y = asin(Normal[1])/ PI + 0.5f;

    // write colour output without modification
    FragmentColour = texture(tex, vec2(x,y));
}

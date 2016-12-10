// ==========================================================================
// Vertex program for barebones GLFW boilerplate
//
// Author:  Sonny Chan, University of Calgary
// Date:    December 2015
// ==========================================================================
#version 410

in vec3 Normal; // Surface normal in world space.
in vec3 VertNormal; // Normal in object space
in vec3 Position; // Position in world space.

uniform vec3 light; // Light's position in world space.
uniform bool isShaded;

out vec4 FragmentColour;

uniform sampler2D tex;
float PI = 3.1459;

void main(void)
{
    // Compute the diffuse term.
    vec3 L = normalize(light - Position);
    float diffuse = max(dot(Normal, L), 0);
    
	// Compute surface colour
	float x = atan(VertNormal.x, VertNormal.z) / (2.0f * PI) + 0.5f;
	float y = asin(VertNormal.y)/ PI + 0.5f;

    FragmentColour = texture(tex, vec2(x, y));

	if(isShaded)
	{
		FragmentColour = FragmentColour * (0.3f + diffuse);
	}
}
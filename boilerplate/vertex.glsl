// ==========================================================================
// Vertex program for barebones GLFW boilerplate
//
// Author:  Sonny Chan, University of Calgary
// Date:    December 2015
// ==========================================================================
#version 410

// location indices for these attributes correspond to those specified in the
// InitializeGeometry() function of the main program
layout(location = 0) in vec3 VertexPosition;
layout(location = 1) in vec3 VertexNormal;

// output to be interpolated between vertices and passed to the fragment stage
out vec3 Normal;
out vec3 VertNormal;
out vec3 Position;

// uniforms
uniform mat4 model;
uniform mat4 view;
uniform mat4 proj;

void main()
{
    // transformations applied right to left, order matters
    gl_Position = proj*view*model*vec4(VertexPosition, 1.0);

	Normal = normalize(model*vec4(VertexNormal,0)).xyz;
	VertNormal = VertexNormal;
	Position = (model * vec4(Normal, 1)).xyz; 
}

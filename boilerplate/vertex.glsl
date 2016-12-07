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
layout(location = 1) in vec3 VertexColour;

// output to be interpolated between vertices and passed to the fragment stage
out vec3 Colour;

uniform mat4 model;
uniform mat4 view;
uniform mat4 proj;

void main()
{
    // assign vertex position without modification
    // transformations applied right to left, order matters
    gl_Position = proj*view*model*vec4(VertexPosition, 1.0);

    // assign output colour to be interpolated
    Colour = VertexColour;
}

#version 330

// Input vertex attributes
in vec3 vertexPosition;
in vec2 barycentric;

// Input uniform values
uniform mat4 mvp;

// Output vertex attributes (to fragment shader)
out vec3 vbc;

void main() {
  vbc.xy = barycentric.xy;
  vbc.z = 1.0 - barycentric.x - barycentric.y;
  gl_Position = mvp * vec4(vertexPosition, 1.0);
}
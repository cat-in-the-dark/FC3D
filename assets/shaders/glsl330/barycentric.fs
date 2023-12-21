#version 330

precision mediump float;

in vec3 vbc;
out vec4 finalColor;

const float lineWidth = 1.0;
// const vec3 color = vec3(0.7, 0.7, 0.7);

// This function is not currently used, but it can be useful
// to achieve a fixed width wireframe regardless of z-depth
float computeScreenSpaceWireframe (vec3 barycentric, float width) {
  vec3 dist = fwidth(barycentric);
  vec3 smoothed = smoothstep(dist * ((width * 0.5) - 0.5), dist * ((width * 0.5) + 0.5), barycentric);
  return 1.0 - min(min(smoothed.x, smoothed.y), smoothed.z);
}

void main() {
  float w = computeScreenSpaceWireframe(vbc, lineWidth);

  finalColor = vec4(0.0, w, 0.0, 1.0);
}
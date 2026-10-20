#version 330

// Input vertex attributes (from vertex shader)
in vec3 fragPosition;
in vec3 fragNormal;
in vec4 fragColor;

// Input uniform values
uniform sampler2D texture0;
uniform vec4 colDiffuse;
uniform float texScale;

// Output fragment color
out vec4 finalColor;

void main()
{
    // Compute blending weights based on surface normal orientation
    vec3 blending = abs(fragNormal);
    blending = pow(blending, vec3(6.0)); // Crisp transitions between projections
    blending /= (blending.x + blending.y + blending.z);

    // Project texture along X, Y, and Z axes using 3D position
    vec4 xaxis = texture(texture0, fragPosition.yz * texScale);
    vec4 yaxis = texture(texture0, fragPosition.xz * texScale);
    vec4 zaxis = texture(texture0, fragPosition.xy * texScale);

    // Blend the three projections
    vec4 texColor = xaxis * blending.x + yaxis * blending.y + zaxis * blending.z;

    finalColor = texColor * colDiffuse * fragColor;
}

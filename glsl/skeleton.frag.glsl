#version 150
// ^ Change this to version 130 if you have compatibility issues

uniform vec3 u_CamPos;

// These are the interpolated values out of the rasterizer, so you can't know
// their specific values without knowing the vertices that contributed to them
in vec3 fs_Pos;
in vec4 fs_Nor;
in vec4 fs_LightVec;
in vec4 fs_Col;

out vec4 out_Col; // This is the final output color that you will see on your
                  // screen for the pixel that is currently being processed.

void main()
{
    // Material base color (before shading)
        vec4 diffuseColor = fs_Col;

        // Calculate the diffuse term for Lambert shading
        vec3 lightVec = normalize(u_CamPos - fs_Pos);
        float diffuseTerm = dot(normalize(fs_Nor.xyz), normalize(lightVec));
        // Avoid negative lighting values
        diffuseTerm = (clamp(diffuseTerm, 0, 1) + 1) * 0.5f;

        float lightIntensity = diffuseTerm;   //Add a small float value to the color multiplier
                                              //to simulate ambient lighting. This ensures that faces that are not
                                              //lit by our point light are not completely black.

        // Compute final shaded color
        out_Col = vec4(diffuseColor.rgb * lightIntensity, diffuseColor.a);
}

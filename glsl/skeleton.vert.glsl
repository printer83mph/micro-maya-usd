#version 150
// ^ Change this to version 130 if you have compatibility issues

uniform mat4 u_Model;       // The matrix that defines the transformation of the
                            // object we're rendering. In this assignment,
                            // this will be the result of traversing your scene graph.

uniform mat4 u_ModelInvTr;  // The inverse transpose of the model matrix.
                            // This allows us to transform the object's normals properly
                            // if the object has been non-uniformly scaled.

uniform mat4 u_ViewProj;    // The matrix that defines the camera's transformation.
                            // We've written a static matrix for you to use for HW2,
                            // but in HW3 you'll have to generate one yourself

uniform mat4 u_BindMats[100];

uniform mat4 u_JointTfms[100];

in vec4 vs_Pos;             // The array of vertex positions passed to the shader

in vec4 vs_Nor;             // The array of vertex normals passed to the shader

in vec4 vs_Col;             // The array of vertex colors passed to the shader.

in ivec2 vs_JointIdx;
in vec2 vs_JointWgt;

out vec3 fs_Pos;
out vec4 fs_Nor;            // The array of normals that has been transformed by u_ModelInvTr. This is implicitly passed to the fragment shader.
out vec4 fs_Col;            // The color of each vertex. This is implicitly passed to the fragment shader.

void main()
{
    fs_Col = vs_Col;                         // Pass the vertex colors to the fragment shader for interpolation

    mat3 invTranspose = mat3(u_ModelInvTr);
    fs_Nor = vec4(invTranspose * vec3(vs_Nor), 0);          // Pass the vertex normals to the fragment shader for interpolation.
                                                            // Transform the geometry's normals by the inverse transpose of the
                                                            // model matrix. This is necessary to ensure the normals remain
                                                            // perpendicular to the surface after the surface is transformed by
                                                            // the model matrix.

    vec4 joint1Pos = u_JointTfms[vs_JointIdx[0]] * u_BindMats[vs_JointIdx[0]] * vs_Pos;
    vec4 joint2Pos = u_JointTfms[vs_JointIdx[1]] * u_BindMats[vs_JointIdx[1]] * vs_Pos;
    vec4 vertPos = vs_JointWgt[0] * joint1Pos + vs_JointWgt[1] * joint2Pos;

    vec4 modelposition = u_Model * vertPos;   // Temporarily store the transformed vertex positions for use below

    fs_Pos = modelposition.xyz;

    gl_Position = u_ViewProj * modelposition;// gl_Position is a built-in variable of OpenGL which is
                                             // used to render the final positions of the geometry's vertices
}

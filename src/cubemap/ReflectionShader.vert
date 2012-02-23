#version 150

uniform mat4 modelViewMatrix;
uniform mat4 projectionMatrix;
uniform mat4 cameraMatrix;
uniform float reflectivity;

in vec4 vertex;

out float factor;
out vec3 textureCoords;

void main(void) {
    mat3 modelViewRotationMatrix = mat3x3(modelViewMatrix);
    vec3 transformedNormal = normalize(modelViewRotationMatrix*vertex.xyz);
    vec4 transformedVertex = modelViewMatrix*vertex;

    /* Reflection vector */
    vec4 reflection = vec4(reflect(normalize(transformedVertex.xyz), transformedNormal), 1.0);
    reflection = cameraMatrix*reflection;
    textureCoords = normalize(reflection.xyz);

    /* Factor of reflectivity - normals perpendicular to viewer are not reflective */
    factor = pow(1 - max(0.0, dot(transformedNormal,
        normalize(mat3x3(cameraMatrix)*modelViewRotationMatrix*vec3(0, 0, 1)))),
        reflectivity);

    gl_Position = projectionMatrix*transformedVertex;
}

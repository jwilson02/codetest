#version 330 core

layout (location = 0) in vec3 a_Position;
layout (location = 1) in vec2 a_TexCoord;
layout (location = 2) in vec4 a_Color;
layout (location = 3) in float a_Size;
layout (location = 4) in float a_Rotation;

out vec2 v_TexCoord;
out vec4 v_Color;

uniform mat4 u_ViewProjection;

void main()
{
    v_TexCoord = a_TexCoord;
    v_Color = a_Color;

    // Apply particle rotation
    vec2 position = a_Position.xy;
    if (a_Rotation != 0.0)
    {
        float s = sin(a_Rotation);
        float c = cos(a_Rotation);
        mat2 rotationMatrix = mat2(c, -s, s, c);
        position = rotationMatrix * position;
    }

    gl_Position = u_ViewProjection * vec4(position, a_Position.z, 1.0);
    gl_PointSize = a_Size;
}

#version 330 core

in vec2 v_TexCoord;
out vec4 FragColor;

uniform sampler2D u_Texture;
uniform bool u_Horizontal;
uniform float u_Weight[5] = float[] (0.227027, 0.1945946, 0.1216216, 0.054054, 0.016216);

void main()
{
    vec2 texelSize = 1.0 / textureSize(u_Texture, 0);
    vec3 result = texture(u_Texture, v_TexCoord).rgb * u_Weight[0];

    if (u_Horizontal)
    {
        for (int i = 1; i < 5; ++i)
        {
            result += texture(u_Texture, v_TexCoord + vec2(texelSize.x * i, 0.0)).rgb * u_Weight[i];
            result += texture(u_Texture, v_TexCoord - vec2(texelSize.x * i, 0.0)).rgb * u_Weight[i];
        }
    }
    else
    {
        for (int i = 1; i < 5; ++i)
        {
            result += texture(u_Texture, v_TexCoord + vec2(0.0, texelSize.y * i)).rgb * u_Weight[i];
            result += texture(u_Texture, v_TexCoord - vec2(0.0, texelSize.y * i)).rgb * u_Weight[i];
        }
    }

    FragColor = vec4(result, 1.0);
}

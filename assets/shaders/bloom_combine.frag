#version 330 core

in vec2 v_TexCoord;
out vec4 FragColor;

uniform sampler2D u_Scene;
uniform sampler2D u_BloomBlur;
uniform float u_Intensity = 1.0;

void main()
{
    vec3 sceneColor = texture(u_Scene, v_TexCoord).rgb;
    vec3 bloomColor = texture(u_BloomBlur, v_TexCoord).rgb;

    // Additive blending
    vec3 result = sceneColor + bloomColor * u_Intensity;

    // Tone mapping (simple Reinhard)
    result = result / (result + vec3(1.0));

    // Gamma correction
    result = pow(result, vec3(1.0 / 2.2));

    FragColor = vec4(result, 1.0);
}

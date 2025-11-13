#version 330 core

in vec2 v_TexCoord;
out vec4 FragColor;

uniform sampler2D u_Texture;
uniform float u_Brightness = 0.0;
uniform float u_Contrast = 1.0;
uniform float u_Saturation = 1.0;
uniform vec3 u_Tint = vec3(1.0);

vec3 adjustBrightness(vec3 color, float brightness)
{
    return color + brightness;
}

vec3 adjustContrast(vec3 color, float contrast)
{
    return (color - 0.5) * contrast + 0.5;
}

vec3 adjustSaturation(vec3 color, float saturation)
{
    float luminance = dot(color, vec3(0.2126, 0.7152, 0.0722));
    return mix(vec3(luminance), color, saturation);
}

void main()
{
    vec3 color = texture(u_Texture, v_TexCoord).rgb;

    // Apply adjustments
    color = adjustBrightness(color, u_Brightness);
    color = adjustContrast(color, u_Contrast);
    color = adjustSaturation(color, u_Saturation);
    color *= u_Tint;

    // Clamp to valid range
    color = clamp(color, 0.0, 1.0);

    FragColor = vec4(color, 1.0);
}

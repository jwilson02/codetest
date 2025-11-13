#version 330 core

in vec2 v_TexCoord;
in vec2 v_WorldPos;
out vec4 FragColor;

uniform sampler2D u_Texture;

// Lighting
struct Light {
    vec2 position;
    vec3 color;
    float intensity;
    float radius;
};

#define MAX_LIGHTS 32
uniform Light u_Lights[MAX_LIGHTS];
uniform int u_LightCount;
uniform vec3 u_AmbientColor;
uniform float u_AmbientIntensity;

void main()
{
    vec4 texColor = texture(u_Texture, v_TexCoord);

    // Start with ambient lighting
    vec3 lighting = u_AmbientColor * u_AmbientIntensity;

    // Add contribution from each light
    for (int i = 0; i < u_LightCount; ++i)
    {
        Light light = u_Lights[i];

        // Calculate distance to light
        float distance = length(v_WorldPos - light.position);

        // Attenuation (inverse square law with smoothing)
        float attenuation = light.intensity / (1.0 + distance * distance / (light.radius * light.radius));

        // Smooth falloff at edge
        float edgeFalloff = smoothstep(light.radius, light.radius * 0.5, distance);
        attenuation *= edgeFalloff;

        // Add light contribution
        lighting += light.color * attenuation;
    }

    // Apply lighting to texture color
    vec3 finalColor = texColor.rgb * lighting;

    FragColor = vec4(finalColor, texColor.a);
}

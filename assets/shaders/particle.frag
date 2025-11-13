#version 330 core

in vec2 v_TexCoord;
in vec4 v_Color;

out vec4 FragColor;

uniform sampler2D u_Texture;
uniform bool u_UseTexture = true;

void main()
{
    vec4 texColor = vec4(1.0);

    if (u_UseTexture)
    {
        texColor = texture(u_Texture, v_TexCoord);
    }
    else
    {
        // Create circular particle shape
        vec2 coord = v_TexCoord * 2.0 - 1.0;
        float dist = length(coord);

        if (dist > 1.0)
            discard;

        // Soft edge falloff
        float alpha = 1.0 - smoothstep(0.8, 1.0, dist);
        texColor = vec4(1.0, 1.0, 1.0, alpha);
    }

    FragColor = texColor * v_Color;

    // Discard fully transparent pixels
    if (FragColor.a < 0.01)
        discard;
}

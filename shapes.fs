#version 330

in vec2 fragTexCoord;
in vec4 fragColor;

out vec4 finalColor;

uniform sampler2D texture0;

void main()
{
    // 1. CIRCLE (UV y > 50.0)
    if (fragTexCoord.y > 50.0) 
    {
        // Επαναφορά των UVs στο εύρος [-1.0, 1.0]
        vec2 circleUV = vec2(fragTexCoord.x, fragTexCoord.y - 100.0);
        float dist = length(circleUV);

        // Anti-aliased κύκλος αντί για σκληρό discard
        // Το fwidth υπολογίζει το μέγεθος του pixel στην οθόνη για smooth σβήσιμο
        float delta = fwidth(dist);
        float alpha = 1.0 - smoothstep(1.0 - delta, 1.0 + delta, dist);

        if (alpha <= 0.0) discard; // Κόβουμε τα pixels έξω από τον κύκλο

        finalColor = vec4(fragColor.rgb, fragColor.a * alpha);
    }
    // 2. RECTANGLE / MESH SHAPES (UV y > 5.0)
    else if (fragTexCoord.y > 5.0) 
    {
        finalColor = fragColor;
    }
    // 3. SPRITE / TEXTURE (Standard UVs 0.0 -> 1.0)
    else 
    {
        vec4 texel = texture(texture0, fragTexCoord);
        
        // Αν το pixel είναι τελείως διάφανο, κόψτο αμέσως
        if (texel.a < 0.01) discard; 

        finalColor = texel * fragColor;
    }
}
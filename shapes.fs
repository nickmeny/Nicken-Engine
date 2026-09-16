#version 330

in vec2 fragTexCoord;
in vec4 fragColor;

out vec4 finalColor;

uniform sampler2D texture0;

void main()
{
    //CIRCLE (UV y > 50.0)
    if (fragTexCoord.y > 50.0) {
        //sub the offset (+100.0) to take back the original coordinates [-1, 1]
        vec2 circleUV = vec2(fragTexCoord.x, fragTexCoord.y - 100.0);
        
        float dist = length(circleUV);
        if (dist > 1.0) {
            discard; // Delete the angles of the quad
        }
        finalColor = fragColor;
    }
    //RECTANGLE (UV y > 5.0)
    else if (fragTexCoord.y > 5.0) {
        finalColor = fragColor;
    }
    //SPRITE / TEXTURE (Standard UVs 0.0 -> 1.0)
    else {
        vec4 texel = texture(texture0, fragTexCoord);
        finalColor = texel * fragColor;
    }
}
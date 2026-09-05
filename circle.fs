#version 330

in vec2 fragTexCoord; //cordinates that was pass from the RenderCircle ( is the UV)
in vec4 fragColor;    //color that was pass from the RenderCircle

out vec4 finalColor;

void main()
{
    // Make the coords of the Uv from [0,1] -> [-1.1] to make the rec have center the (0,0)
    vec2 st = fragTexCoord * 2.0 - 1.0;
    
    //Calculate the distance from the center
    float dist = length(st);
    
    // if the dist from the center is greater than the r of the circle
    if (dist > 1.0) {
        discard;
    }
    
    // Smooth anti-aliasing στα άκρα του κύκλου για να μην κάνει "δοντάκια"
    float alpha = smoothstep(1.0, 0.95, dist);
    finalColor = vec4(fragColor.rgb, fragColor.a * alpha);
}
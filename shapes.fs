#version 330

in vec2 fragTexCoord;
in vec4 fragColor;

out vec4 finalColor;

void main()
{
    // If the V is >1 the shape is Rec so we dont have to do anythink ( I have put 1.5 instead of 1.0 beacuse of th floating point precision errors the gpu are make)
    if (fragTexCoord.y > 1.5) 
    {
        finalColor = fragColor;
    } 
    else 
    {
        
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
}
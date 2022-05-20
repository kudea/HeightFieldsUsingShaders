#version 150

in vec3 position;
in vec4 color;
out vec4 col;

uniform int mode;

in vec3 P_left, P_right, P_down, P_up;

uniform mat4 modelViewMatrix;
uniform mat4 projectionMatrix;

void main()
{
  // compute the transformed and projected vertex position (into gl_Position) 
  // compute the vertex color (into col)
    if(mode == 0){
        gl_Position = projectionMatrix * modelViewMatrix * vec4(position.x, position.y, position.z, 1.0f);
        col = color;
    }
    
    else if(mode == 1){
        float smoothenedHeight;
        float eps = 0.00000006f;
        vec4 outputColor;
        
        smoothenedHeight = float(P_left.y + P_right.y + P_down.y + P_up.y) / 4.0f;
        
        outputColor = max(color, vec4(eps)) / max(position.y, 0.000006f) * smoothenedHeight;
        
        gl_Position = projectionMatrix * modelViewMatrix * vec4(position.x, smoothenedHeight, position.z, 1.0f);
        
        col = outputColor;
    }
  
}


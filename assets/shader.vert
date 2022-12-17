#version 330 core
layout(location=0)in vec2 aPos;
layout(location=1)in float aMass;

uniform mat4 mvp;

out vec4 position;

void main()
{
    float mass=(aMass-1.)/10000.;
    
    gl_Position=mvp*vec4(aPos,0.,1.);
    gl_PointSize=mass/20.;
    
    position=gl_Position;
}
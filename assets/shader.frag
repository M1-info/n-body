#version 330 core
out vec4 FragColor;

in vec4 position;

void main()
{
    float dist=length(position);
    dist=(dist-1.)/2000.;
    
    FragColor=vec4(1.-dist,dist,0.,1.);
}
#version 130
#extension GL_EXT_geometry_shader4 : enable

layout(points) in;
layout(points, max_vertices = 1) out;
out vec4 newMovingPoint;

void main()
{
    vec4 pos = gl_PositionIn[0];
    if (pos.x <= 1.0 && pos.x >= (-1.0) && pos.y <= 1.0 && pos.y >= (-1.0)) {
        newMovingPoint = vec4( pos.x + pos.z, pos.y + pos.w, pos.z, pos.w);
        pos = vec4(pos.x,pos.y,0.0,1.0);
        gl_Position = pos;
        EmitVertex();
        EndPrimitive();
    } 
        
}



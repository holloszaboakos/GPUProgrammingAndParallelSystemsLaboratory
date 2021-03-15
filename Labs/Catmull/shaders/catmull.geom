#version 130
#extension GL_EXT_geometry_shader4 : enable


layout (lines_adjacency) in;
layout (line_strip,max_vertices = 200) out;

in vec3 gColor[];

out vec3 fColor;

void main(void) {
    //collecting control points
    vec4 pos1 = gl_PositionIn[0];
    vec4 pos2 = gl_PositionIn[1];
    vec4 pos3 = gl_PositionIn[2];
    vec4 pos4 = gl_PositionIn[3];
    mat4 posMat = mat4(pos1,pos2,pos3,pos4);
    vec3 col1 = gColor[0];
    vec3 col2 = gColor[1];
    vec3 col3 = gColor[2];
    vec3 col4 = gColor[3];
    mat4 colMat = mat4(
        vec4(col1,0),
        vec4(col2,0),
        vec4(col3,0),
        vec4(col4,0)
    );
    float s = 0.5;
    mat4 weightMat = mat4(
        -s, 2.0-s, s-2.0, s,
        2.0*s, s-3.0, 3.0-(2.0*s), -s,
        -s, 0.0, s, 0.0,
        0.0, 1.0, 0.0, 0.0
    );
    /*
    //set vertex
    gl_Position = vec4(pos1.xyz,1.0);
    //set color
    fColor = vec3(0.0,0.0,0.0);//(tVec * weightMat * colMat).xyz;
    //send vertex out
    EmitVertex();

    //set vertex
    gl_Position = vec4(pos2.xyz,1.0);
    //set color
    fColor = vec3(1.0,0.0,0.0);//(tVec * weightMat * colMat).xyz;
    //send vertex out
    EmitVertex();
    EndPrimitive();
    */  
    for(float t = 0.0; t <= 1.0; t+=(1.0/16)){
        vec4 tVec = vec4(t*t*t,t*t,t,1);
        //set vertex
        gl_Position = vec4((( posMat * weightMat) * tVec).xyz,1.0);
        //set color
        fColor = vec3(0.0,0.0,0.0);//(tVec * weightMat * colMat).xyz;
        //send vertex out
        EmitVertex();
    }
    //end current primitive
    EndPrimitive();
}



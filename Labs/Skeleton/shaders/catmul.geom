#version 330
#extension GL_EXT_gpu_shader4: enable
#extension GL_EXT_geometry_shader4: enable

layout(lines_adjacency) in;
layout(line_strip, max_vertices=200) out;
out vec4 color;


void main(void){
   int size = gl_VerticesIn;
   for(int i = 0; i < size; i++){
      gl_Position = gl_PositionIn[i];
      color = vec4(1,0,0,1);
         EmitVertex();
   }
   EndPrimitive();
   //int size = gl_VerticesIn;
   float weight = 0.5f;
   mat4 m = mat4(
            -0.5, 1.5, -1.5, 0.5,
            1, -2.5, 2, -0.5,
            -0.5, 0, 0.5, 0,
            0, 1, 0, 0
         );
         

      mat4 control = mat4(
            gl_PositionIn[0].x, gl_PositionIn[0].y, gl_PositionIn[0].z, gl_PositionIn[0].w,
            gl_PositionIn[1].x, gl_PositionIn[1].y, gl_PositionIn[1].z, gl_PositionIn[1].w,
            gl_PositionIn[2].x, gl_PositionIn[2].y, gl_PositionIn[2].z, gl_PositionIn[2].w,
            gl_PositionIn[3].x, gl_PositionIn[3].y, gl_PositionIn[3].z, gl_PositionIn[3].w
         );

      for( float t = 0; t <= 1.0; t += (0.1f / 16.0f) ){

         vec4 time = vec4(t*t*t, t*t, t, 1.0f );
         gl_Position = control*m*time;
         color = vec4(0,1,0,1);
         EmitVertex();
      }
   EndPrimitive();
   
}

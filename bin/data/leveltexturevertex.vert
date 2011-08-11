//varying vec4 verpos;

void main()
{
	gl_FrontColor = gl_Color;
	gl_TexCoord[0] = gl_MultiTexCoord0;
	gl_Position = ftransform();
	//verpos = gl_ModelViewMatrix*gl_Vertex;
}
/*
//vertex shader
uniform mat4 model;
uniform mat4 view;
uniform mat4 proj;

in vec4 vertex_in;
out vec4 vertex_world;

main(){
   vertex_world = model * vertex_in;
   gl_Position = proj * view * vertex_world;
}

uniform mat4 view_matrix;
uniform mat4 model_matrix;
gl_Position = gl_ProjectionMatrix * view_matrix * model_matrix * gl_Vertex;
*/

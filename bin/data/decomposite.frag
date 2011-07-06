
// sampler2DRect because we're using NPOT texture sizes
uniform sampler2DRect sceneTexture;
uniform sampler2DRect frameTexture;

void main(){
     vec4 frameTexel, sceneTexel;
	// Get texels from textures
	sceneTexel = texture2DRect(sceneTexture, gl_TexCoord[0].xy);
	frameTexel = texture2DRect(frameTexture, gl_TexCoord[0].xy);
	
	// set frame alpha depending on the scenes alpha
	if(sceneTexel.a < (240.0/255.0)){
		sceneTexel.a = 1.0;
		sceneTexel.rgb = frameTexel.rgb;
	}
	// stack frame textel ontop of
	

	gl_FragColor = sceneTexel;

}
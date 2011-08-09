
// sampler2DRect because we're using NPOT texture sizes
uniform sampler2DRect alphaTexture;
uniform sampler2DRect frameTexture;

void main(){
     vec4 frameTexel, alphaTexel;
	// Get texels from textures
     alphaTexel = texture2DRect(alphaTexture, gl_TexCoord[0].xy);
     frameTexel = texture2DRect(frameTexture, gl_TexCoord[0].xy);
	// Perform masking, expects mask to be red
	frameTexel.a = alphaTexel.r;
	gl_FragColor = frameTexel;

}
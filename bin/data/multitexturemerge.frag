/* sampler2DRect because we're using NPOT texture sizes */
/*
	Order for textures is assumed to be
	scene background
	victim
	attacker1
	attacker2
*/
uniform sampler2DRect textures[4];
uniform int numTextures;
uniform float gammaCorrection;
uniform float blendRatio;

vec4 blend(vec4 a, vec4 b){
	vec4 result = (blendRatio * a) + ((1.0-blendRatio)*b) * gammaCorrection;
	result.a = 1.0;
	return result;
}

void main(){
	vec4 sceneTexel, vicTexel, atk1Texel, atk2Texel;
	/* these values are pretty aproximate right now */
	float atk1AlphaValue	= 104.0/255.0;
	float atk2Alphavalue	= 178.0/255.0;
	float vicAlphaValue		= 226.0/255.0;
	float alphaDelta		= 0.1;

	
	/* Get texels from textures */
	sceneTexel	= texture2DRect(textures[0], gl_TexCoord[0].xy);
	vicTexel	= texture2DRect(textures[1], gl_TexCoord[0].xy);
	atk1Texel	= texture2DRect(textures[2], gl_TexCoord[0].xy);
	
	if(numTextures == 4) {
		atk2Texel = texture2DRect(textures[3], gl_TexCoord[0].xy);
	} else atk2Texel.rgba = vec4(0.0, 0.0, 0.0, 0.0); /* fake null value */
	
	/* Uncomment this to see actor textures unmasked */
//	if(vicTexel.a > 0.0){
//		sceneTexel.rgb = vicTexel.rgb;
//	}
//
//	if(atk1Texel.a > 0.0){
//		sceneTexel.rgb = atk1Texel.rgb;
//	}
//
//	if(atk2Texel.a > 0.0){
//		sceneTexel.rgb = atk2Texel.rgb;
//	}
	
	/* set frame alpha depending on the scenes alpha */
	if(sceneTexel.a < (240.0/255.0)){ /* dodge the noise */
	
		if (sceneTexel.a < vicAlphaValue + alphaDelta &&
			sceneTexel.a > vicAlphaValue - alphaDelta &&
			vicTexel.a != 0.0) {
			
			sceneTexel.rgb = blend(sceneTexel, vicTexel).rgb;
		} 
		
		if (sceneTexel.a < atk2Alphavalue + alphaDelta &&
			sceneTexel.a > atk2Alphavalue - alphaDelta &&
			atk2Texel.a != 0.0) {
			
			sceneTexel.rgb = blend(sceneTexel, atk2Texel).rgb;
		} 
		
		if (sceneTexel.a < atk1AlphaValue + alphaDelta &&
			sceneTexel.a > atk1AlphaValue - alphaDelta &&
			atk1Texel.a != 0.0) {
			
			sceneTexel.rgb = blend(sceneTexel, atk1Texel).rgb;
		}

	}

	/* set alpha to be full after we've gotten the data out */
	sceneTexel.a = 1.0; 

	gl_FragColor = sceneTexel;
}
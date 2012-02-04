//#extension GL_ARB_texture_rectangle : enable
// sampler2DRect because we're using NPOT texture sizes
/*
 Order for textures is assumed to be
 scene background
 victim1
 attacker1
 attacker2
 */
uniform sampler2DRect textures[4];
uniform int	showUnmaskedTextures;
uniform int numTextures;
uniform float gammaCorrection;
uniform float blendRatio;

vec4 blend(vec4 a, vec4 b){
	vec4 result = (blendRatio * a) + ((1.0-blendRatio)*b) * gammaCorrection;
	result.a = 1.0;
	return result;
}

void main(){

	vec4 sceneTexel, vic1Texel, atk1Texel, atk2Texel;

	float atk2Alphavalue	= 104.0/255.0;
	float atk1AlphaValue	= 104.0/255.0; // deal with this problem below for double attackers
	float vic1AlphaValue	= 226.0/255.0;
	float alphaDelta		= 0.1; // reduced from 0.2, 0.2 was mixing alpha values


	// Get texels from textures
	sceneTexel	= texture2DRect(textures[0], gl_TexCoord[0].xy);
	vic1Texel	= texture2DRect(textures[1], gl_TexCoord[0].xy);
	atk1Texel	= texture2DRect(textures[2], gl_TexCoord[0].xy);

	if(numTextures == 4) {
        atk2Alphavalue	= 104.0/255.0;
        atk1AlphaValue	= 178.0/255.0;  // dealing with the alpha problem
		atk2Texel = texture2DRect(textures[3], gl_TexCoord[0].xy);
	} else atk2Texel.rgba = vec4(0.0, 0.0, 0.0, 0.0); // fake null value

	if (showUnmaskedTextures == 1) {
		if(vic1Texel.a > 0.0){
			sceneTexel.rgb = vic1Texel.rgb;
		}

		if(atk1Texel.a > 0.0){
			sceneTexel.rgb = atk1Texel.rgb;
		}

		if(atk2Texel.a > 0.0){
			sceneTexel.rgb = atk2Texel.rgb;
		}
	}

	// set frame alpha depending on the scenes alpha
	if(sceneTexel.a < (240.0/255.0)){ // dodge the noise

		if (sceneTexel.a < vic1AlphaValue + alphaDelta &&
			sceneTexel.a > vic1AlphaValue - alphaDelta &&
			vic1Texel.a != 0.0) {

			sceneTexel.rgb = blend(sceneTexel, vic1Texel).rgb;
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

	// set alpha to be full after we've gotten the data out
	sceneTexel.a = 1.0;

	gl_FragColor = sceneTexel;
}
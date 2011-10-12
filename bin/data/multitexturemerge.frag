//#define SHADER_BUILDER
#extension GL_ARB_texture_rectangle : enable
// sampler2DRect because we're using NPOT texture sizes
/*
 Order for textures is assumed to be
 scene background
 victim1
 attacker1
 attacker2
 */
uniform sampler2DRect textures[4];
#ifdef SHADER_BUILDER
uniform sampler2DRect photoshop_alpha_hack;
#endif
uniform int showUnmaskedTextures;
uniform int numTextures;
uniform float gammaCorrection;
uniform float blendRatio;
//uniform float edgeDistance;

vec4 blend(vec4 a, vec4 b, float blendMod){
	float blendRatioByMod = blendRatio * blendMod;
	vec4 result = (blendRatioByMod * a) + ((1.0-blendRatioByMod)*b) * gammaCorrection;
	result.a = 1.0;
	return result;
}

// blurs the current texture coord, returns a texel.
//vec4 blur(sampler2DRect textureToBlur){
//	vec2 texCoord = gl_TexCoord[0].st; // get current texture coordinate
//	vec2 direction = vec2(0.5, 0.5); // blur direction values
//	vec4 blurredTexel = 0.225454 * texture2DRect(textureToBlur, texCoord);
//	blurredTexel += 0.313517 *
//		(texture2DRect(textureToBlur, texCoord - (direction * 1.38491)) +
//		texture2DRect(textureToBlur, texCoord + (direction * 1.38491)));
//	blurredTexel += 0.0737561 *
//		(texture2DRect(textureToBlur, texCoord - (direction * 3.25091)) +
//		texture2DRect(textureToBlur, texCoord + (direction * 3.25091)));
//	return blurredTexel;
//
//}

// checks if a given coordinate is just outside of a shape
// On error, it returns 0.0,0.0,0.0,-1.0
// On success, it returns the colour of whatever square that was not black,
// eg the edge of a shape, this lets us know what its on the edge of and what we should
// blend with
vec4 closeToEdge(sampler2DRect texture){
	vec2 texCoord = gl_TexCoord[0].st;
	vec2 box[8];
	float aValue = texture2DRect(texture, texCoord).a; // get alpha value
	//float dist = edgeDistance; // uniform, uncomment above too.
	float dist = 1.0; // @matt, this might need tweaking, though 1 is probably ok
	// blackFloor is for checking that the alpha value is not '0.0', we bump it up a bit to avoid noise from After Effects
	float blackFloor = 0.05; // @Matt, this might need to be tweaked...

	if(aValue > blackFloor){
		//texCoord is not black, so we dont care, its not on an edge
		return vec4(0.0, 0.0, 0.0, -1.0);
	}
	// If we  get here, texCoord is black, so find a non black somewhere around it

	box[0] = vec2(texCoord.x, texCoord.y+dist); // n
	box[1] = vec2(texCoord.x+dist, texCoord.y+dist); // ne
	box[2] = vec2(texCoord.x+dist, texCoord.y); // e
	box[3] = vec2(texCoord.x+dist, texCoord.y-dist); // se
	box[4] = vec2(texCoord.x, texCoord.y-dist); // s
	box[5] = vec2(texCoord.x-dist, texCoord.y-dist); // sw
	box[6] = vec2(texCoord.x-dist, texCoord.y); // w
	box[7] = vec2(texCoord.x-dist, texCoord.y+dist); // nw

// not required, even on clamp to edge, not in my tester anyway.
//	for(int i = 0; i < 8; i++){
//		if(box[i].y > 512.0){ box[i].y = 512.0;}
//		if(box[i].x > 512.0){ box[i].x = 512.0;}
//		if(box[i].y < 0.0){box[i].y = 0.0;}
//		if(box[i].x < 0.0){box[i].x = 0.0;}
//	}

	vec4 texel;
	for(int i = 0; i < 8; i++){
		texel = texture2DRect(texture, box[i]);
		if(texel.a > blackFloor){
			// Found a non-black alpha around texCoord, so return true, this texCoord is on an edge
			return texel;
		}
	}

	return vec4(0.0, 0.0, 0.0, -1.0);
}

void main(){
	vec4 sceneTexel; // scene background texel
	vec4 vic1Texel, atk1Texel, atk2Texel; // faces texel
	// Blurred scene texel, only interested in alpha value for this

	float atk2Alphavalue	= 104.0/255.0;
	float atk1AlphaValue	= 104.0/255.0; // deal with this problem below for double attackers
	float vic1AlphaValue	= 226.0/255.0;
	float alphaDelta		= 0.2;

	bool stop = false; // used to break out of 'if' preemptively

	// Get texels from textures
	sceneTexel	= texture2DRect(textures[0], gl_TexCoord[0].xy);	vic1Texel	= texture2DRect(textures[1], gl_TexCoord[0].xy);
	atk1Texel	= texture2DRect(textures[2], gl_TexCoord[0].xy);

	if(numTextures == 4) {
		atk2Alphavalue = 104.0/255.0;
		atk1AlphaValue = 178.0/255.0;  // dealing with the alpha problem
		atk2Texel = texture2DRect(textures[3], gl_TexCoord[0].xy);
	} else atk2Texel.rgba = vec4(0.0, 0.0, 0.0, 0.0); // fake null value

#ifdef SHADER_BUILDER
	// OLLIES TEST IMAGES VALUES REMOVE THESE
	atk2Alphavalue	= 44.0/255.0;
	atk1AlphaValue	= 99.0/255.0; // deal with this problem below for double attackers
	vic1AlphaValue	= 172.0/255.0;
	alphaDelta		= 0.11;

	// photoshop exports alphas in pngs as a mask but actually removes the content from the image too, so we have to fake it
	vec4 psahack = texture2DRect(photoshop_alpha_hack, gl_TexCoord[0].xy);
	sceneTexel.a = psahack.a;

#endif

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
#ifdef SHADER_BUILDER
		vec4 test = closeToEdge(photoshop_alpha_hack); // check if we're close to an edge
#else
		vec4 test = closeToEdge(textures[0]); // check if we're close to an edge
#endif
		// @matt blendmod might have to be tweaked, a _higher value_ makes the edges fade out more
		// Cant be a uniform (sent in from OF) because it must be modifiable per result
		float blendMod = 1.5; // don't do a full blend on an edge, so set a blend mod
		if(test.a == -1.0){
			// Not close to an edge BUT we might actually be in a mask, so check
			// against the sceneTexel.
			test = sceneTexel;
//gl_FragColor = sceneTexel; return;
			blendMod = 1.0;
		}
		//else{gl_FragColor = vec4(1.0, 0.0, 0.0, 1.0); return;}

		if (!stop && vic1Texel.a != 0.0 &&
			test.a < vic1AlphaValue + alphaDelta &&
			test.a > vic1AlphaValue - alphaDelta) {
			sceneTexel.rgb = blend(sceneTexel, vic1Texel, blendMod).rgb;
			stop = true;
		}

		if (!stop && atk2Texel.a != 0.0 &&
			test.a < atk2Alphavalue + alphaDelta &&
			test.a > atk2Alphavalue - alphaDelta) {
			sceneTexel.rgb = blend(sceneTexel, atk2Texel, blendMod).rgb;
			stop = true;
		}

		if (!stop && atk1Texel.a != 0.0 &&
			test.a < atk1AlphaValue + alphaDelta &&
			test.a > atk1AlphaValue - alphaDelta) {
			sceneTexel.rgb = blend(sceneTexel, atk1Texel, blendMod).rgb;
			stop = true;
		}
	}

	// set alpha to be full after we've gotten the data out
	sceneTexel.a = 1.0;
	gl_FragColor = sceneTexel;
}

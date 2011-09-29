#extension GL_ARB_texture_rectangle : enable
// sampler2DRect because we're using NPOT texture sizes
/*
Order for textures is assumed to be
meter_mask
meter_on
meter_off
*/

uniform sampler2DRect textures[3];
uniform float level;

void main(){

	vec4 meter_mask;
	vec4 meter_onTexel;
	vec4 meter_offTexel;

	// Get texels from textures
	meter_mask      = texture2DRect(textures[0], gl_TexCoord[0].xy);
	meter_onTexel	= texture2DRect(textures[1], gl_TexCoord[0].xy);
	meter_offTexel	= texture2DRect(textures[2], gl_TexCoord[0].xy);

    if (meter_mask.a > 0.0) {
        gl_FragColor = meter_onTexel.rgba * level;
    } else gl_FragColor = meter_offTexel.rgba * level;

}

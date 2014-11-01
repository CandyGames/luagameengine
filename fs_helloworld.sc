$input v_color0, v_texcoord0

/*
 * Copyright 2011-2013 Branimir Karadzic. All rights reserved.
 * License: http://www.opensource.org/licenses/BSD-2-Clause
 */

// References:
// Sphere tracing: a geometric method for the antialiased ray tracing of implicit surfaces - John C. Hart
// http://web.archive.org/web/20110331200546/http://graphics.cs.uiuc.edu/~jch/papers/zeno.pdf
//
// Modeling with distance functions
// http://www.iquilezles.org/www/articles/distfunctions/distfunctions.htm

#include "../common/common.sh"

SAMPLER2D(u_texColor, 0);

void main()
{
	vec4 color = texture2D(u_texColor, v_texcoord0);
	//tmp = mul(u_mtx, vec4(v_texcoord0.x, v_texcoord0.y, 0.0, 1.0) );

	gl_FragColor = color;
}

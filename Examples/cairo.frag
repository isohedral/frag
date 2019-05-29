//
// Outline squares
//

float c = 0.5;
float s = 0.5*sqrt(3.0);

const int lkup[16] = int[16](
	0, 1, 2, 3,
	3, 1, 2, 0,
	0, 2, 1, 3,
	3, 2, 1, 0 );

void main() 
{ 
	vec2 r = mod(wpos, 2.0);
	vec2 r2 = mat2(c,-s,s,c) * (min(r, 2.0-r) - 0.5);

	int cell = ((r2.x>0) ? 2 : 0) + ((r2.y>0)? 1: 0);
	int cidx = ((r.y>1.0)?8:0) + ((r.x>1.0)?4:0) + cell;

	// float g = float(lkup[cidx]) / 5.0 + 0.2;
	//col = vec4( g, g, g, 1 );
	int c = lkup[cidx];

	if( c == 0 ) {
		col = vec4( 88.0 / 255.0, 140.0 / 255.0, 94.0 / 255.0, 1.0 );
	} else if( c == 1 ) {
		col = vec4( 191.0 / 255.0, 67.0 / 255.0, 105.0 / 255.0, 1.0 );
	} else if( c == 2 ) {
		col = vec4( 128.0 / 255.0, 38.0 / 255.0, 64.0 / 255.0, 1.0 );
	} else {
		col = vec4( 163.0 / 255.0, 204.0 / 255.0, 153.0 / 255.0, 1.0 );
	}
}

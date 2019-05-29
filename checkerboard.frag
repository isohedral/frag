// A simple black-and-white checkerboard.

void main() 
{ 
	ivec2 ip = ivec2(floor(wpos));
	// col = mix( vec4(0.6,0.6,0.8,1.0), vec4(0.2,0.2,0.5,1.0), (ip.x+ip.y)%2 );
	float g = 255.0*((ip.x+ip.y)%2);
	col = vec4(g,g,g,1);
}

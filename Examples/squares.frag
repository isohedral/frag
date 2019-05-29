//
// Outline squares
//

void main() 
{ 
	vec2 r = abs( round(wpos) - wpos );
	col = mix(vec4(0,0,0,1), vec4(1,1,1,1), step(max(r.x,r.y),0.4));
}

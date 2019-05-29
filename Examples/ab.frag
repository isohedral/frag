#define Swidth  .004
#define Sradius .02
#define Bradius .04
#define Mradius .02

const float A = 0.5*sqrt(2.0);
const float B = 0.5;

const vec4 udir = vec4(A,B,0,-B);
const vec4 vdir = vec4(0,B,A,B);
//the 3 directions perpendicular to the cut and project plane
const vec4 u2dir = vec4(A,-B,0,B);
const vec4 v2dir = vec4(0,B,-A,B);

//Cut and project plane origin
vec4 CPO = vec4(0);

//results of tiling search
struct Data{
	float dist;
	ivec2 sides;
	vec2 posInTile;
};

//Gives the coordinates of the point (x,y) on the "cutting" 
// plane into the 5D space
vec4 P2E4(vec2 z){
	return CPO + z.x * udir + z.y * vdir;
}

//given a point p, return the nearest vertex in the lattice and the offset.
void getRoundAndOffest(in vec4 p, out vec4 ip, out vec4 ofs){
	ip = floor(p+vec4(.5));
	ofs = p - ip;
}

//given a vector Ofs, return the vector of 1 when component >0 and -1 otherwise 
vec4 getOfsDir(vec4 ofs){
	vec4 dir;
	for(int i=0; i<4; i++){
		//if(ofs[i]>0.) dir[i]=1.; else dir[i]=-1.;
		dir[i] = 2. * float(ofs[i] > 0.) - 1.;
	}
	return dir;
}

//project the vector ofs onto the plane (udir,vdir)
vec2 projectOfs(vec4 ofs){
   //dot products
	vec2 pofs = vec2(0);
    pofs.x = dot(ofs,udir);
    pofs.y = dot(ofs,vdir);
	return pofs;
}

//Distance from a to the parallelogramm defined
//by u and v. a is expressed in the (u,v) basis
float Dist22V2(vec2 a, float f){
	vec2 p = abs(a-vec2(.5))-vec2(.5);
	return max(p.x,p.y)*f;//
}

//Finds if p is inside a the tile defined by (i,j,ip)
//dir is not per se necessary it could be se to 1s
Data section(int i, int j, vec4 p, vec4 ip, mat2 m, float f, vec2 s){
    //check intersection with dual
    vec2 lhs = vec2(ip[i] - CPO[i], ip[j] - CPO[j]) + 0.5*s;
	vec2 z = lhs * m;
	
    vec4  ofs, q;
	q = P2E4(z);
	//the intersection can be on a neighbouring tile!
	for(int k=0; k<4; k++){
		q[k] = floor(q[k]+.5);
		if(k==i)      ofs[k]=p[k] - (ip[k] + .5 * (s.x - 1.));
        else if(k==j) ofs[k]=p[k] - (ip[k] + .5 * (s.y - 1.));
		else          ofs[k]=p[k] - q[k];
	}
	
	vec2 pofs = projectOfs(ofs);
	
	//get the face corresponding to the intersected dual
    vec2 pit = (m * pofs);
    
	float dist   = Dist22V2(pit, f);
	Data d1 = Data(dist, ivec2(i,j), pit);
	return d1;
}

//
Data DE(vec2 z){

	vec4 p = P2E4(z);
	
	vec4 ip, ofs;
	getRoundAndOffest(p,ip,ofs);

    for(int i=0; i<3; i++)
	{
		for(int j=i+1; j<4; j++)
		{
			//m and f can/should be precomputed!
            // the inverse of m is used to test if:
            // - the projection of p onto cutting plane is inside the current tile
            // - the cut plane intersects the dual of the current tile
            mat2 m = mat2(vec2(udir[i],vdir[i]), vec2(udir[j],vdir[j]));
            // f is a correction factor to get the distance to the boundary of the tile
    		float f = dot(m[0],m[1]); f = sqrt(dot(m[0],m[0]) - f*f / dot(m[1],m[1]));
            //We use the inverse of m in reality :D
    		m = inverse(m);
            
            vec2 s = vec2(1.,-1.);
            //Scan the diffrent possible 4 directions
            Data d1 = section(i, j, p, ip, m, f, s.xx);
			if(d1.dist < 0.) return d1;

			d1 = section(i, j, p, ip, m, f, s.xy);
			if(d1.dist < 0.) return d1;
			
			d1 = section(i, j, p, ip, m, f, s.yx);
			if(d1.dist < 0.) return d1;

			d1 = section(i, j, p, ip, m, f, s.yy);
			if(d1.dist < 0.) return d1;
		}
	}
	return Data(0., ivec2(0), vec2(0));
}

float getFaceSurf(int i, int j){
	vec2 u,v;
	u[0]=udir[i]; u[1]=vdir[i];
	v[0]=udir[j]; v[1]=vdir[j];
	return abs(u[0]*v[1]-u[1]*v[0]);
}

float coverageFunction(float t){
	//this function returns the area of the part of the unit disc that is at the rigth of the verical line x=t.
	//the exact coverage function is:
	//t=clamp(t,-1.,1.); return (acos(t)-t*sqrt(1.-t*t))/PI;
	//this is a good approximation
	return 1.-smoothstep(-1.,1.,t);
	//a better approximation:
	//t=clamp(t,-1.,1.); return (t*t*t*t-5.)*t*1./8.+0.5;//but there is no visual difference
}

const float DRadius = 0.75;
const float Width = 2.0;
const vec3 BackgroundColor = vec3(1,1,1);
const vec3 CurveColor = vec3(0,0,0);

float coverageLine(float d, float lineWidth, float pixsize){
	d=d*1./pixsize;
	float v1=(d-0.5*lineWidth)/DRadius;
	float v2=(d+0.5*lineWidth)/DRadius;
	return coverageFunction(v1)-coverageFunction(v2);
}

vec3 color(vec2 pos) {
	//float pixsize=dFdx(pos.x);
	float pixsize=0.0001;
	Data data = DE(pos);
	float v=coverageLine(abs(data.dist), Width, pixsize);
    
	// vec3 faceCol = vec3(getFaceSurf(data.sides.x, data.sides.y)*1.8);
	vec3 faceCol = vec3(1,1,1);
    
	vec3 linCol = mix(BackgroundColor,CurveColor,v);
	return linCol*faceCol;
}

void main()
{
    CPO = 1.0 * u2dir + 1.0 * v2dir;
    col = vec4(color(wpos*1000.0),1.0);
}

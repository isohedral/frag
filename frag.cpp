// MIT License
//
// Copyright (c) 2019 Craig S. Kaplan
// 
// Permission is hereby granted, free of charge, to any person obtaining a copy
// of this software and associated documentation files (the "Software"), to deal
// in the Software without restriction, including without limitation the rights
// to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
// copies of the Software, and to permit persons to whom the Software is
// furnished to do so, subject to the following conditions:
// 
// The above copyright notice and this permission notice shall be included in all
// copies or substantial portions of the Software.
// 
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
// OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
// SOFTWARE.
//

// Frag -- a simple interactive viewer for GLSL fragment shaders, inspired
// by Shadertoy (www.shadertoy.com/).  This tool was designed to support
// one small feature over and above Shadertoy: the ability to navigate a
// virtual 2D plane interactively, using the mouse.  (This is possible
// with Shadertoy as well, but it's inconvenient.)
// 
// Launching:
//
//  Straightforward, just run a fragment shader:
//   ./frag something.frag
//  Run a fragment shader that reads from a texture:
//   ./frag -tex something_with_texture.frag
//  Run a fragment shader that writes in multiple passes to an FBO
//  and then reads from the FBO to write to the screen.
//   ./frag fbo1.frag fbo2.frag ... fbon.frag compose.frag
//
// The shader:
//
//  The program wraps your shader code inside boilerplate containing a
//  few additional declarations via uniforms.  The main ones are simple:
//   wpos: a vec2d with the 2D world coordinates of the point to sample
//   col: a vec4 to hold the output colour.
//  Your shader should be a function called main() that consumes no 
//  arguments and writes to col.
//
//  The only other things defined are:
//   tex: if an FBO is used, it'll be here.
//   tiletex: if a texture is loaded via the commandline, it will be
//    available in a sampler called tiletex.
//   resolution: an ivec2D telling you the width and height of the 
//    framebuffer.
//
// The program depends on GLFW, STB_IMAGE, and GLM.  It's not particularly
// well written, but it could easily be cleaned up and extended.

#include <vector>
#include <iostream>
#include <fstream>
#include <string>

#include <GL/glew.h>
#include <GLFW/glfw3.h>

#include <glm/glm.hpp>

#define STB_IMAGE_IMPLEMENTATION
#include "stb/stb_image.h"

using namespace glm;
using namespace std;

const size_t WIDTH = 1000;
const size_t HEIGHT = 800;
const bool DEBUG = false;

float verts[] = {
	-1.0f, -1.0f, 
	1.0f, -1.0f,
	1.0f, 1.0f, 
	-1.0f, 1.0f };

// This program is for running custom fragment shaders, so we only need
// one vertex shader.
static const char *vertex_shader_txt = R"glsl(
	#version 410

	in vec2 pos;
	uniform mat3 T;
	out vec2 wpos;

	void main() 
	{
		gl_Position = vec4( pos, 0.0, 1.0 ); 
		wpos = (T * vec3(pos,1.0)).xy;
	}
)glsl";

// Here's the boilerplate added to every fragment shader.
static const char *fragment_preamble = R"glsl(
	#version 410

	in vec2 wpos;
	layout(location=0) out vec4 col;

	uniform ivec2 resolution;
	uniform sampler2D tex;
)glsl";

static mat3 S;
static mat3 Si;

static mat3 M( 4.0, 0.0, 0.0,  0.0, 4.0, 0.0,  0.0, 0.0, 1.0 );
static mat3 Mi = inverse( M );

static bool mouse_down = false;
static bool rot_scale = false;
static vec2 prev_ndc;

static dvec2 down_pos;
static mat3 down_M;

static double last_time;
static size_t frames;

static GLuint fbo = 0;
static GLuint tex = 0;
static bool use_fbo = false;

static GLuint tiletex = 0;
bool use_tiletex = false;

// Use the spacebar to switch between translation and rotation/scaling.
static void keyPress( GLFWwindow* win, int key, int, int action, int )
{
	if( action == GLFW_PRESS ) {
		if( key == GLFW_KEY_ESCAPE || key == GLFW_KEY_Q ) {
			glfwSetWindowShouldClose( win, GL_TRUE );
		} else if( key == GLFW_KEY_SPACE ) {
			rot_scale = !rot_scale;
		}
	}
}

static vec2 getMouseNDC( GLFWwindow *win )
{
	int width;
	int height;
	glfwGetWindowSize( win, &width, &height );

	double sx;
	double sy;
	glfwGetCursorPos( win, &sx, &sy );

	return vec2( 
		S * vec3( -1.0 + 2.0*float(sx/width), 1.0 - 2.0*float(sy/height), 1.0));
}

static void mouseButton( GLFWwindow* window, int button, int action, int )
{
	if( button == GLFW_MOUSE_BUTTON_LEFT ) {
		if( action == GLFW_PRESS ) {
			if( rot_scale ) {
				glfwGetCursorPos( window, &down_pos.x, &down_pos.y );
				down_M = M;
			} else {
				prev_ndc = getMouseNDC( window );
			}
			mouse_down = true;
		} else if( action == GLFW_RELEASE ) {
			mouse_down = false;
		}
	}
}

static void mouseMotion( GLFWwindow* win, double, double )
{
	if( mouse_down ) {
		if( rot_scale ) {
			int width;
			int height;
			glfwGetWindowSize( win, &width, &height );
			vec2 c( float(width) * 0.5, float(height) * 0.5 );

			dvec2 pos;
			glfwGetCursorPos( win, &pos.x, &pos.y );

			double a1 = atan2( down_pos.y - c.y, down_pos.x - c.x );
			double a2 = atan2( pos.y - c.y, pos.x - c.x );
			double th = a2 - a1;

			double d1 = distance( c, vec2(down_pos) );
			float sc = 1.0;

			if( fabs( d1 ) > 1e-7 ) {
				double d2 = distance( c, vec2(pos) );
				sc = std::max( 0.0001, d2 / d1 );
			}

			float rc = cos(th);
			float rs = sin(th);

			mat3 SC(1.0/sc,0.0,0.0,0.0,1.0/sc,0.0,0.0,0.0,1.0);
			mat3 RO(rc,rs,0.0,-rs,rc,0.0,0.0,0.0,1.0);

			M = down_M * RO * SC;
			Mi = inverse( down_M );
		} else {
			vec2 mp = getMouseNDC( win );
			// cen += vec2( T * vec3(mp - prev_ndc, 1.0) );
			// computeTransforms();
			M = M * mat3( 
				1.0, 0.0, 0.0,  
				0.0, 1.0, 0.0,
				prev_ndc.x - mp.x, prev_ndc.y - mp.y, 1.0 );
			Mi = inverse( M );
			prev_ndc = mp;
		}
	}
}

static void resize( GLFWwindow *win, int w, int h )
{
	if( use_fbo ) {
		glBindFramebuffer( GL_FRAMEBUFFER, 0 );
		glfwGetFramebufferSize( win, &w, &h );

		glBindFramebuffer( GL_FRAMEBUFFER, fbo );
		glBindTexture( GL_TEXTURE_2D, tex );
		glTexImage2D( 
			GL_TEXTURE_2D, 0, GL_RGB, w, h, 0, GL_RGB, GL_UNSIGNED_BYTE, 0 );
		glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST );
		glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST );

		// glFramebufferTexture( GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, tex, 0 );
	}

	float asp = float(w) / float(h);
	S = mat3( asp, 0.0, 0.0,  0.0, 1.0, 0.0,  0.0, 0.0, 1.0 );
	Si = inverse( S );
}

static GLFWwindow *createWindow()
{
	if( !glfwInit() ) {
		std::cerr << "GLFW initialization failed." << std::endl;
		exit( -1 );
	}

	glfwWindowHint( GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE );
	glfwWindowHint( GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE );
	glfwWindowHint( GLFW_CONTEXT_VERSION_MAJOR, 3 );
	glfwWindowHint( GLFW_CONTEXT_VERSION_MINOR, 2 );

	GLFWwindow *win = glfwCreateWindow( 
		WIDTH, HEIGHT, "Tiling Viewer", nullptr, nullptr );

	if( !win ) {
		std::cerr << "GLFW initialization failed." << std::endl;
		exit( -1 );
	}

	glfwSetCursorPosCallback( win, mouseMotion );
	glfwSetMouseButtonCallback( win, mouseButton );
	glfwSetKeyCallback( win, keyPress );
	glfwSetWindowSizeCallback( win, resize );

	float asp = float(WIDTH) / float(HEIGHT);
	S = mat3( asp, 0.0, 0.0,  0.0, 1.0, 0.0,  0.0, 0.0, 1.0 );
	Si = inverse( S );

	return win;
}

static GLuint buildVertexArrayObject()
{
	GLuint vertex_buffer;
	GLuint result;

	glGenBuffers( 1, &vertex_buffer );
	glBindBuffer( GL_ARRAY_BUFFER, vertex_buffer );
	glBufferData( GL_ARRAY_BUFFER, 8 * sizeof(float), verts, GL_STATIC_DRAW );

	glGenVertexArrays( 1, &result );
	glBindVertexArray( result );
	glEnableVertexAttribArray( 0 );
	glBindBuffer( GL_ARRAY_BUFFER, vertex_buffer );
	glVertexAttribPointer( 0, 2, GL_FLOAT, GL_FALSE, 0, nullptr );

	return result;
}

void initializeOpenGL( GLFWwindow *win )
{
	glfwMakeContextCurrent( win );

	GLenum err = glewInit();
	if( err != GLEW_OK ) {
		std::cerr << "GLEW initialization failed." << std::endl;
		exit( -1 );
	}

	if( DEBUG ) {
		std::cout << glGetString( GL_VERSION ) << std::endl;
		std::cout << glGetString( GL_SHADING_LANGUAGE_VERSION ) << std::endl;
		std::cout << glGetString( GL_VENDOR ) << std::endl;
		std::cout << glGetString( GL_RENDERER ) << std::endl;
	}

	glfwSwapInterval( 1 );
	glDisable( GL_DEPTH );
}

static bool checkCompile( GLuint shader )
{
	GLint isCompiled = 0;
	glGetShaderiv( shader, GL_COMPILE_STATUS, &isCompiled );

	if(isCompiled == GL_FALSE) {
		GLint maxLength = 0;
		glGetShaderiv( shader, GL_INFO_LOG_LENGTH, &maxLength );

		GLchar *chs = new GLchar[ maxLength ];
		glGetShaderInfoLog( shader, maxLength, &maxLength, chs );

		std::cerr << chs << std::endl;
		delete [] chs;

		glDeleteShader( shader ); // Don't leak the shader.
		return false;
	}

	return true;
}

static GLuint buildShader( GLuint what, const char *str )
{
	GLuint ret = glCreateShader( what );
	glShaderSource( ret, 1, &str, nullptr );
	glCompileShader( ret );
	checkCompile( ret );
	return ret;
}

GLuint buildFBO( size_t w, size_t h, GLuint& tex_id )
{
	// www.opengl-tutorial.org/intermediate-tutorials/tutorial-14-render-to-texture/

	GLuint fbo;
	glGenFramebuffers( 1, &fbo );
	glBindFramebuffer( GL_FRAMEBUFFER, fbo );

	glGenTextures( 1, &tex_id );
	glBindTexture( GL_TEXTURE_2D, tex_id );
	glTexImage2D( 
		GL_TEXTURE_2D, 0, GL_RGB, w, h, 0, GL_RGB, GL_UNSIGNED_BYTE, 0 );
	glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST );
	glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST );

	glFramebufferTexture( GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, tex_id, 0 );
	GLenum bufs[1] = { GL_COLOR_ATTACHMENT0 };
	glDrawBuffers( 1, bufs );

	if( glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE ) {
		std::cerr << "Could not create FBO." << std::endl;
		exit( -1 );
	}

	return fbo;
}

static GLuint loadTexture( const char *fname )
{
	int w, h, n;
	unsigned char *bits = stbi_load( fname, &w, &h, &n, 0 );

	GLuint ret;
	glGenTextures( 1, &ret );
	glBindTexture( GL_TEXTURE_2D, ret );
	glTexImage2D( GL_TEXTURE_2D, 0, GL_RGBA, w, h, 0, GL_RGB, 
		GL_UNSIGNED_BYTE, bits );
	glGenerateMipmap( GL_TEXTURE_2D );
	glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR );
	glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER,
		GL_LINEAR_MIPMAP_LINEAR );
	glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT );
	glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT );

	stbi_image_free( bits );

	return ret;
}

int main( int argc, char **argv )
{
	mouse_down = false;

	GLFWwindow *win;
	GLuint vertex_ao;

	GLuint vertex_shader;
	std::vector<GLuint> progs;

	win = createWindow();
	last_time = glfwGetTime();
	frames = 0;

	initializeOpenGL( win );
	vertex_ao = buildVertexArrayObject();

	vertex_shader = buildShader( GL_VERTEX_SHADER, vertex_shader_txt );

	std::vector<std::string> fnames;

	for( int idx = 1; idx < argc; ++idx ) {
		if( !strcmp( argv[idx], "-tex" ) ) {
			++idx;
			tiletex = loadTexture( argv[idx] );
			use_tiletex = true;
		} else {
			fnames.push_back( argv[idx] );
			if( fnames.size() > 1 ) {
				use_fbo = true;
			}
		}
	}

	if( fnames.size() == 0 ) {
		fnames.push_back( "checkerboard.frag" );
	}

	if( use_fbo ) {
		int width;
		int height;

		glfwGetFramebufferSize( win, &width, &height );
		fbo = buildFBO( width, height, tex );
	}

	for( auto& fname : fnames ) {
		std::ifstream ifs( fname );
		std::string ifs_contents( (std::istreambuf_iterator<char>( ifs )),
			std::istreambuf_iterator<char>() );
		std::string fsh = fragment_preamble + ifs_contents;
		GLuint frag = buildShader( GL_FRAGMENT_SHADER, fsh.c_str() );

		GLuint program = glCreateProgram();
		glAttachShader( program, vertex_shader );
		glAttachShader( program, frag );
		glLinkProgram( program );

		progs.push_back( program );
	}

	while( !glfwWindowShouldClose( win ) ) {
		int width;
		int height;

		glfwGetFramebufferSize( win, &width, &height );
		glClear( GL_COLOR_BUFFER_BIT );

		if( !use_fbo ) {
			glViewport( 0, 0, width, height );
		}

		for( size_t idx = 0; idx < progs.size(); ++idx ) {
			GLuint program = progs[idx];

			glUseProgram( program );

			if( use_fbo ) {
				if( idx == progs.size() - 1 ) {
					glBindFramebuffer( GL_FRAMEBUFFER, 0 );

					glActiveTexture( GL_TEXTURE0 );
					glBindTexture( GL_TEXTURE_2D, tex );
					GLint tex_loc = glGetUniformLocation( program, "tex" );
					if( tex_loc != -1 ) {
						glUniform1i( tex_loc, 0 );
					}
				} else {
					glBindFramebuffer( GL_FRAMEBUFFER, fbo );
				}
				glViewport( 0, 0, width, height );
			}

			GLint t_loc = glGetUniformLocation( program, "T" );
			if( t_loc != -1 ) {
				mat3 T = M * S;
				glUniformMatrix3fv( t_loc, 1, GL_FALSE, &T[0][0] );
			}

			GLint res_loc = glGetUniformLocation( program, "resolution" );
			if( res_loc != -1 ) {
				glUniform2i( res_loc, width, height );
			}

			if( use_tiletex ) {
				glActiveTexture( GL_TEXTURE1 );
				glBindTexture( GL_TEXTURE_2D, tiletex );
				GLint loc = glGetUniformLocation( program, "tiletex" );
				if( loc != -1 ) {
					glUniform1i( loc, 1 );
				}
			}

			glBindVertexArray( vertex_ao );
			glDrawArrays( GL_TRIANGLE_FAN, 0, 4 );
		}

		glfwSwapBuffers( win );
		glfwWaitEvents();
	}

	glfwTerminate();
	return 0;
}

#define WIN32_LEAN_AND_MEAN
#define NOMINMAX
#include <windows.h>
#include <GL/gl.h>
#include "rffs_gl.h"
#include <string>
#include <stdexcept>
#include <vector>
#include <algorithm>
namespace rffs::gpu {
static SDL_Window* win;static SDL_GLContext context;static GLuint program,texture,vao;
static int lastW=0,lastH=0;
#define FN(ret,name,...) static ret (APIENTRY *p##name)(__VA_ARGS__)
FN(GLuint,glCreateShader,GLenum);FN(void,glShaderSource,GLuint,GLsizei,const char* const*,const GLint*);FN(void,glCompileShader,GLuint);FN(void,glGetShaderiv,GLuint,GLenum,GLint*);FN(void,glGetShaderInfoLog,GLuint,GLsizei,GLsizei*,char*);FN(GLuint,glCreateProgram,void);FN(void,glAttachShader,GLuint,GLuint);FN(void,glLinkProgram,GLuint);FN(void,glGetProgramiv,GLuint,GLenum,GLint*);FN(void,glGetProgramInfoLog,GLuint,GLsizei,GLsizei*,char*);FN(void,glDeleteShader,GLuint);FN(void,glDeleteProgram,GLuint);FN(void,glUseProgram,GLuint);FN(GLint,glGetUniformLocation,GLuint,const char*);FN(void,glUniform1i,GLint,GLint);FN(void,glUniform1f,GLint,GLfloat);FN(void,glUniform2f,GLint,GLfloat,GLfloat);FN(void,glGenVertexArrays,GLsizei,GLuint*);FN(void,glBindVertexArray,GLuint);FN(void,glDeleteVertexArrays,GLsizei,const GLuint*);FN(void,glActiveTexture,GLenum);
static GLuint shader(GLenum type,const char* source){GLuint result=pglCreateShader(type);pglShaderSource(result,1,&source,nullptr);pglCompileShader(result);GLint ok=0;pglGetShaderiv(result,0x8B81,&ok);if(!ok){char error[2048]={};pglGetShaderInfoLog(result,2047,nullptr,error);throw std::runtime_error(error);}return result;}
bool Init(SDL_Window* window){
 win=window;context=SDL_GL_CreateContext(window);if(!context)return false;
#define LOAD(name) p##name=reinterpret_cast<decltype(p##name)>(SDL_GL_GetProcAddress(#name));if(!p##name)throw std::runtime_error("OpenGL: " #name)
 LOAD(glCreateShader);LOAD(glShaderSource);LOAD(glCompileShader);LOAD(glGetShaderiv);LOAD(glGetShaderInfoLog);LOAD(glCreateProgram);LOAD(glAttachShader);LOAD(glLinkProgram);LOAD(glGetProgramiv);LOAD(glGetProgramInfoLog);LOAD(glDeleteShader);LOAD(glDeleteProgram);LOAD(glUseProgram);LOAD(glGetUniformLocation);LOAD(glUniform1i);LOAD(glUniform1f);LOAD(glUniform2f);LOAD(glGenVertexArrays);LOAD(glBindVertexArray);LOAD(glDeleteVertexArrays);LOAD(glActiveTexture);
 static const char* vertex=R"(#version 330 core
 out vec2 uv;
 void main(){vec2 p=vec2((gl_VertexID<<1)&2,gl_VertexID&2);uv=p;gl_Position=vec4(p*2.0-1.0,0,1);}
 )";
 static const char* fragment=R"(#version 330 core
 in vec2 uv;out vec4 result;
 uniform sampler2D image;uniform vec2 sourceSize;uniform int mode;uniform float softness;uniform float brightness;
 vec3 pixel(ivec2 p){return texelFetch(image,clamp(p,ivec2(0),ivec2(sourceSize)-1),0).rgb;}
 bool same(vec3 a,vec3 b){return all(lessThan(abs(a-b),vec3(.001)));}
 vec3 scale2x(ivec2 p){ivec2 b=ivec2(floor(vec2(p)*.5));ivec2 q=p-b*2;vec3 E=pixel(b),B=pixel(b+ivec2(0,-1)),D=pixel(b+ivec2(-1,0)),F=pixel(b+ivec2(1,0)),H=pixel(b+ivec2(0,1));
 if(!same(B,H)&&!same(D,F)){if(q.x==0&&q.y==0&&same(D,B))return D;if(q.x==1&&q.y==0&&same(B,F))return F;if(q.x==0&&q.y==1&&same(D,H))return D;if(q.x==1&&q.y==1&&same(H,F))return F;}return E;}
 vec3 smoothImage(vec2 p,bool art){vec2 pos=p*(art?2.0:1.0)-.5;ivec2 a=ivec2(floor(pos));vec2 f=fract(pos);vec3 c00,c10,c01,c11;
 if(art){c00=scale2x(a);c10=scale2x(a+ivec2(1,0));c01=scale2x(a+ivec2(0,1));c11=scale2x(a+ivec2(1,1));}
 else{c00=pixel(a);c10=pixel(a+ivec2(1,0));c01=pixel(a+ivec2(0,1));c11=pixel(a+ivec2(1,1));}
 return mix(mix(c00,c10,f.x),mix(c01,c11,f.x),f.y);}
 void main(){vec2 p=vec2(uv.x,1.0-uv.y)*sourceSize;vec3 sharp=pixel(ivec2(floor(p)));vec3 c=sharp;
 if(mode==1||mode==2)c=mix(sharp,smoothImage(p,mode==2),softness);
 if(mode==3){c=smoothImage(p,false);c*=.86+.14*cos(p.y*6.2831853);int band=int(gl_FragCoord.x)%3;c*=band==0?vec3(1,.96,.96):(band==1?vec3(.96,1,.96):vec3(.96,.96,1));}
 result=vec4(clamp(c*brightness,0.0,1.0),1);}
 )";
 GLuint vs=shader(0x8B31,vertex),fs=shader(0x8B30,fragment);program=pglCreateProgram();pglAttachShader(program,vs);pglAttachShader(program,fs);pglLinkProgram(program);GLint ok=0;pglGetProgramiv(program,0x8B82,&ok);pglDeleteShader(vs);pglDeleteShader(fs);if(!ok){char error[2048]={};pglGetProgramInfoLog(program,2047,nullptr,error);throw std::runtime_error(error);}
 pglGenVertexArrays(1,&vao);glGenTextures(1,&texture);glBindTexture(GL_TEXTURE_2D,texture);glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MIN_FILTER,GL_NEAREST);glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MAG_FILTER,GL_NEAREST);glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_WRAP_S,0x812F);glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_WRAP_T,0x812F);SDL_GL_SetSwapInterval(0);return true;
}
void Draw(const uint32_t* pixels,int w,int h,int mode,float smoothing,float brightness){
 if(SDL_GL_GetCurrentContext()!=context&&!SDL_GL_MakeCurrent(win,context))throw std::runtime_error("Falha ao ativar o contexto gráfico");
 int width,height;SDL_GetWindowSizeInPixels(win,&width,&height);glViewport(0,0,width,height);glDisable(GL_SCISSOR_TEST);glClearColor(.015f,.018f,.028f,1);glClear(GL_COLOR_BUFFER_BIT);
 float aspect=w>320?16.0f/9.0f:4.0f/3.0f;int drawW=width,drawH=int(width/aspect+.5f);if(drawH>height){drawH=height;drawW=int(height*aspect+.5f);}glViewport((width-drawW)/2,(height-drawH)/2,drawW,drawH);
 glDisable(GL_BLEND);glDisable(GL_DEPTH_TEST);glDisable(GL_CULL_FACE);pglUseProgram(program);pglActiveTexture(0x84C0);glBindTexture(GL_TEXTURE_2D,texture);glPixelStorei(GL_UNPACK_ALIGNMENT,4);
 if(w!=lastW||h!=lastH){glTexImage2D(GL_TEXTURE_2D,0,0x8058,w,h,0,0x80E1,GL_UNSIGNED_BYTE,pixels);lastW=w;lastH=h;}else glTexSubImage2D(GL_TEXTURE_2D,0,0,0,w,h,0x80E1,GL_UNSIGNED_BYTE,pixels);
 pglUniform1i(pglGetUniformLocation(program,"image"),0);pglUniform2f(pglGetUniformLocation(program,"sourceSize"),float(w),float(h));pglUniform1i(pglGetUniformLocation(program,"mode"),mode);pglUniform1f(pglGetUniformLocation(program,"softness"),smoothing);pglUniform1f(pglGetUniformLocation(program,"brightness"),brightness);pglBindVertexArray(vao);glDrawArrays(GL_TRIANGLES,0,3);pglBindVertexArray(0);pglUseProgram(0);glViewport(0,0,width,height);
 if(glGetError()!=GL_NO_ERROR)throw std::runtime_error("Falha ao desenhar o shader OpenGL");
}
void Capture(const char* path){int w,h;SDL_GetWindowSizeInPixels(win,&w,&h);std::vector<uint32_t> p(w*h),flipped(w*h);glPixelStorei(GL_PACK_ALIGNMENT,4);glReadPixels(0,0,w,h,0x80E1,GL_UNSIGNED_BYTE,p.data());for(int y=0;y<h;++y)std::copy_n(p.data()+y*w,w,flipped.data()+(h-1-y)*w);auto* s=SDL_CreateSurfaceFrom(w,h,SDL_PIXELFORMAT_ARGB8888,flipped.data(),w*4);if(s){SDL_SaveBMP(s,path);SDL_DestroySurface(s);}}
void Swap(){SDL_GL_SwapWindow(win);}
void Shutdown(){if(context){if(texture)glDeleteTextures(1,&texture);if(vao)pglDeleteVertexArrays(1,&vao);if(program)pglDeleteProgram(program);SDL_GL_DestroyContext(context);context=nullptr;}}
}
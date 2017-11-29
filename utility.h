#include <GL/glew.h>
#include <GL/glut.h>

#include <bits/stdc++.h>

using namespace std;


#include "glm/glm.hpp"
#include "glm/gtc/matrix_transform.hpp"
#include "glm/gtc/type_ptr.hpp"
#include "glm/gtc/quaternion.hpp"
#include "glm/gtx/quaternion.hpp"
#include "glm/gtx/euler_angles.hpp"
#include "glm/gtx/norm.hpp"

using namespace glm;

#ifndef UTILITY_H
#define UTILITY_H

const string VSPath = "shaderFiles/vertex/";
const string FSPath = "shaderFiles/fragment/";
const string TRPath = "resource/textures/";

//inits
void glutInitialize(int argc, char **argv);
int glewInitialize();


//shader loader utilities
GLuint shaderLoaderFile(string path, GLenum shaderType);
GLuint shaderLoaderString(string contents, GLenum shaderType);

//texture loader
unsigned int TextureLoader(string path, bool isAlpha);
unsigned int genNoiseTex(int width, int height);


//fps interject
void CalculateFrameRate(const string &start);

//**********//
void chooseScreenArea(float x, float y, float w, float h, float scale, int is3D);

//rotationAlongAxis
mat4 rotationMatrix(vec3 axis, float angle);
//matrix between two orientations
quat rotationBetweenVectors(vec3 start, vec3 dest);
//switch to new coordinate system
mat4 rotateModelViewToNewCoordinateSpace(vec3 new_x, vec3 new_y, vec3 new_z);

//font drawing
void drawString2D(vec2 pos, vec4 col, void* font, string s);
void drawString3D(vec3 pos, vec4 col, void* font, string s);

//conversions
float degrees(float radians);
float radians(float degrees);

//shader class
class Shader
{
public:
    unsigned int ID;
    void init(string vertexPath, string fragmentPath);
    void use();
    void set1b(const string &name, bool value) const;
    void set1i(const string &name, int value) const;
    void set1f(const string &name, float value) const;
    void set4f(const string &name, float x, float y, float z, float w) const;
    void set3m(const string &name, mat3 transform) const;
    void set4m(const string &name, mat4 transform) const;
    void set3fv(const string &name, vec3 vPtr) const;
    void setsu(GLenum shaderType, const string &name, const string &value) const;
    
private:
    void checkCompileErrors(unsigned int shader, string type);
};
#endif

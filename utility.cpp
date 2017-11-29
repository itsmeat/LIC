#include "utility.h"

#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"


int sWidth = 800;
int sHeight = 800;


//constants
float PI = 3.1415;

unsigned int TextureLoader(string path, bool isAlpha){

    // load and create a texture 
    // -------------------------
    //Deal with the image vertically down rule vs opengl vertically up rule.
    stbi_set_flip_vertically_on_load(true);
    
    unsigned int texture=0;
    GLenum interpret = GL_RGB;
    if(isAlpha==true)
        interpret = GL_RGBA;
    glGenTextures(1, &texture);
    glBindTexture(GL_TEXTURE_2D, texture); // all upcoming GL_TEXTURE_2D operations now have effect on this texture object
    
    // set the texture wrapping parameters
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);   // set texture wrapping to GL_REPEAT (default wrapping method)
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    
    // set texture filtering parameters
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    
    // load image, create texture and generate mipmaps
    int width, height, nrChannels;
    unsigned char *data = stbi_load((TRPath+path).c_str(), &width, &height, &nrChannels, 0);
    if (data)
    {
        glTexImage2D(GL_TEXTURE_2D, 0, interpret, width, height, 0, interpret, GL_UNSIGNED_BYTE, data);
        glGenerateMipmap(GL_TEXTURE_2D);
    }
    else
    {
        cout << "Failed to load texture" << endl;
    }
    stbi_image_free(data);
    return texture;
}

/////////////////////////////////////////////////////////////////////////////////
    // constructor generates the shader on the fly
    // ------------------------------------------------------------------------
void Shader::init(string vertexPath, string fragmentPath)
    {
        // 1. retrieve the vertex/fragment source code from filePath
        string vertexCode;
        string fragmentCode;
        ifstream vShaderFile;
        ifstream fShaderFile;
        // ensure ifstream objects can throw exceptions:
        vShaderFile.exceptions (ifstream::failbit | ifstream::badbit);
        fShaderFile.exceptions (ifstream::failbit | ifstream::badbit);
        try 
        {
            // open files
            vShaderFile.open((VSPath+vertexPath).c_str());
            fShaderFile.open((FSPath+fragmentPath).c_str());
            stringstream vShaderStream, fShaderStream;
            // read file's buffer contents into streams
            vShaderStream << vShaderFile.rdbuf();
            fShaderStream << fShaderFile.rdbuf();
            // close file handlers
            vShaderFile.close();
            fShaderFile.close();
            // convert stream into string
            vertexCode   = vShaderStream.str();
            fragmentCode = fShaderStream.str();
        }
        catch (ifstream::failure e)
        {
            cout << "ERROR::SHADER::FILE_NOT_SUCCESFULLY_READ" << endl;
        }
        const char* vShaderCode = vertexCode.c_str();
        const char * fShaderCode = fragmentCode.c_str();
        // 2. compile shaders
        unsigned int vertex, fragment;
        int success;
        char infoLog[512];
        // vertex shader
        vertex = glCreateShader(GL_VERTEX_SHADER);
        glShaderSource(vertex, 1, &vShaderCode, NULL);
        glCompileShader(vertex);
        checkCompileErrors(vertex, "VERTEX");
        // fragment Shader
        fragment = glCreateShader(GL_FRAGMENT_SHADER);
        glShaderSource(fragment, 1, &fShaderCode, NULL);
        glCompileShader(fragment);
        checkCompileErrors(fragment, "FRAGMENT");
        // shader Program
        ID = glCreateProgram();
        glAttachShader(ID, vertex);
        glAttachShader(ID, fragment);
        glLinkProgram(ID);
        checkCompileErrors(ID, "PROGRAM");
        // delete the shaders as they're linked into our program now and no longer necessary
        glDeleteShader(vertex);
        glDeleteShader(fragment);
    }
    // activate the shader
    // ------------------------------------------------------------------------
void Shader::use() 
    { 
        glUseProgram(ID); 
    }
    // utility uniform functions
    // ------------------------------------------------------------------------
void Shader::set1b(const string &name, bool value) const
    {         
        glUniform1i(glGetUniformLocation(ID, name.c_str()), (int)value); 
    }
    // ------------------------------------------------------------------------
void Shader::set1i(const string &name, int value) const
    { 
        glUniform1i(glGetUniformLocation(ID, name.c_str()), value); 
    }
    // ------------------------------------------------------------------------
void Shader::set1f(const string &name, float value) const
    { 
        glUniform1f(glGetUniformLocation(ID, name.c_str()), value); 
    }
void Shader::set4f(const string &name, float x, float y, float z, float w) const
	{
		glUniform4f(glGetUniformLocation(ID, name.c_str()), x, y, z, w);	
	}

void Shader::set3m(const string &name, mat3 transform) const
    {
        glUniformMatrix3fv(glGetUniformLocation(ID, name.c_str()), 1, GL_FALSE, value_ptr(transform));
    }

void Shader::set4m(const string &name, mat4 transform) const
    {
        glUniformMatrix4fv(glGetUniformLocation(ID, name.c_str()), 1, GL_FALSE, value_ptr(transform));
    }


void Shader::set3fv(const string &name, vec3 vPtr) const
    {
        glUniform3fv(glGetUniformLocation(ID, name.c_str()), 1, value_ptr(vPtr));
    }

void Shader::setsu(GLenum shaderType, const string &name, const string &value) const
    {
        int activeS,countActiveSU;
        char suname[256], surname[256]; 
        int len, numCompS;
        
        glGetProgramStageiv(ID, shaderType, GL_ACTIVE_SUBROUTINE_UNIFORMS, &countActiveSU);
        GLuint indices[countActiveSU];
        memset(indices, 0, sizeof(int)*countActiveSU);

        for (int i = 0; i < countActiveSU; ++i) {
             
            glGetActiveSubroutineUniformName(ID, shaderType, i, 256, &len, suname);
            printf("Suroutine Uniform: %d name: %s\n", i, suname);
            glGetActiveSubroutineUniformiv(ID, shaderType, i, GL_NUM_COMPATIBLE_SUBROUTINES, &numCompS);
             
            int *s = (int *)malloc(sizeof(int) * numCompS);
            glGetActiveSubroutineUniformiv(ID, shaderType, i, GL_COMPATIBLE_SUBROUTINES, s);
            printf("Compatible Subroutines:\n");
            for (int j=0; j < numCompS; ++j) {
             
                glGetActiveSubroutineName(ID, shaderType, s[j], 256, &len, surname);
                printf("\t%d - %s\n", s[j], surname);
                if(suname==name and surname==value){

                    GLuint index = glGetSubroutineIndex(ID, shaderType, surname);
                    indices[i] = s[j];
                    glUniformSubroutinesuiv(shaderType, 1, &index);
                    return;
                }
            }
            printf("\n");
            free(s);
        }
    }

// utility function for checking shader compilation/linking errors.
    // ------------------------------------------------------------------------
void Shader::checkCompileErrors(unsigned int shader, string type)
    {
        int success;
        char infoLog[1024];
        if (type != "PROGRAM")
        {
            glGetShaderiv(shader, GL_COMPILE_STATUS, &success);
            if (!success)
            {
                glGetShaderInfoLog(shader, 1024, NULL, infoLog);
                cout << "ERROR::SHADER_COMPILATION_ERROR of type: " << type << "\n" << infoLog << "\n -- --------------------------------------------------- -- " << endl;
            }
        }
        else
        {
            glGetProgramiv(shader, GL_LINK_STATUS, &success);
            if (!success)
            {
                glGetProgramInfoLog(shader, 1024, NULL, infoLog);
                cout << "ERROR::PROGRAM_LINKING_ERROR of type: " << type << "\n" << infoLog << "\n -- --------------------------------------------------- -- " << endl;
            }
        }
    }
/////////////////////////////////////////////////////////////////////////////////////

GLuint shaderLoaderFile(string path, GLenum shaderType){
    
    GLuint shaderID = 0;
    //load file contents
    string shaderString; 
    ifstream sourceFile( path.c_str() ); 
    //Source file loaded 
    if( sourceFile ) { 
    	
    	//Get shader source 
    	shaderString.assign( ( istreambuf_iterator< char >( sourceFile ) ), istreambuf_iterator< char >() );
	    return shaderLoaderString(shaderString, shaderType); 
	}
	else { 
		printf( "Unable to open file %s\n", path.c_str() ); 
	} 
	return shaderID;
}

GLuint shaderLoaderString(string contents, GLenum shaderType){

	//Create shader ID 
	GLuint shaderID = glCreateShader( shaderType ); 
	//Set shader source 
	const GLchar* shaderSource = contents.c_str(); 
	glShaderSource( shaderID, 1, (const GLchar**)&shaderSource, NULL ); 
	//Compile shader source 
	glCompileShader( shaderID );
	
	int  success;
	char infoLog[512];
	glGetShaderiv(shaderID, GL_COMPILE_STATUS, &success);
	if(!success)
	{
	    glGetShaderInfoLog(shaderID, 512, NULL, infoLog);
	    cout << "ERROR::SHADER::VERTEX::COMPILATION_FAILED\n" << infoLog << endl;
	}

	return shaderID;
}

//Initialization and Context creation wrappers for glfw and glad
// ---------------------------------------------------------------------------------------------
void glutInitialize(int argc, char **argv){

    glutInit(&argc, argv);

    glutInitWindowSize(sWidth, sHeight);
    glutInitWindowPosition(10, 10);

    glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGB);

}

int glewInitialize(){

	// initialize glew and check for OpenGL 2.0 support
    glewInit();
    if(glewIsSupported("GL_VERSION_4_0")) {
        cout << "Ready for OpenGL 4.0" << endl;
    } else {
        cerr << "OpenGL 2.0 not supported" << endl;
        return 0;
    }
    return 1;
}
//----------------------------------------------------------------------------------------------------

unsigned int genNoiseTex(int width, int height){

    unsigned int noise_texture;

    // create the noise texture
    glGenTextures(1 ,&noise_texture);
    glBindTexture(GL_TEXTURE_2D, noise_texture);

    float *arr = new float[width*height];
    for(int i=0; i<width*height; i++) {
        arr[i] = drand48();
    }

    // set the texture wrapping parameters
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);   // set texture wrapping to GL_REPEAT (default wrapping method)
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    
    // set texture filtering parameters
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    glTexImage2D(GL_TEXTURE_2D, 0, 1, width, height, 0, GL_LUMINANCE, GL_FLOAT, arr);
    delete[] arr;

    return noise_texture;
}

void CalculateFrameRate(const string &start){

    static float framesPerSecond    = 0.0f;       // This will store our fps
    static float lastTime   = 0.0f;       // This will hold the time from the last frame
    float currentTime = glutGet(GLUT_ELAPSED_TIME) * 0.001f;    
    ++framesPerSecond;
    string temp = "";
    if( currentTime - lastTime > 1.0f )
    {
        lastTime = currentTime;
        // printf("\nCurrent Frames Per Second: %d\n\n", (int)framesPerSecond);
        // sprintf(temp, "LIC  |  %d fps  ", (int)framesPerSecond);
        temp += start + "  |  " + to_string((int)framesPerSecond) + " fps";
        glutSetWindowTitle(temp.c_str());
        framesPerSecond = 0;
    }
}

mat4 rotationMatrix(vec3 axis, float angle)
{
    axis = normalize(axis);
    float s = sin(angle);
    float c = cos(angle);
    float oc = 1.0 - c;
    
    return mat4(oc * axis.x * axis.x + c,           oc * axis.x * axis.y - axis.z * s,  oc * axis.z * axis.x + axis.y * s,  0.0,
                oc * axis.x * axis.y + axis.z * s,  oc * axis.y * axis.y + c,           oc * axis.y * axis.z - axis.x * s,  0.0,
                oc * axis.z * axis.x - axis.y * s,  oc * axis.y * axis.z + axis.x * s,  oc * axis.z * axis.z + c,           0.0,
                0.0,                                0.0,                                0.0,                                1.0);
}

///////////////////////////////////////////////////////////////////////////////
// Support for selecting window context screen space via viewport.
// And then use this space for orthogonal or perspective projection.
///////////////////////////////////////////////////////////////////////////////
void chooseScreenArea(float x, float y, float w, float h, float scale, int is3D){

    glViewport (x, y, w, h);
    //use projection scaling in the chosen viewport
    glMatrixMode (GL_PROJECTION);       /* Select The Projection Matrix */
    glLoadIdentity ();                          /* Reset The Projection Matrix */
    if(is3D == 1){
        
        glClearDepth(1.0f);                   // Set background depth to farthest
        glEnable(GL_DEPTH_TEST);   // Enable depth testing for z-culling
        glDepthFunc(GL_LEQUAL);    // Set the type of depth-test
        glShadeModel(GL_SMOOTH);   // Enable smooth shading
        glHint(GL_PERSPECTIVE_CORRECTION_HINT, GL_NICEST);  // Nice perspective corrections

        gluPerspective(45, w/h, 0.1, 100);
        
    }
    else
        gluOrtho2D(x, w*scale, y, h*scale);

    glMatrixMode (GL_MODELVIEW);            /* Select The Modelview Matrix */
    glLoadIdentity ();                              /* Reset The Modelview Matrix */

}

///////////////////////////////////////////////////////////////////////////////
// write 2d text using GLUT
// The projection matrix must be set to orthogonal before call this function.
///////////////////////////////////////////////////////////////////////////////
void drawString2D(vec2 pos, vec4 col, void* font, string s)
{
    glColor4fv((float *)&col);
    glRasterPos2fv((float *)&pos);
    int len, i;
    const char *str = s.c_str();
    len = s.length();
    for (i = 0; i < len; i++) {
    glutBitmapCharacter(font, str[i]);
    }
}

///////////////////////////////////////////////////////////////////////////////
// draw a string in 3D space
///////////////////////////////////////////////////////////////////////////////
void drawString3D(vec3 pos, vec4 col, void* font, string s)
{
    glColor4fv((float *)&col);          // set text color
    glRasterPos3fv((float *)&pos);        // place text position

    int len, i;
    const char *str = s.c_str();
    len = s.length();
    for (i = 0; i < len; i++) {
    glutBitmapCharacter(font, str[i]);
    }
}

float degrees(float radians){

    return radians*180.0f/PI;
}

float radians(float degrees){

    return degrees*PI/180.0f;
}

// Returns a quaternion such that q*start = dest
quat rotationBetweenVectors(vec3 start, vec3 dest){
    start = normalize(start);
    dest = normalize(dest);
    
    float cosTheta = dot(start, dest);
    vec3 rotationAxis;
    
    if (cosTheta < -1 + 0.001f){
        // special case when vectors in opposite directions :
        // there is no "ideal" rotation axis
        // So guess one; any will do as long as it's perpendicular to start
        // This implementation favors a rotation around the Up axis,
        // since it's often what you want to do.
        rotationAxis = cross(vec3(0.0f, 0.0f, 1.0f), start);
        if (length2(rotationAxis) < 0.01 ) // bad luck, they were parallel, try again!
            rotationAxis = cross(vec3(1.0f, 0.0f, 0.0f), start);
        
        rotationAxis = normalize(rotationAxis);
        return angleAxis(glm::radians(180.0f), rotationAxis);
    }

    // Implementation from Stan Melax's Game Programming Gems 1 article
    rotationAxis = cross(start, dest);

    float s = sqrt( (1+cosTheta)*2 );
    float invs = 1 / s;

    return quat(
        s * 0.5f, 
        rotationAxis.x * invs,
        rotationAxis.y * invs,
        rotationAxis.z * invs
    );


}

//sets the modelview matrix to new coordinate system defined by the axes
mat4 rotateModelViewToNewCoordinateSpace(vec3 new_x, vec3 new_y, vec3 new_z){

        /*************************Dont' Delete*********************/
    // glRotatef(-angle_x, 1, 0, 0);   // pitch
    // glRotatef(-angle_y, 0, 1, 0);   // heading
    // glRotatef(cameraAngleZ, 0, 0, 1);   // roll

    // quat quaternion = rotationBetweenVectors(front, vec3(0, 0, 1));
    // mat4 rotationMatrix = toMat4(quaternion);
    // glMultMatrixf((float*)&rotationMatrix);
    /***********************************************************/

    mat4 rotationMatrix = mat4(vec4(new_x, 0), vec4(new_y, 0), vec4(new_z, 0), vec4(0, 0, 0, 1));
    rotationMatrix = transpose(rotationMatrix);
    return rotationMatrix;

}

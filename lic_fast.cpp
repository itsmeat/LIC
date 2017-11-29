#include "utility.h"
#include <unistd.h>
//Implementing fastLIC

//opengl data
extern int sWidth, sHeight;
GLfloat zoom = 1;

// time
float t = 0.0;
float max_t = numeric_limits<float>::max();

//Algorithms Params
float h = 0.5;//step-size
int L = 20,//streamline-length
    M = 10,//extension
    width = sWidth,
    height =sHeight,
    minNumHits = 1,
    offset = 0;

//intensity data
float *Idata;
//hit data
unsigned int *numHits;
//in texture
float *noise;
//out texture
unsigned int texture;

//vector field definition
vec2 field(vec2 v) {
	
	float x = v[0] - width / 2;
	float y = v[1] - height / 2;
	float norm = sqrt(x * x + y * y);

	if (norm == 0) {
		return vec2(0, 0);
	}
	
	return vec2(-y*cos(t)/norm, x*sin(t)/norm);
}

//initialize data
void initializeVectors() {

	//choose the input texture
    noise = new float[width*height];
    for(int i=0; i<width*height; i++){
    	noise[i] = drand48();
    }
	//intensity data
	Idata = new float[width * height];
	//pixel hit data
	numHits = new unsigned int[width * height];
}

vec2 RK(vec2 p, float h) {

	vec2 v = field(p);
	vec2  k1, k2, k3, k4;

	k1 = v*h;

	v = field(p + k1*0.5f);
	k2 = v*h;

	v = field(p + k2*0.5f);
	k3 = v*h;

	v = field(p + k3);
	k4 = v*h;

	float a = 1.0/6, b = 1.0/3;
	p = p + k1*a + k2*b + k3*b + k4*a;

	return p;
}

vector<vec2> computeStreamLine(vec2 p) {
  
	vector<vec2> fwd(L+M-1), bwd(L+M-1);
	vec2 f = p, b = p;

	for (int i = 0; i < L + M -1; ++i) {
		f = RK(f,  h);
		fwd[i] = f;

		b = RK(b, -h);
		bwd[i] = b;
	}

	reverse(bwd.begin(), bwd.end());
	bwd.push_back(p);
	bwd.insert(bwd.end(), fwd.begin(), fwd.end());

	return bwd;
}

bool inBounds(vec2 p) {
  return p[0] >= 0 && p[0] < width && p[1] >= 0 && p[1] < height;
}

void I(vector<vec2> streamLine){

	int l = streamLine.size(),
		mid = (l / 2) + offset,
		k = 0;
	ivec2 x0 = streamLine[mid];

	//compute integral for center of streamline
	float Ix0;
	for (int i = -L; i <=L; ++i) {
		
		ivec2 xi = streamLine[mid + i];
		if (inBounds(xi)) {
		  Ix0 += noise[xi[0] + xi[1]*width];
		  k++;
		}
	}
	Ix0 /= (float)k;

	Idata[x0[0] + x0[1] * width] += Ix0;
	numHits[x0[0] + x0[1] * width]++;

	float IxFwd = Ix0, IxBwd = Ix0;
	//compute integral for left and right points along the streamline
	for (int i = 1; i < M; ++i) {
	
		//compute fwd integral
		int iFwd = i + mid,
		    iFwdRight = iFwd + L,
		    iFwdLeft  = iFwd - L;

		ivec2 xFwd = streamLine[iFwd],
			  xFwdLeft = streamLine[iFwdLeft],
			  xFwdRight = streamLine[iFwdRight];

		if (inBounds(xFwdLeft) && inBounds(xFwdRight)) {

			IxFwd += (noise[xFwdRight[0] + xFwdRight[1]*width] - noise[xFwdLeft[0] + xFwdLeft[1]*width]) /(float)k;
			Idata[xFwd[0] + xFwd[1] * width] += IxFwd;
			numHits[xFwd[0] + xFwd[1] * width]++;
		}

		//compute bwd integral
		int iBwd = -i + mid,
		    iBwdRight = iBwd - L,
		    iBwdLeft  = iBwd + L;

		ivec2 xBwd = streamLine[iBwd],
			  xBwdLeft = streamLine[iBwdLeft],
		  	  xBwdRight = streamLine[iBwdRight];

		if (inBounds(xBwdLeft) && inBounds(xBwdRight)) {

			IxBwd += (noise[xBwdRight[0] + xBwdRight[1]*width] - noise[xBwdLeft[0] + xBwdLeft[1]*width]) /(float)k;
			Idata[xBwd[0] + xBwd[1] * width] += IxBwd;
			numHits[xBwd[0] + xBwd[1] * width]++;
		}
	}
}

void LIC(){

	vector<vec2> sl;
	for(int i=0; i<width*height; i++){

		vec2 p = vec2(i%width, i/width);
		if(numHits[i] < minNumHits){

			sl = computeStreamLine(p);
			I(sl);
		}
	}

	//normalize LICs
	for (int i = 0; i < width * height; ++i) {
		if(numHits!=0)
			Idata[i] /= numHits[i];
		// printf("%f\n", Idata[i]);
	}
}

GLvoid draw() {

    // render
    // -----
    glClearColor(0.2f, 0.3f, 0.3f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT);

    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    glOrtho(-zoom,zoom,-zoom,zoom,-0.5,0.5);

    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();


    // bump time step
    t += 0.005;
    if(t >= max_t) {
        t = 0.0;
    }
    CalculateFrameRate();
    fill(Idata, Idata+width*height, 0);
    fill(numHits, numHits + width*height, 0);
   //2.compute LIC
    LIC();
    glTexImage2D(GL_TEXTURE_2D, 0, GL_LUMINANCE, width, height, 0, GL_LUMINANCE, GL_FLOAT, Idata);

    glBegin(GL_QUADS);
    glTexCoord2f(0,0);
    glVertex2f(-1,-1);

    glTexCoord2f(1,0);
    glVertex2f(1,-1);

    glTexCoord2f(1,1);
    glVertex2f(1,1);

    glTexCoord2f(0,1);
    glVertex2f(-1,1);

    glEnd();
    glFinish();

    // back to the old-fashioned pipeline
    // glUseProgram(0);

    glutSwapBuffers();

    // animate
    glutPostRedisplay();
}


GLvoid reshape(GLint vpw, GLint vph) {
    
    sWidth = vpw;
    sHeight = vph;
    glViewport(0, 0, sWidth, sHeight);
    glutReshapeWindow(sWidth, sHeight);

    // add display event to queue
    glutPostRedisplay();
}

int main(int argc, char **argv)
{
    glutInitialize(argc, argv);

    glutCreateWindow("LIC");

    glutReshapeFunc(reshape);
    // glutKeyboardFunc(keyboard);
    // glutMouseFunc(mouse_button);
    // glutMotionFunc(button_motion);
    glutDisplayFunc(draw);

    if(!glewInitialize())
        return -1;
    
    glEnable(GL_TEXTURE_2D);

    // no need for depth test
    glDisable(GL_DEPTH_TEST);


    //1.
    initializeVectors();

    glGenTextures(1 ,&texture);
    glBindTexture(GL_TEXTURE_2D, texture);

    // set the texture wrapping parameters
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);   // set texture wrapping to GL_REPEAT (default wrapping method)
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    
    // set texture filtering parameters
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	

    glutMainLoop();
    return 0;
}
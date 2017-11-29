#include "utility.h"

bool is_left_down = false;
GLfloat zoom = 1;
GLint mprev[2];

GLuint noise_texture;
Shader ourShader;
// time
float t = 0.0;
float max_t = numeric_limits<float>::max();

int lic_number_of_steps = 50;

extern int sWidth, sHeight;
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

    ourShader.use();
    ourShader.set1f("t", t);
    ourShader.set1i("STEPS", lic_number_of_steps);
    
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

GLvoid mouse_button(GLint btn, GLint state, GLint x, GLint y) {
    if(btn == GLUT_LEFT_BUTTON) {
        if(state==GLUT_DOWN) {
            is_left_down = true;
            mprev[0] = x;
            mprev[1] = y;
        }
        if(state==GLUT_UP) {
            is_left_down = false;
            zoom += 8*(y-mprev[1])/(float)sHeight;
        }
    }
}

GLvoid button_motion(GLint x, GLint y) {
    if(is_left_down) {
        // middle button is down - update zoom factor (related to field of view - see draw()
        // also, save the
        zoom *= 1+(y-mprev[1])/200.0;
        mprev[0] = x;
        mprev[1] = y;
    }
    glutPostRedisplay(); // add display event to queue
    return;
}

GLvoid keyboard(GLubyte key, GLint x, GLint y) {
    switch(key) {
        // Esc
        case 27:
            exit(0);
            break;
        default:
            break;
    }
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
    glutKeyboardFunc(keyboard);
    glutMouseFunc(mouse_button);
    glutMotionFunc(button_motion);
    glutDisplayFunc(draw);

    if(!glewInitialize())
        return -1;
    
    glEnable(GL_TEXTURE_2D);

    // no need for depth test
    glDisable(GL_DEPTH_TEST);

    // create the noise texture
    glGenTextures(1 ,&noise_texture);
    glBindTexture(GL_TEXTURE_2D, noise_texture);
    {
        float *arr = new float[sWidth*sHeight];

        for(int i=0; i<sWidth*sHeight; i++) {
            arr[i] = drand48();
        }
        glTexImage2D(GL_TEXTURE_2D, 0, 1, sWidth, sHeight, 0, GL_LUMINANCE, GL_FLOAT, arr);
        delete[] arr;
    }
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);

    unsigned int texture1 = TextureLoader("desert.jpg", 0);
    unsigned int texture1 = TextureLoader("awesomeface.png", 1);
    ourShader.init("lic", "lic");
    ourShader.use();
    ourShader.set1i("tex", 0);

    // bind Texture
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, texture1);
    // glBindTexture(GL_TEXTURE_2D, noise_texture);
    glutMainLoop();
    return 0;
}

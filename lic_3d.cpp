#include "utility.h"

bool is_left_down = false;
GLfloat zoom = 1;
GLint mprev[2];

GLuint texture;
Shader ourShader;
// time
float t = 0.0;
float max_t = numeric_limits<float>::max();

int lic_number_of_steps = 100;
int user_selection;
int angle_x = 0, angle_y = 0;
int gmouse_x = 0, gmouse_y = 0;
int constant = 2;
void* gFont = GLUT_BITMAP_TIMES_ROMAN_24;
extern int sWidth, sHeight;
mat3 rotMat;

//eye position
vec3 eye = vec3(0, 0, 30);
//aim position
vec3 aim = vec3(0, 0, -30);

//front
vec3 front = vec3(0, 0, 1);
//top
vec3 top = vec3(0, 1, 0);
//side
vec3 side = vec3(1, 0, 0);

void rotateSide(vec3 &axis, int neg){

    vec4 tmp;
    float angle = constant;
    if(neg)
        angle *= -1;

    tmp = rotationMatrix(side, radians(-angle))*vec4(axis, 1);
    axis = vec3(tmp.x, tmp.y, tmp.z); 
}

void rotateTop(vec3 &axis, int neg){

    vec4 tmp;
    float angle = constant;
    if(neg)
        angle *= -1;

    tmp = rotationMatrix(top, radians(-angle))*vec4(axis, 1);
    axis = vec3(tmp.x, tmp.y, tmp.z);
}

void makeCyCo(float r0, float r1, float h, vec3 col)
{
    GLUquadricObj *qObj = gluNewQuadric();
    gluQuadricDrawStyle(qObj, GLU_FILL);
    gluQuadricNormals(qObj, GLU_SMOOTH);

    glColor3f(col.r, col.g, col.b);
    gluCylinder(qObj, r0, r1, h, 16, 1);

    gluDeleteQuadric(qObj);
}

void drawPositionSystem(){

    glColor3f(1, 1, 1);
    
    glPushMatrix();   
        
        mat4 rotMat = rotateModelViewToNewCoordinateSpace(side, top, front);
        glMultMatrixf(value_ptr(rotMat));

        glPushMatrix();
            glRotatef(90, 1, 0, 0); 
            glutWireSphere(5, 16, 10);
        glPopMatrix();
        
        glPushMatrix();
            glRotatef(90, 0, 1, 0);
            makeCyCo(0.2, 0.2, 10, vec3(1,0,0));
            glTranslatef(0, 0, 10);
            makeCyCo(0.3, 0, 2, vec3(1,0,0));    
        glPopMatrix();

        glPushMatrix();
            glRotatef(-90, 1, 0, 0);
            makeCyCo(0.2, 0.2, 10, vec3(0,1,0));
            glTranslatef(0, 0, 10);
            makeCyCo(0.3, 0, 2, vec3(0,1,0));
        glPopMatrix();

        glPushMatrix();
            glRotatef(0, 0, 0, 1);
            makeCyCo(0.2, 0.2, 10, vec3(0,0,1));
            glTranslatef(0, 0, 10);
            makeCyCo(0.2, 0, 2, vec3(0,0,1));
        glPopMatrix();

    glPopMatrix();
}

GLvoid draw() {

    // render
    // -----
    //Background Wash
    glClearColor(0, 0, 0, 1);
    glClear(GL_COLOR_BUFFER_BIT);
    glClear (GL_DEPTH_BUFFER_BIT);

    // bump time step
    t += 0.005;
    if(t >= max_t) {
        t = 0.0;
    }
    string temp = "LIC  |  ("+to_string(front.x)+": "+to_string(front.y)+ ":" + to_string(front.z) + ") ";
    CalculateFrameRate(temp);
    
    ourShader.use();
    //time
    ourShader.set1f("t", t);
    //point_click
    ourShader.set1f("user_x", (float)mprev[0]/sWidth);
    ourShader.set1f("user_y", (float)mprev[1]/sHeight);
    ourShader.set1i("bool_lic", (is_left_down==true?1:0));
    ourShader.set1f("angle_x", (float)angle_x);
    ourShader.set1f("angle_y", (float)angle_y);


    // printf("F:%f, %f, %f\n", front.x, front.y, front.z);
    // printf("T:%f, %f, %f\n", top.x, top.y, top.z);
    // printf("S:%f, %f, %f\n", side.x, side.y, side.z);
    
    //bottom left(Front)
    rotMat = transpose(mat3(side, top, front));
    ourShader.set3m("rotMat", rotMat);
    chooseScreenArea(0, 0, sWidth/2, sHeight/2, 1, 0);
    glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);

    //top left(Top)
    rotMat = transpose(mat3(side, -front, top));
    ourShader.set3m("rotMat", rotMat);
    chooseScreenArea(0, sHeight/2, sWidth/2, sHeight/2, 1, 0);
    glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);
    
    //top right(Side)
    rotMat = transpose(mat3(-front, top, side));
    ourShader.set3m("rotMat", rotMat);
    chooseScreenArea(sWidth/2, sHeight/2, sWidth/2, sHeight/2, 1, 0);
    glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);
    
    glUseProgram(0);
    
    chooseScreenArea(0, 0, sWidth, sHeight, 1, 0);
    drawString2D(vec2(10, 10), vec4(0, 0, 0, 1), gFont, "Front");
    drawString2D(vec2(10, 350), vec4(0, 0, 0, 1), gFont, "Top");
    drawString2D(vec2(400, 350), vec4(0, 0, 0, 1), gFont, "Side");
    
    //bottom right-wiremesh sphere with arrow protuding showing the camera position
    chooseScreenArea(sWidth/2, 0, sWidth/2, sHeight/2, 1, 1);
    // set the camera position
    gluLookAt(  eye.x, eye.y, eye.z,   //  eye pos
                aim.x, aim.y, aim.z,   //  aim point
                0, 1, 0); //  up direction
    drawPositionSystem();

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
    }
}

GLvoid button_motion(GLint x, GLint y) {

    gmouse_x = x;
    gmouse_y = y;

    if(is_left_down) {
        
        mprev[0] = x;
        mprev[1] = y;
    }
    glutPostRedisplay(); // add display event to queue
    return;
}

void keyspecial(int key, int x, int y)
{
  int i = 0;
  switch (key) {

    case GLUT_KEY_LEFT:
            angle_y -= constant;
            angle_y %= 360;

            rotateTop(front, 1);
            // rotateTop(top, 1);//************Since no effect
            rotateTop(side, 1);
        break;

    case GLUT_KEY_RIGHT:
            angle_y += constant;
            angle_y %= 360;

            rotateTop(front, 0);
            // rotateTop(top, 0);
            rotateTop(side, 0);
        break;
    
    case GLUT_KEY_UP:
            angle_x -= constant;
            angle_x %= 360;

            rotateSide(front, 1);
            rotateSide(top, 1);
            // rotateSide(side, 1);
         break;
    
    case GLUT_KEY_DOWN:
            angle_x += constant;
            angle_x %= 360;

            rotateSide(front, 0);
            rotateSide(top, 0);
            // rotateSide(side, 0);
        break;

  }
    glutPostRedisplay();
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
    //user_input
    if(argc == 2){

        sscanf(argv[1], "%d", &user_selection);
    }
    else
        user_selection = -1;
    //
    glutInitialize(argc, argv);

    glutCreateWindow("LIC");

    glutReshapeFunc(reshape);
    glutKeyboardFunc(keyboard);
    glutMouseFunc(mouse_button);
    glutMotionFunc(button_motion);
    //the camera functionality
    glutSpecialFunc(keyspecial);
    glutDisplayFunc(draw);

    if(!glewInitialize())
        return -1;
    
    glEnable(GL_TEXTURE_2D);

    // no need for depth test
    glDisable(GL_DEPTH_TEST);

    // create the noise texture

    texture = genNoiseTex(sWidth/2, sHeight/2);
    // texture = TextureLoader("desert.jpg", 0);
    // texture = TextureLoader("awesomeface.png", 1);
    
    float vertices[] = {
        // positions          // texture coords
         1.0f,  1.0f, 0.0f,   1.0f, 1.0f, // top right
         1.0f, -1.0f, 0.0f,   1.0f, 0.0f, // bottom right
        -1.0f, -1.0f, 0.0f,   0.0f, 0.0f, // bottom left
        -1.0f,  1.0f, 0.0f,   0.0f, 1.0f  // top left 
    };
    unsigned int indices[] = {  
        0, 1, 3, // first triangle
        1, 2, 3  // second triangle
    };
    unsigned int VBO, VAO, EBO;
    glGenVertexArrays(1, &VAO);
    glGenBuffers(1, &VBO);
    glGenBuffers(1, &EBO);

    glBindVertexArray(VAO);

    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW);

    // position attribute
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);
    // texture coord attribute
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)(3 * sizeof(float)));
    glEnableVertexAttribArray(1);

    ourShader.init("3d", "3d");
    ourShader.use();
    ourShader.set1i("tex", 0);
    ourShader.set1i("STEPS", lic_number_of_steps);

    if(user_selection <0 or user_selection>10)
        ourShader.setsu(GL_FRAGMENT_SHADER, "selection", "f");
    else
        ourShader.setsu(GL_FRAGMENT_SHADER, "selection", "f"+to_string(user_selection));

    // bind Texture
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, texture);

    //bind VAO
    glBindVertexArray(VAO);

    glutMainLoop();
    return 0;
}

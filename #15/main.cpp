#define _CRT_SECURE_NO_WARNINGS //--- ���α׷� �� �տ� ������ ��
#pragma comment(lib, "glew32.lib")
#pragma comment(lib, "freeglut.lib")
#include <iostream>
#include <gl/glew.h>
#include <gl/freeglut.h>
#include <gl/freeglut_ext.h>
#include <time.h>
#include <random>
#include <stdlib.h>
#include <glm/glm.hpp>
#include <glm/ext.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <random>

#define WINDOWX 800
#define WINDOWY 800

using namespace std;

random_device rd;
default_random_engine dre(rd());
uniform_real_distribution<float>uid(0, 1);

GLchar* vertexsource, * fragmentsource; //--- �ҽ��ڵ� ���� ����
GLuint vertexshader, fragmentshader; //--- ���̴� ��ü

void make_vertexShaders();
void make_fragmentShaders();
GLuint make_shaderProgram();
GLvoid InitBuffer();
void InitShader();

GLuint shaderID;
GLint width, height;

GLuint VAO, VBO[2];

class Line {
public:
    GLfloat p[6];
    GLfloat color[6];

    void Bind() {
        glBindBuffer(GL_ARRAY_BUFFER, VBO[0]);
        glBufferData(GL_ARRAY_BUFFER, sizeof(p), p, GL_STATIC_DRAW);
        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), 0);
        glEnableVertexAttribArray(0);

        glBindBuffer(GL_ARRAY_BUFFER, VBO[1]);
        glBufferData(GL_ARRAY_BUFFER, sizeof(color), color, GL_STATIC_DRAW);
        glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), 0);
        glEnableVertexAttribArray(1);
    }
    void Draw() {
        glBindVertexArray(VAO);
        glLineWidth(1.0);
        glDrawArrays(GL_LINE_STRIP, 0, 2);
    }
};//�� 2�� ���� �̾��شٰ�

bool line = false;
bool culling = false;
class Plane {
public:
    GLfloat p[9];
    GLfloat color[9];

    void Bind() {
        glBindBuffer(GL_ARRAY_BUFFER, VBO[0]);
        glBufferData(GL_ARRAY_BUFFER, sizeof(p), p, GL_STATIC_DRAW);
        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), 0);
        glEnableVertexAttribArray(0);

        glBindBuffer(GL_ARRAY_BUFFER, VBO[1]);
        glBufferData(GL_ARRAY_BUFFER, sizeof(color), color, GL_STATIC_DRAW);
        glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), 0);
        glEnableVertexAttribArray(1);
    }
    void Draw() {
        glBindVertexArray(VAO);
        if (line) {
            glLineWidth(5.0);
            glDrawArrays(GL_LINE_LOOP, 0, 3);
        }
        else {
            if(culling) glEnable(GL_CULL_FACE);
            glDrawArrays(GL_TRIANGLES, 0, 3);
            glDisable(GL_CULL_FACE);
        }
    }
};//�� 3��

float mx, my, px, py;
float cnt;
int sel;
char comm;

BOOL Time = true, start = TRUE, TimeLoop;

float BackGround[] = { 1.0, 1.0, 1.0 };
Line xl = {
   0, 1, 0,
   0, -1, 0,

   0, 0, 0,
   0, 0, 0,
};
Line yl = {
   1, 0, 0,
   -1, 0, 0,

   0, 0, 0,
   0, 0, 0,
};
glm::vec4* vertex;
glm::vec4* face;

Plane* Fvertex[2];

glm::mat4 TR = glm::mat4(1.0f);
GLUquadricObj* obj;
FILE* FL;
int faceNum = 0;
int Click = 0;

bool mode = true;

void ReadObj(FILE* objFile);
void keyboard(unsigned char, int, int);
void Mouse(int button, int state, int x, int y);
void Motion(int x, int y);
void TimerFunction(int value);
GLvoid drawScene(GLvoid);
GLvoid Reshape(int w, int h);
void vectoplane(Plane* p);
void planecolorset(Plane* p, int a);

char* filetobuf(const char* file)
{
    FILE* fptr;
    long length;
    char* buf;
    fptr = fopen(file, "rb"); // Open file for reading 
    if (!fptr) // Return NULL on failure 
        return NULL;
    fseek(fptr, 0, SEEK_END); // Seek to the end of the file 
    length = ftell(fptr); // Find out how many bytes into the file we are 
    buf = (char*)malloc(length + 1); // Allocate a buffer for the entire length of the file and a null terminator 
    fseek(fptr, 0, SEEK_SET); // Go back to the beginning of the file 
    fread(buf, length, 1, fptr); // Read the contents of the file in to the buffer 
    fclose(fptr); // Close the file 
    buf[length] = 0; // Null terminator 
    return buf; // Return the buffer 
}

void make_vertexShaders()
{
    vertexsource = filetobuf("vertex.glsl");
    vertexshader = glCreateShader(GL_VERTEX_SHADER);
    glShaderSource(vertexshader, 1, (const GLchar**)&vertexsource, 0);
    glCompileShader(vertexshader);

    GLint result;
    GLchar errorLog[512];
    glGetShaderiv(vertexshader, GL_COMPILE_STATUS, &result);
    if (!result)
    {
        glGetShaderInfoLog(vertexshader, 512, NULL, errorLog);
        cerr << "ERROR: vertex shader ������ ����\n" << errorLog << endl;
        exit(-1);
    }
}
void make_fragmentShaders()
{
    fragmentsource = filetobuf("fragment.glsl");

    fragmentshader = glCreateShader(GL_FRAGMENT_SHADER);
    glShaderSource(fragmentshader, 1, &fragmentsource, NULL);
    glCompileShader(fragmentshader);

    GLint result;
    GLchar errorLog[512];
    glGetShaderiv(fragmentshader, GL_COMPILE_STATUS, &result);
    if (!result)
    {
        glGetShaderInfoLog(fragmentshader, 512, NULL, errorLog);
        cerr << "ERROR: fragment shader ������ ����\n" << errorLog << endl;
        exit(-1);
    }

}
GLuint make_shaderProgram()
{
    GLint result;
    GLchar errorLog[512];
    GLuint ShaderProgramID;
    ShaderProgramID = glCreateProgram(); //--- ���̴� ���α׷� �����
    glAttachShader(ShaderProgramID, vertexshader); //--- ���̴� ���α׷��� ���ؽ� ���̴� ���̱�
    glAttachShader(ShaderProgramID, fragmentshader); //--- ���̴� ���α׷��� �����׸�Ʈ ���̴� ���̱�
    glLinkProgram(ShaderProgramID); //--- ���̴� ���α׷� ��ũ�ϱ�

    glDeleteShader(vertexshader); //--- ���̴� ��ü�� ���̴� ���α׷��� ��ũ��������, ���̴� ��ü ��ü�� ���� ����
    glDeleteShader(fragmentshader);

    glGetProgramiv(ShaderProgramID, GL_LINK_STATUS, &result); // ---���̴��� �� ����Ǿ����� üũ�ϱ�
    if (!result) {
        glGetProgramInfoLog(ShaderProgramID, 512, NULL, errorLog);
        cerr << "ERROR: shader program ���� ����\n" << errorLog << endl;
        exit(-1);
    }
    glUseProgram(ShaderProgramID); //--- ������� ���̴� ���α׷� ����ϱ�
    //--- ���� ���� ���̴����α׷� ���� �� �ְ�, �� �� �Ѱ��� ���α׷��� ����Ϸ���
    //--- glUseProgram �Լ��� ȣ���Ͽ� ��� �� Ư�� ���α׷��� �����Ѵ�.
    //--- ����ϱ� ������ ȣ���� �� �ִ�.
    return ShaderProgramID;
}
void InitShader()
{
    make_vertexShaders(); //--- ���ؽ� ���̴� �����
    make_fragmentShaders(); //--- �����׸�Ʈ ���̴� �����
    shaderID = make_shaderProgram(); //--- ���̴� ���α׷� �����
}
GLvoid InitBuffer() {
    //--- VAO ��ü ���� �� ���ε�
    glGenVertexArrays(1, &VAO);
    //--- vertex data ������ ���� VBO ���� �� ���ε�.
    glGenBuffers(2, VBO);
}

void main(int argc, char** argv) //--- ������ ����ϰ� �ݹ��Լ� ���� { //--- ������ �����ϱ�
{
    srand((unsigned int)time(NULL));
    glutInit(&argc, argv); // glut �ʱ�ȭ
    glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGBA | GLUT_DEPTH); // ���÷��� ��� ����
    glutInitWindowPosition(0, 0); // �������� ��ġ ����
    glutInitWindowSize(WINDOWX, WINDOWY); // �������� ũ�� ����
    glutCreateWindow("�ǽ� 15");// ������ ����   (������ �̸�)
    //--- GLEW �ʱ�ȭ�ϱ�
    glewExperimental = GL_TRUE;
    glewInit();

    InitShader();
    InitBuffer();

    glutMouseFunc(Mouse);
    glutMotionFunc(Motion);
    glutKeyboardFunc(keyboard);
    glutTimerFunc(10, TimerFunction, 1);

    glutDisplayFunc(drawScene); //--- ��� �ݹ� �Լ�
    glutReshapeFunc(Reshape);
    glutMainLoop();
}

float rotate_x = 30.0;
float rotate_y = -30.0;
float tx = 0.0, ty = 0.0, tz = 0.0;
GLvoid drawScene() //--- �ݹ� �Լ�: �׸��� �ݹ� �Լ� 
{
    if (start) {
        start = FALSE;
        Time = FALSE; TimeLoop = TRUE;
        sel = 0;

        glEnable(GL_DEPTH_TEST);

        FL = fopen("cube.obj", "rt");
        ReadObj(FL);
        fclose(FL);
        Fvertex[0] = (Plane*)malloc(sizeof(Plane) * faceNum);
        vectoplane(Fvertex[0]);
        planecolorset(Fvertex[0], 0);

        FL = fopen("Pyramid.obj", "rt");
        ReadObj(FL);
        fclose(FL);
        Fvertex[1] = (Plane*)malloc(sizeof(Plane) * faceNum);
        vectoplane(Fvertex[1]);
        planecolorset(Fvertex[1], 1);

    } // �ʱ�ȭ�� ������
    glClearColor(BackGround[0], BackGround[1], BackGround[2], 1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    glUseProgram(shaderID);
    glBindVertexArray(VAO);// ���̴� , ���� �迭 ���

    glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGB | GLUT_DEPTH);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    TR = glm::mat4(1.0f);
    unsigned int modelLocation = glGetUniformLocation(shaderID, "modelTransform");
    glUniformMatrix4fv(modelLocation, 1, GL_FALSE, glm::value_ptr(TR));

    // �׸��� �ڵ�
    for (int i = 0; i < 6; ++i) {
        for (int j = 0; j < 6; ++j) {
            xl.color[j] = 0.0;
            yl.color[j] = 0.0;
        }
    }
    xl.Bind();
    xl.Draw();
    yl.Bind();
    yl.Draw();

    if (mode == true) {
        TR = glm::translate(TR, glm::vec3(tx, ty, tz));
        TR = glm::scale(TR, glm::vec3(0.5f, 0.5f, 0.5f));
        TR = glm::rotate(TR, (float)glm::radians(rotate_x), glm::vec3(1.0f, 0.0f, 0.0f));
        TR = glm::rotate(TR, (float)glm::radians(rotate_y), glm::vec3(0.0f, 1.0f, 0.0f));
    }
    else {
        TR = glm::translate(TR, glm::vec3(tx, ty, tz));
        TR = glm::scale(TR, glm::vec3(0.5f, 0.5f, 0.5f));
        TR = glm::rotate(TR, (float)glm::radians(rotate_x), glm::vec3(1.0f, 0.0f, 0.0f));
        TR = glm::rotate(TR, (float)glm::radians(rotate_y), glm::vec3(0.0f, 1.0f, 0.0f));
    }


    modelLocation = glGetUniformLocation(shaderID, "modelTransform");
    glUniformMatrix4fv(modelLocation, 1, GL_FALSE, glm::value_ptr(TR));

    if (mode == true) {
        for (int i = 0; i < 12; ++i) {
            Fvertex[0][i].Bind();
            Fvertex[0][i].Draw();
        }
    }
    else {
        for (int i = 0; i < 6; ++i) {
            Fvertex[1][i].Bind();
            Fvertex[1][i].Draw();
        }
    }

    glutSwapBuffers();
    glutPostRedisplay();
}
GLvoid Reshape(int w, int h) //--- �ݹ� �Լ�: �ٽ� �׸��� �ݹ� �Լ�
{
    glViewport(0, 0, w, h);
}

void Mouse(int button, int state, int x, int y)
{
    if (button == GLUT_LEFT_BUTTON && state == GLUT_DOWN) {
        mx = ((float)x - ((float)WINDOWX / (float)2)) / ((float)WINDOWX / (float)2);
        my = -((float)y - ((float)WINDOWY / (float)2)) / ((float)WINDOWY / (float)2);
        Click = 1;
    }
    if (button == GLUT_LEFT_BUTTON && state == GLUT_UP) {
        Click = 0;
    }
}

void Motion(int x, int y)
{
    if (Click == 1) {
        mx = ((float)x - ((float)WINDOWX / (float)2)) / ((float)WINDOWX / (float)2);
        my = -((float)y - ((float)WINDOWY / (float)2)) / ((float)WINDOWY / (float)2);
    }
}

bool x_move_plus = false; bool x_move_minus = false;
bool y_move_plus = false; bool y_move_minus = false;
int culling_cnt = 0;
void keyboard(unsigned char key, int x, int y) {
    switch (key) {
    case 'c':
        mode = true;
        break;
    case 'p':
        mode = false;
        break;
    case 'w':
        line = true;
        break;
    case 'W':
        line = false;
        break;
    case 'x': // x�� ���� ���� �ѱ�
        x_move_plus = true; x_move_minus = false;
        y_move_plus = false; y_move_minus = false;
        break;
    case 'X': // x�� ���� ���� ����
        x_move_plus = false; x_move_minus = true;
        y_move_plus = false; y_move_minus = false;
        break;
    case 'y': // y�� ���� ���� �ѱ�
        x_move_plus = false; x_move_minus = false;
        y_move_plus = true; y_move_minus = false;
        break;
    case 'Y': // y�� ���� ���� ����
        x_move_plus = false; x_move_minus = false;
        y_move_plus = false; y_move_minus = true;
        break;
    case 's': // �ʱ�ȭ
        x_move_plus = false; x_move_minus = false;
        y_move_plus = false; y_move_minus = false;
        tx = 0.0; ty = 0.0;
        break;
    case 'h': // ���� ����
        if (culling_cnt % 2 == 0)culling = true;
        else culling = false;
        culling_cnt++;
        break;
    case 'j':
        tx -= 0.01;
        break;
    case 'l':
        tx += 0.01;
        break;
    case 'i':
        ty += 0.01;
        break;
    case 'k':
        ty -= 0.01;
        break;
    }

    glutPostRedisplay();
}

void TimerFunction(int value) {
    if (x_move_plus)rotate_x += 0.5;
    else if (x_move_minus)rotate_x -= 0.5;
    else if (y_move_plus) rotate_y += 0.5;
    else if (y_move_minus)rotate_y -= 0.5;
    glutPostRedisplay();
    glutTimerFunc(10, TimerFunction, 1);
}

void ReadObj(FILE* objFile)
{
    faceNum = 0;
    //--- 1. ��ü ���ؽ� ���� �� �ﰢ�� ���� ����
    char count[100];
    char bind[100];
    int vertexNum = 0;
    while (!feof(objFile)) {
        fscanf(objFile, "%s", count);
        if (count[0] == 'v' && count[1] == '\0')
            vertexNum += 1;
        else if (count[0] == 'f' && count[1] == '\0')
            faceNum += 1;
        memset(count, '\0', sizeof(count));
    }
    int vertIndex = 0;
    int faceIndex = 0;
    vertex = (glm::vec4*)malloc(sizeof(glm::vec4) * vertexNum);
    face = (glm::vec4*)malloc(sizeof(glm::vec4) * faceNum);

    fseek(objFile, 0, 0);
    while (!feof(objFile)) {
        fscanf(objFile, "%s", bind);
        if (bind[0] == 'v' && bind[1] == '\0') {
            fscanf(objFile, "%f %f %f",
                &vertex[vertIndex].x, &vertex[vertIndex].y, &vertex[vertIndex].z);
            vertIndex++;
        }
        else if (bind[0] == 'f' && bind[1] == '\0') {
            fscanf(objFile, "%f %f %f",
                &face[faceIndex].x, &face[faceIndex].y, &face[faceIndex].z);
            int x = face[faceIndex].x - 1, y = face[faceIndex].y - 1, z = face[faceIndex].z - 1;
            faceIndex++;
        }
    }
}

void vectoplane(Plane* p) {
    for (int i = 0; i < faceNum; ++i) {
        int x = face[i].x - 1, y = face[i].y - 1, z = face[i].z - 1;
        p[i].p[0] = vertex[x].x;
        p[i].p[1] = vertex[x].y;
        p[i].p[2] = vertex[x].z;

        p[i].p[3] = vertex[y].x;
        p[i].p[4] = vertex[y].y;
        p[i].p[5] = vertex[y].z;

        p[i].p[6] = vertex[z].x;
        p[i].p[7] = vertex[z].y;
        p[i].p[8] = vertex[z].z;
    }
}

void planecolorset(Plane* p, int a) {
    if (a == 0) {
        for (int i = 0; i < faceNum; i += 2) {
            float R = uid(dre);
            float G = uid(dre);
            float B = uid(dre);
            for (int j = 0; j < 3; ++j) {
                p[i].color[j * 3] = R;
                p[i].color[j * 3 + 1] = G;
                p[i].color[j * 3 + 2] = B;

                p[i + 1].color[j * 3] = R;
                p[i + 1].color[j * 3 + 1] = G;
                p[i + 1].color[j * 3 + 2] = B;
            }
        }
    }
    else if (a == 1) {
        for (int i = 0; i < faceNum; ++i) {
            float R = uid(dre);
            float G = uid(dre);
            float B = uid(dre);
            for (int j = 0; j < 3; ++j) {
                p[i].color[j * 3] = R;
                p[i].color[j * 3 + 1] = G;
                p[i].color[j * 3 + 2] = B;
            }
        }
    }
}
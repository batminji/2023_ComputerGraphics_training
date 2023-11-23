#define _CRT_SECURE_NO_WARNINGS //--- 프로그램 맨 앞에 선언할 것
#pragma comment(lib, "glew32.lib")
#pragma comment(lib, "freeglut.lib")
#include <iostream>
#include <gl/glew.h>
#include <gl/freeglut.h>
#include <gl/freeglut_ext.h>
#include <time.h>
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

GLchar* vertexsource, * fragmentsource; //--- 소스코드 저장 변수
GLuint vertexshader, fragmentshader; //--- 세이더 객체

void make_vertexShaders();
void make_fragmentShaders();
GLuint make_shaderProgram();
GLvoid InitBuffer();
void InitShader();

GLuint shaderID;
GLint width, height;

GLuint vertexShader;
GLuint fragmentShader;

GLUquadricObj* qobj;

GLuint VAO, VBO[2];
bool Cbool = false, Xplus = false, Xminus = false, Yplus = false, Yminus = false, Rbool = false, rbool = false;
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
        glLineWidth(2.0);
        glDrawArrays(GL_LINE_STRIP, 0, 2);
    }
};

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
        glLineWidth(2.0);
        glDrawArrays(GL_LINE_LOOP, 0, 3);
    }
};

float rotate_x1, rotate_x2, rotate_y1, rotate_y2;
float rotatexx, rotateyy;

float mx, my, px, py;
float cnt;
int sel;
char comm;

BOOL Time, start = TRUE, TimeLoop;

float BackGround[] = { 1.0, 1.0, 1.0 };
Line xl = {
   0, 1, 0,
   0, -1, 0,

   0, 1, 0,
   0, 1, 0,
};
Line yl = {
   1, 0, 0,
   -1, 0, 0,

   1, 0, 0,
   1, 0, 0,
};
Line zl = {
   0, 0, 1,
   0, 0, -1,

   0, 0, 1,
   0, 0, 1,
};
glm::vec4* vertex;
glm::vec4* face;

Plane* Fvertex[2];

glm::mat4 TR = glm::mat4(1.0f);
GLUquadricObj* obj;
FILE* FL;
int faceNum = 0;
int Click = 0;

float movex, movey = 0;
float movexx, moveyy = 0;

void ReadObj(FILE* objFile);
void keyboard(unsigned char, int, int);
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
    vertexShader = glCreateShader(GL_VERTEX_SHADER);
    glShaderSource(vertexShader, 1, (const GLchar**)&vertexsource, NULL);
    glCompileShader(vertexShader);

    GLint result;
    GLchar errorLog[512];
    glGetShaderiv(vertexShader, GL_COMPILE_STATUS, &result);
    if (!result)
    {
        glGetShaderInfoLog(vertexShader, 512, NULL, errorLog);
        cerr << "ERROR: vertex shader 컴파일 실패\n" << errorLog << endl;
        exit(-1);
    }
}
void make_fragmentShaders()
{
    fragmentsource = filetobuf("fragment.glsl");

    fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);
    glShaderSource(fragmentShader, 1, &fragmentsource, NULL);
    glCompileShader(fragmentShader);



    GLint result;
    GLchar errorLog[512];
    glGetShaderiv(fragmentShader, GL_COMPILE_STATUS, &result);
    if (!result)
    {
        glGetShaderInfoLog(fragmentShader, 512, NULL, errorLog);
        cerr << "ERROR: fragment shader 컴파일 실패\n" << errorLog << endl;
        exit(-1);
    }

}
GLuint make_shaderProgram()
{
    GLint result;
    GLchar errorLog[512];
    GLuint ShaderProgramID;
    ShaderProgramID = glCreateProgram(); //--- 세이더 프로그램 만들기
    glAttachShader(ShaderProgramID, vertexShader); //--- 세이더 프로그램에 버텍스 세이더 붙이기
    glAttachShader(ShaderProgramID, fragmentShader); //--- 세이더 프로그램에 프래그먼트 세이더 붙이기
    glLinkProgram(ShaderProgramID); //--- 세이더 프로그램 링크하기

    glDeleteShader(vertexShader); //--- 세이더 객체를 세이더 프로그램에 링크했음으로, 세이더 객체 자체는 삭제 가능
    glDeleteShader(fragmentShader);

    glGetProgramiv(ShaderProgramID, GL_LINK_STATUS, &result); // ---세이더가 잘 연결되었는지 체크하기
    if (!result) {
        glGetProgramInfoLog(ShaderProgramID, 512, NULL, errorLog);
        cerr << "ERROR: shader program 연결 실패\n" << errorLog << endl;
        exit(-1);
    }
    glUseProgram(ShaderProgramID); //--- 만들어진 세이더 프로그램 사용하기
    //--- 여러 개의 세이더프로그램 만들 수 있고, 그 중 한개의 프로그램을 사용하려면
    //--- glUseProgram 함수를 호출하여 사용 할 특정 프로그램을 지정한다.
    //--- 사용하기 직전에 호출할 수 있다.
    return ShaderProgramID;
}
void InitShader()
{
    make_vertexShaders(); //--- 버텍스 세이더 만들기
    make_fragmentShaders(); //--- 프래그먼트 세이더 만들기
    shaderID = make_shaderProgram(); //--- 세이더 프로그램 만들기
}
GLvoid InitBuffer() {
    //--- VAO 객체 생성 및 바인딩
    glGenVertexArrays(1, &VAO);
    //--- vertex data 저장을 위한 VBO 생성 및 바인딩.
    glGenBuffers(2, VBO);
}

void main(int argc, char** argv) //--- 윈도우 출력하고 콜백함수 설정 { //--- 윈도우 생성하기
{
    srand((unsigned int)time(NULL));
    glutInit(&argc, argv); // glut 초기화
    glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGBA | GLUT_DEPTH); // 디스플레이 모드 설정
    glutInitWindowPosition(0, 0); // 윈도우의 위치 지정
    glutInitWindowSize(WINDOWX, WINDOWY); // 윈도우의 크기 지정
    glutCreateWindow("실습 16");// 윈도우 생성   (윈도우 이름)
    //--- GLEW 초기화하기
    glewExperimental = GL_TRUE;
    glewInit();

    InitShader();
    InitBuffer();

    glutKeyboardFunc(keyboard);
    glutTimerFunc(10, TimerFunction, 1);
    glutDisplayFunc(drawScene); //--- 출력 콜백 함수
    glutReshapeFunc(Reshape);
    glutMainLoop();
}

GLvoid drawScene() //--- 콜백 함수: 그리기 콜백 함수 
{
    if (start) {
        start = FALSE;
        sel = 0;
        glEnable(GL_DEPTH_TEST);
        FL = fopen("cube.obj", "rt");
        ReadObj(FL);
        fclose(FL);
        Fvertex[0] = (Plane*)malloc(sizeof(Plane) * faceNum);
        vectoplane(Fvertex[0]);
        planecolorset(Fvertex[0], 0);
    } // 초기화할 데이터
    glEnable(GL_DEPTH_TEST);

    glClearColor(BackGround[0], BackGround[1], BackGround[2], 1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);


    glUseProgram(shaderID);
    glBindVertexArray(VAO);// 쉐이더 , 버퍼 배열 사용

    glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGB | GLUT_DEPTH);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    TR = glm::mat4(1.0f);
    unsigned int modelLocation = glGetUniformLocation(shaderID, "modelTransform");
    glUniformMatrix4fv(modelLocation, 1, GL_FALSE, glm::value_ptr(TR));

    // 그리기 코드
    TR = glm::mat4(1.0f);
    TR = glm::rotate(TR, (float)glm::radians(20.0), glm::vec3(1.0f, 0.0f, 0.0f));
    TR = glm::rotate(TR, (float)glm::radians(-20.0), glm::vec3(0.0f, 1.0f, 0.0f));
    glUniformMatrix4fv(modelLocation, 1, GL_FALSE, glm::value_ptr(TR));
    xl.Bind();
    xl.Draw();
    TR = glm::mat4(1.0f);
    TR = glm::rotate(TR, (float)glm::radians(20.0), glm::vec3(1.0f, 0.0f, 0.0f));
    TR = glm::rotate(TR, (float)glm::radians(-20.0), glm::vec3(0.0f, 1.0f, 0.0f));
    glUniformMatrix4fv(modelLocation, 1, GL_FALSE, glm::value_ptr(TR));
    yl.Bind();
    yl.Draw();
    TR = glm::mat4(1.0f);
    TR = glm::rotate(TR, (float)glm::radians(20.0), glm::vec3(1.0f, 0.0f, 0.0f));
    TR = glm::rotate(TR, (float)glm::radians(-20.0), glm::vec3(0.0f, 1.0f, 0.0f));
    glUniformMatrix4fv(modelLocation, 1, GL_FALSE, glm::value_ptr(TR));
    zl.Bind();
    zl.Draw();
    // x, y, z 라인

    // 정육면체 실린더
    TR = glm::mat4(1.0f);

    TR = glm::rotate(TR, (float)glm::radians(20.0), glm::vec3(1.0f, 0.0f, 0.0f));
    TR = glm::rotate(TR, (float)glm::radians(-20.0), glm::vec3(0.0f, 1.0f, 0.0f));
    TR = glm::rotate(TR, (float)glm::radians(rotatexx), glm::vec3(0.0f, 1.0f, 0.0f));
    TR = glm::translate(TR, glm::vec3(0.5, 0.0, 0.0f));
    TR = glm::rotate(TR, (float)glm::radians(30.0 + rotate_x1), glm::vec3(1.0f, 0.0f, 0.0f));
    TR = glm::rotate(TR, (float)glm::radians(-30.0 + rotate_y1), glm::vec3(0.0f, 1.0f, 0.0f));

    if (!Cbool)
        TR = glm::scale(TR, glm::vec3(0.3f, 0.3f, 0.3f));
    else
        TR = glm::scale(TR, glm::vec3(0.2f, 0.2f, 0.2f));

    modelLocation = glGetUniformLocation(shaderID, "modelTransform");
    glUniformMatrix4fv(modelLocation, 1, GL_FALSE, glm::value_ptr(TR));

    if (!Cbool) {
        for (int i = 0; i < 12; ++i) {
            Fvertex[0][i].Bind();
            Fvertex[0][i].Draw();
        }
    }
    else{
        qobj = gluNewQuadric();
        gluQuadricDrawStyle(qobj, GLU_LINE);
        gluCylinder(qobj, 1.0, 1.0, 2.0, 20, 8);
    }

    // 구, 원뿔
    TR = glm::mat4(1.0f);

    TR = glm::rotate(TR, (float)glm::radians(20.0), glm::vec3(1.0f, 0.0f, 0.0f));
    TR = glm::rotate(TR, (float)glm::radians(-20.0), glm::vec3(0.0f, 1.0f, 0.0f));
    TR = glm::rotate(TR, (float)glm::radians(rotatexx), glm::vec3(0.0f, 1.0f, 0.0f));
    TR = glm::translate(TR, glm::vec3(-0.5f, 0, 0.0f));
    TR = glm::rotate(TR, (float)glm::radians(30.0 + rotate_x2), glm::vec3(1.0f, 0.0f, 0.0f));
    TR = glm::rotate(TR, (float)glm::radians(-30.0 + rotate_y2), glm::vec3(0.0f, 1.0f, 0.0f));
    TR = glm::scale(TR, glm::vec3(0.15f, 0.15f, 0.15f));

    glUniformMatrix4fv(modelLocation, 1, GL_FALSE, glm::value_ptr(TR));
    if (!Cbool)
    {
        qobj = gluNewQuadric();
        gluQuadricDrawStyle(qobj, GLU_LINE);
        gluSphere(qobj, 1.5, 50, 50);
    }
    else
    {
        qobj = gluNewQuadric();
        gluQuadricDrawStyle(qobj, GLU_LINE);
        gluCylinder(qobj, 1.0, 0.0, 2.0, 20, 8); // 원뿔
    }

    glutSwapBuffers();
    glutPostRedisplay();
}
GLvoid Reshape(int w, int h) //--- 콜백 함수: 다시 그리기 콜백 함수
{
    glViewport(0, 0, w, h);
}

bool left_shape = true, right_shape = true;
void keyboard(unsigned char key, int x, int y) {
    switch (key) {
    case 'x':
        Xminus = false;
        Xplus = !Xplus;
        break;
    case 'X':
        Xplus = false;
        Xminus = !Xminus;
        break;
    case 'y':
        Yminus = false;
        Yplus = !Yplus;
        break;
    case 'Y':
        Yplus = false;
        Yminus = !Yminus;
        break;
    case '1':
        left_shape = false; right_shape = true;
        break;
    case '2':
        left_shape = true; right_shape = false;
        break;
    case '3':
        left_shape = true; right_shape = true;
        break;
    case 'r':
        rbool = !rbool;
        Rbool = false;
        break;
    case 'R':
        Rbool = !Rbool;
        rbool = false;
        break;
    case 'c':
        Cbool = !Cbool;
        break;
    case 's':
        Cbool = false;
        Xminus = false; Xplus = false;
        Yminus = false; Yplus = false;        
        rotate_x1 = 0; rotate_y1 = 0;
        rotate_x2 = 0; rotate_y2 = 0;
        rotatexx = 0;
        rbool = false; Rbool = false;
        break;
    }
    glutPostRedisplay();
}

void TimerFunction(int value)
{
    if (Xplus) {
        if (left_shape)rotate_x1 += 1.0;
        if(right_shape)rotate_x2 += 1.0;
    }
    else if (Xminus) {
        if (left_shape)rotate_x1 -= 1.0;
        if (right_shape)rotate_x2 -= 1.0;
    }
    if (Yplus) {
        if (left_shape)rotate_y1 += 1.0;
        if (right_shape)rotate_y2 += 1.0;
    }
    else if (Yminus) {
        if (left_shape)rotate_y1 -= 1.0;
        if (right_shape)rotate_y2 -= 1.0;
    }
    if (rbool)
        rotatexx += 1.0;
    if (Rbool)
        rotatexx -= 1.0;
    glutPostRedisplay();
    glutTimerFunc(10, TimerFunction, 1);
}

void ReadObj(FILE* objFile)
{
    faceNum = 0;
    //--- 1. 전체 버텍스 개수 및 삼각형 개수 세기
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
        float rgb[8][3]{};
        for (int i = 0; i < 8; ++i)
        {
            for (int j = 0; j < 3; ++j)
            {
                rgb[i][j] = uid(dre);
            }
        }
        for (int k = 0; k < 3; ++k)
        {
            p[0].color[k] = rgb[0][k];
            p[1].color[k] = rgb[0][k];
            p[2].color[k] = rgb[0][k];
            p[3].color[k] = rgb[0][k];
            p[4].color[k] = rgb[2][k];
            p[5].color[k] = rgb[2][k];
            p[6].color[k] = rgb[4][k];
            p[7].color[k] = rgb[4][k];
            p[8].color[k] = rgb[0][k];
            p[9].color[k] = rgb[0][k];
            p[10].color[k] = rgb[1][k];
            p[11].color[k] = rgb[1][k];
        }
        for (int k = 3; k < 6; ++k)
        {
            p[0].color[k] = rgb[6][k - 3];
            p[1].color[k] = rgb[2][k - 3];
            p[2].color[k] = rgb[3][k - 3];
            p[3].color[k] = rgb[1][k - 3];
            p[4].color[k] = rgb[7][k - 3];
            p[5].color[k] = rgb[3][k - 3];
            p[6].color[k] = rgb[6][k - 3];
            p[7].color[k] = rgb[7][k - 3];
            p[8].color[k] = rgb[4][k - 3];
            p[9].color[k] = rgb[5][k - 3];
            p[10].color[k] = rgb[5][k - 3];
            p[11].color[k] = rgb[7][k - 3];

        }
        for (int k = 6; k < 9; ++k)
        {
            p[0].color[k] = rgb[4][k - 6];
            p[1].color[k] = rgb[6][k - 6];
            p[2].color[k] = rgb[2][k - 6];
            p[3].color[k] = rgb[3][k - 6];
            p[4].color[k] = rgb[6][k - 6];
            p[5].color[k] = rgb[7][k - 6];
            p[6].color[k] = rgb[7][k - 6];
            p[7].color[k] = rgb[5][k - 6];
            p[8].color[k] = rgb[5][k - 6];
            p[9].color[k] = rgb[1][k - 6];
            p[10].color[k] = rgb[7][k - 6];
            p[11].color[k] = rgb[3][k - 6];
        }
    }
}
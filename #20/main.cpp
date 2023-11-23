#define _CRT_SECURE_NO_WARNINGS
#pragma comment(lib, "glew32.lib")
#pragma comment(lib, "freeglut.lib")
#include <iostream>
#include <gl/glew.h>
#include <gl/freeglut.h>
#include <gl/freeglut_ext.h>
#include <time.h>
#include <stdlib.h>
#include <cmath>
#include <glm/glm.hpp>
#include <glm/ext.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <random>

#define WINDOWX 800
#define WINDOWY 800
#define pie 3.141592

using namespace std;

random_device rd;
default_random_engine dre(rd());
uniform_real_distribution<float>uid(0, 1);

void make_vertexShaders();
void make_fragmentShaders();
GLuint make_shaderProgram();
GLvoid InitBuffer();
void InitShader();
GLchar* filetobuf(const char* file);

GLuint shaderID;
GLint width, height;

GLuint vertexShader;
GLuint fragmentShader;

GLuint VAO, VBO[2];
class Line {
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
        glDrawArrays(GL_TRIANGLES, 0, 3);
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
        glDrawArrays(GL_TRIANGLES, 0, 3);
    }

};

BOOL Time, start = TRUE, TimeLoop;

float BackGround[] = { 0.0, 0.0, 0.0 };
Plane* Fvertex[6];
Line ground[2];

glm::vec4* vertex;
glm::vec4* face;
glm::vec3* outColor;


glm::mat4 TR = glm::mat4(1.0f);


FILE* FL;
int faceNum = 0;
int Click = 0;
float mx, my, mz = 0;
float bx = 0;
float tx = 0;
float yy = 0;
float ry = 0;
float zz = 1;
float xx = 0;
int bmove = 0;
int mturn = 0;
int tturn = 0;
int yturn = 0;
int rturn = 0;

float f_turn1 = 0.0f;
float f_x_move = 0.05f;

void ReadObj(FILE* objFile);
void keyboard(unsigned char, int, int);
void Mouse(int button, int state, int x, int y);
void Motion(int x, int y);
void TimerFunction(int value);
GLvoid drawScene(GLvoid);
GLvoid Reshape(int w, int h);
void vectoplane(Plane* p);
void planecolorset(Plane* p, int a);

void make_vertexShaders()
{

    GLchar* vertexShaderSource;

    vertexShaderSource = filetobuf("vertex.glsl");

    vertexShader = glCreateShader(GL_VERTEX_SHADER);
    glShaderSource(vertexShader, 1, &vertexShaderSource, NULL);
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
    const GLchar* fragmentShaderSource = {
       "#version 330 core\n"
       "out vec4 fColor; \n"
       "in vec3 outColor;\n"
       "void main() \n"
       "{\n"
       "fColor = vec4(outColor, 1.0); \n"
       "} \0"
    };


    // fragmentShaderSource = filetobuf("fragment.glsl");

    fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);
    glShaderSource(fragmentShader, 1, &fragmentShaderSource, NULL);
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
    glutCreateWindow("실습 20");// 윈도우 생성   (윈도우 이름)
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
        Time = FALSE; TimeLoop = TRUE;

        FL = fopen("cube.obj", "rt");
        ReadObj(FL);
        fclose(FL);
        Fvertex[0] = (Plane*)malloc(sizeof(Plane) * faceNum);
        vectoplane(Fvertex[0]);
        planecolorset(Fvertex[0], 0);

        FL = fopen("cube.obj", "rt");
        ReadObj(FL);
        fclose(FL);
        Fvertex[1] = (Plane*)malloc(sizeof(Plane) * faceNum);
        vectoplane(Fvertex[1]);
        planecolorset(Fvertex[1], 0);

        FL = fopen("cube.obj", "rt");
        ReadObj(FL);
        fclose(FL);
        Fvertex[2] = (Plane*)malloc(sizeof(Plane) * faceNum);
        vectoplane(Fvertex[2]);
        planecolorset(Fvertex[2], 0);

        FL = fopen("cube.obj", "rt");
        ReadObj(FL);
        fclose(FL);
        Fvertex[3] = (Plane*)malloc(sizeof(Plane) * faceNum);
        vectoplane(Fvertex[3]);
        planecolorset(Fvertex[3], 0);

        FL = fopen("cube.obj", "rt");
        ReadObj(FL);
        fclose(FL);
        Fvertex[4] = (Plane*)malloc(sizeof(Plane) * faceNum);
        vectoplane(Fvertex[4]);
        planecolorset(Fvertex[4], 0);

        FL = fopen("cube.obj", "rt");
        ReadObj(FL);
        fclose(FL);
        Fvertex[5] = (Plane*)malloc(sizeof(Plane) * faceNum);
        vectoplane(Fvertex[5]);
        planecolorset(Fvertex[5], 0);

        for (int j = 0; j < 2; ++j) {
            for (int k = 0; k < 9; ++k) {
                ground[j].color[k] = 0.7;
                if (k == 0 || k == 3 || k == 6) {
                    ground[j].color[k] = 1.0;
                }
            }
        }

        ground[0].p[0] = -1.0;
        ground[0].p[1] = 0;
        ground[0].p[2] = -1.0;

        ground[0].p[3] = 1.0;
        ground[0].p[4] = 0;
        ground[0].p[5] = -1.0;

        ground[0].p[6] = -1.0;
        ground[0].p[7] = 0;
        ground[0].p[8] = 1.0;

        ground[1].p[0] = 1.0;
        ground[1].p[1] = 0;
        ground[1].p[2] = -1.0;

        ground[1].p[3] = 1.0;
        ground[1].p[4] = 0;
        ground[1].p[5] = 1.0;

        ground[1].p[6] = -1.0;
        ground[1].p[7] = 0;
        ground[1].p[8] = 1.0;

        glEnable(GL_DEPTH_TEST);

    } // 초기화할 데이터

    glViewport(0, 0, 800, 800);
    glClearColor(0.0, 0.0, 0.0, 0.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);    //배경

    glUseProgram(shaderID);
    glBindVertexArray(VAO);// 쉐이더 , 버퍼 배열 사용

    glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGB | GLUT_DEPTH);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);


    unsigned int color = glGetUniformLocation(shaderID, "outColor");
    unsigned int modelLocation = glGetUniformLocation(shaderID, "model");
    unsigned int viewLocation = glGetUniformLocation(shaderID, "view");
    unsigned int projLocation = glGetUniformLocation(shaderID, "projection");

    glm::mat4 Vw = glm::mat4(1.0f);
    glm::mat4 Cp = glm::mat4(1.0f);

    Cp = glm::rotate(Cp, (float)glm::radians(ry), glm::vec3(0.0f, 1.0f, 0.0f));

    glm::vec3 cameraPos = glm::vec4(xx, 0.1, zz, 1.0f) * Cp;
    glm::vec3 cameraDirection = glm::vec4(sin((float)glm::radians(yy)), -0.0, -cos((float)glm::radians(yy)), 1.0f) * Cp;
    glm::vec3 cameraUp = glm::vec3(0.0f, 1.0f, 0.0f);

    Vw = glm::lookAt(cameraPos, cameraPos + cameraDirection, cameraUp);
    glUniformMatrix4fv(viewLocation, 1, GL_FALSE, &Vw[0][0]);

    glm::mat4 Pj = glm::mat4(1.0f);
    Pj = glm::perspective(glm::radians(45.0f), (float)WINDOWX / (float)WINDOWY, 0.1f, 100.0f);

    glUniformMatrix4fv(projLocation, 1, GL_FALSE, &Pj[0][0]);

    // 그리기 코드

    TR = glm::mat4(1.0f);
    modelLocation = glGetUniformLocation(shaderID, "model");
    glUniformMatrix4fv(modelLocation, 1, GL_FALSE, glm::value_ptr(TR));
    for (int i = 0; i < 2; ++i) {
        ground[i].Bind();
        ground[i].Draw();
    }                                                                     // 바닥 그리기

    TR = glm::mat4(1.0f);
    modelLocation = glGetUniformLocation(shaderID, "model");
    TR = glm::translate(TR, glm::vec3(0.0f, 0.015f, bx));
    TR = glm::scale(TR, glm::vec3(0.2f, 0.03f, 0.2f));
    glUniformMatrix4fv(modelLocation, 1, GL_FALSE, glm::value_ptr(TR));


    for (int i = 0; i < 12; ++i) {
        Fvertex[0][i].Bind();
        Fvertex[0][i].Draw();
    }                                                                                       // 아래 몸체

    TR = glm::mat4(1.0f);
    modelLocation = glGetUniformLocation(shaderID, "model");

    TR = glm::translate(TR, glm::vec3(0.0f, 0.055f, bx));
    TR = glm::rotate(TR, (float)glm::radians(my), glm::vec3(0.0f, 1.0f, 0.0f));
    TR = glm::scale(TR, glm::vec3(0.1f, 0.05f, 0.1f));
    glUniformMatrix4fv(modelLocation, 1, GL_FALSE, glm::value_ptr(TR));

    for (int i = 0; i < 12; ++i) {
        Fvertex[1][i].Bind();
        Fvertex[1][i].Draw();
    }                                                                                       // 중앙 몸체

    TR = glm::mat4(1.0f);
    modelLocation = glGetUniformLocation(shaderID, "model");

    /*TR = glm::translate(TR, glm::vec3(0.0f, 0.0f, bx));
    TR = glm::rotate(TR, (float)glm::radians(f_turn1), glm::vec3(0.0f, 1.0f, 0.0f));
    TR = glm::translate(TR, glm::vec3(0.0f, 0.0f, -bx));*/

    TR = glm::translate(TR, glm::vec3(0.0f, 0.0f, bx));
    TR = glm::rotate(TR, (float)glm::radians(-f_turn1), glm::vec3(0.0f, 1.0f, 0.0f));
    TR = glm::translate(TR, glm::vec3(-0.0f, 0.0f, -bx));

    TR = glm::translate(TR, glm::vec3(-f_x_move, 0.01f, bx + 0.2f));

    TR = glm::scale(TR, glm::vec3(0.02f, 0.02f, 0.35f));
    glUniformMatrix4fv(modelLocation, 1, GL_FALSE, glm::value_ptr(TR));

    for (int i = 0; i < 12; ++i) {
        Fvertex[4][i].Bind();
        Fvertex[4][i].Draw();
    }                                                // 왼쪽 포신

    TR = glm::mat4(1.0f);
    modelLocation = glGetUniformLocation(shaderID, "model");

    TR = glm::translate(TR, glm::vec3(0.0f, 0.0f, bx));
    TR = glm::rotate(TR, (float)glm::radians(f_turn1), glm::vec3(0.0f, 1.0f, 0.0f));
    TR = glm::translate(TR, glm::vec3(0.0f, 0.0f, -bx));

    TR = glm::translate(TR, glm::vec3(f_x_move, 0.01f, bx + 0.2f));

    TR = glm::scale(TR, glm::vec3(0.02f, 0.02f, 0.35f));
    glUniformMatrix4fv(modelLocation, 1, GL_FALSE, glm::value_ptr(TR));

    for (int i = 0; i < 12; ++i) {
        Fvertex[5][i].Bind();
        Fvertex[5][i].Draw();
    }                                                // 오른쪽 포신

    TR = glm::mat4(1.0f);
    modelLocation = glGetUniformLocation(shaderID, "model");

    TR = glm::translate(TR, glm::vec3(0.0f, 0.0f, bx));
    TR = glm::rotate(TR, (float)glm::radians(my), glm::vec3(0.0f, 1.0f, 0.0f));
    TR = glm::translate(TR, glm::vec3(0.0f, 0.0f, -bx));

    TR = glm::translate(TR, glm::vec3(-0.02f, 0.13f, bx));
    TR = glm::translate(TR, glm::vec3(0.0f, -0.04f, 0.0f));
    TR = glm::rotate(TR, (float)glm::radians(tx), glm::vec3(1.0f, 0.0f, 0.0f));
    TR = glm::translate(TR, glm::vec3(0.0f, 0.04f, 0.0f));
    TR = glm::scale(TR, glm::vec3(0.02f, 0.1f, 0.02f));
    glUniformMatrix4fv(modelLocation, 1, GL_FALSE, glm::value_ptr(TR));

    for (int i = 0; i < 12; ++i) {
        Fvertex[2][i].Bind();
        Fvertex[2][i].Draw();
    }                                                                                       // 왼 팔

    TR = glm::mat4(1.0f);
    modelLocation = glGetUniformLocation(shaderID, "model");

    TR = glm::translate(TR, glm::vec3(0.0f, 0.0f, bx));
    TR = glm::rotate(TR, (float)glm::radians(my), glm::vec3(0.0f, 1.0f, 0.0f));
    TR = glm::translate(TR, glm::vec3(0.0f, 0.0f, -bx));

    TR = glm::translate(TR, glm::vec3(0.02f, 0.13f, bx));
    TR = glm::translate(TR, glm::vec3(0.0f, -0.04f, 0.0f));
    TR = glm::rotate(TR, (float)glm::radians(-tx), glm::vec3(1.0f, 0.0f, 0.0f));
    TR = glm::translate(TR, glm::vec3(0.0f, 0.04f, 0.0f));
    TR = glm::scale(TR, glm::vec3(0.02f, 0.1f, 0.02f));
    glUniformMatrix4fv(modelLocation, 1, GL_FALSE, glm::value_ptr(TR));

    for (int i = 0; i < 12; ++i) {
        Fvertex[3][i].Bind();
        Fvertex[3][i].Draw();
    }                                                                                       // 오른 팔

    glutSwapBuffers();
    glutPostRedisplay();
}

GLvoid Reshape(int w, int h) //--- 콜백 함수: 다시 그리기 콜백 함수
{

}

bool f_timer1 = false;
bool f_timer2 = false;
bool f_timer3 = false;
int f_cnt = 0;
void keyboard(unsigned char key, int x, int y) {
    switch (key) {
    case 'b':
        if (bmove == 1) {
            bmove = 0;
        }
        else {
            bmove = 1;
        }
        break;
    case 'B':
        if (bmove == 2) {
            bmove = 0;
        }
        else {
            bmove = 2;
        }
        break;
    case 'm':
        if (mturn == 1) {
            mturn = 0;
        }
        else {
            mturn = 1;
        }
        break;
    case 'M':
        if (mturn == 2) {
            mturn = 0;
        }
        else {
            mturn = 2;
        }
        break;
    case 't':
        if (tturn == 1) {
            tturn = 0;
        }
        else {
            tturn = 1;
        }
        break;
    case 'T':
        if (tturn == 2) {
            tturn = 0;
        }
        else {
            tturn = 2;
        }
        break;
    case 'z':
        zz += 0.01;
        break;
    case 'Z':
        zz -= 0.01;
        break;
    case 'x':
        xx += 0.01;
        break;
    case 'X':
        xx -= 0.01;
        break;
    case 'y':
        if (yturn == 1) {
            yturn = 0;
        }
        else {
            yturn = 1;
        }
        break;
    case 'Y':
        if (yturn == 2) {
            yturn = 0;
        }
        else {
            yturn = 2;
        }
        break;
    case 'r':
        if (rturn == 1) {
            rturn = 0;
        }
        else {
            rturn = 1;
        }
        break;
    case 'R':
        if (rturn == 2) {
            rturn = 0;
        }
        else {
            rturn = 2;
        }
        break;
    case 's':
        bmove = 0;
        mturn = 0;
        tturn = 0;
        yturn = 0;
        rturn = 0;
        break;
    case'c':
        my = 0;
        bx = 0;
        tx = 0;
        zz = 1;
        xx = 0;
        bmove = 0;
        mturn = 0;
        tturn = 0;
        yturn = 0;
        rturn = 0;
        break;
    case 'f':
        f_timer1 = true; f_timer2 = false; f_timer3 = false;
        break;
    case 'e':
        f_timer1 = false; f_timer2 = true; f_timer3 = false;
        break;
    case 'E':
        f_timer1 = false; f_timer2 = false; f_timer3 = true;
    }

    glutPostRedisplay();
}

void TimerFunction(int value)
{
    if (bmove == 1)
        bx += 0.005;
    else if (bmove == 2)
        bx -= 0.005;

    if (mturn == 1)
        my += 2;
    else if (mturn == 2)
        my -= 2;

    if (rturn == 1)
        ry += 0.5;
    else if (rturn == 2)
        ry -= 0.5;

    if (yturn == 1)
        yy += 2;
    else if (yturn == 2)
        yy -= 2;

    if (tturn == 1) {
        tx += 2;
        if (tx > 90) {
            tx = 90;
        }
    }
    else if (tturn == 2) {
        tx -= 2;
        if (tx < -90) {
            tx = -90;
        }
    }
    if (f_timer1) {
        if (f_turn1 >= 90.0);
        else {
            f_turn1++;
        }
    }
    else if (f_timer2) {
        if (f_turn1 > 0.0) {
            f_turn1--;
        }
        else {
            if (f_x_move < 0.0) {
                f_timer2 = false;
            }
            else {
                f_x_move -= 0.001f;
            }
        }
    }
    if (f_timer3) {
        if (f_x_move > 0.04) {
            f_timer3 = false;
        }
        else {
            f_x_move += 0.001f;
        }
    }

    glutPostRedisplay();
    glutTimerFunc(10, TimerFunction, 1);
}

GLchar* filetobuf(const char* file) {
    FILE* fptr;
    long length;
    char* buf;
    fptr = fopen(file, "rb");
    if (!fptr)
        return NULL;
    fseek(fptr, 0, SEEK_END);
    length = ftell(fptr);
    buf = (char*)malloc(length + 1);
    fseek(fptr, 0, SEEK_SET);
    fread(buf, length, 1, fptr);
    fclose(fptr);
    buf[length] = 0;
    return buf;
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
}
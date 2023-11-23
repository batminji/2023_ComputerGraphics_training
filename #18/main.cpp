#define _CRT_SECURE_NO_WARNINGS //--- 프로그램 맨 앞에 선언할 것
#pragma comment(lib, "glew32.lib")
#pragma comment(lib, "freeglut.lib")
#include <iostream>
#include <gl/glew.h>
#include <gl/freeglut.h>
#include <gl/freeglut_ext.h>
#include <time.h>
#include <math.h>
#include <stdlib.h>
#include <glm/glm.hpp>
#include <glm/ext.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <random>

#define WINDOWX 800
#define WINDOWY 800
using namespace std;

#define PI 3.141592

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

struct Dot {
    float x;
    float y;
    float z;
};
struct Circle {
    vector<Dot>dot;
    float circle_x, circle_z;
    float r;
    float degree;
};
Dot new_dot;
Circle circle;

GLfloat colors[]{
    0.0, 0.0, 0.0, 0.0, 0.0, 0.0
};

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
        glDrawArrays(GL_LINE_STRIP, 0, 2);
    }
};

bool culling = true;

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
        if (culling) {
            glEnable(GL_CULL_FACE);
        }
        glDrawArrays(GL_TRIANGLES, 0, 3);
        glDisable(GL_CULL_FACE);
    }
};

float mx, my, px, py;
float cnt;
int sel;
char comm;

BOOL Time, start = TRUE, TimeLoop;

float BackGround[] = { 0.0, 0.0, 0.0 };
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
glm::mat4 newTR = glm::mat4(1.0f);
GLUquadricObj* obj;
FILE* FL;
int faceNum = 0;
int Click = 0;

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
    glutCreateWindow("실습 18");// 윈도우 생성   (윈도우 이름)
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

bool mode = true, Top_Rotate = false, Front_Rotate = false, Side_Trans = false, Back_Scale = false;
bool Pyramid_Open = false, Pyramid_Open_2 = false;
float rotate_y = 0.0f, top_rotate = 0.0f, front_rotate = 0.0f, side_trans_y = 0.0f, back_scale = 1.0f;
float pyramid_open_1 = 0.0f, pyramid_open_2 = 0.0f, pyramid_open_3 = 0.0f, pyramid_open_4 = 0.0f;
bool Projection = true;
GLvoid drawScene() //--- 콜백 함수: 그리기 콜백 함수 
{
    if (start) {
        start = FALSE;
        // sel = 0;
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
    } // 초기화할 데이터
    glEnable(GL_DEPTH_TEST);

    glClearColor(BackGround[0], BackGround[1], BackGround[2], 1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    // glClear(GL_COLOR_BUFFER_BIT);


    glUseProgram(shaderID);
    glBindVertexArray(VAO);// 쉐이더 , 버퍼 배열 사용

    glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGB | GLUT_DEPTH);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    TR = glm::mat4(1.0f);

    // 그리기 코드
    glm::vec3 cameraPos = glm::vec3(0.0f, 0.0f, 5.0f); //--- 카메라 위치
    glm::vec3 cameraDirection = glm::vec3(0.0f, 0.0f, 0.0f); //--- 카메라 바라보는 방향
    glm::vec3 cameraUp = glm::vec3(0.0f, 1.0f, 0.0f); //--- 카메라 위쪽 방향
    glm::mat4 view = glm::mat4(1.0f);

    view = glm::lookAt(cameraPos, cameraDirection, cameraUp);
    unsigned int viewLocation = glGetUniformLocation(shaderID, "view"); //--- 뷰잉 변환 설정
    glUniformMatrix4fv(viewLocation, 1, GL_FALSE, &view[0][0]);

    glm::mat4 projection = glm::mat4(1.0f);
    if (Projection) {
        projection = glm::perspective(glm::radians(45.0f), 1.0f, 0.1f, 50.0f); //--- 투영 공간 설정: fovy, aspect, near, far
        projection = glm::translate(projection, glm::vec3(0.0f, 0.0f, -1.0f));
    }
    else {
        projection = glm::ortho(-5.0f, 5.0f, -5.0f, 5.0f, -5.0f, 5.0f);
        projection = glm::translate(projection, glm::vec3(0.0f, 0.0f, 1.0f));
    }
    unsigned int projectionLocation = glGetUniformLocation(shaderID, "projection"); //--- 투영 변환 값 설정
    glUniformMatrix4fv(projectionLocation, 1, GL_FALSE, &projection[0][0]);

    TR = glm::mat4(1.0f);
    unsigned int modelLocation = glGetUniformLocation(shaderID, "model");
    glUniformMatrix4fv(modelLocation, 1, GL_FALSE, glm::value_ptr(TR));

    TR = glm::rotate(TR, (float)glm::radians(30.0), glm::vec3(1.0f, 0.0f, 0.0f));
    TR = glm::rotate(TR, (float)glm::radians(-30.0), glm::vec3(0.0f, 1.0f, 0.0f));
    glUniformMatrix4fv(modelLocation, 1, GL_FALSE, glm::value_ptr(TR));
    xl.Bind();
    xl.Draw();
    TR = glm::mat4(1.0f);
    TR = glm::rotate(TR, (float)glm::radians(30.0), glm::vec3(1.0f, 0.0f, 0.0f));
    TR = glm::rotate(TR, (float)glm::radians(-30.0), glm::vec3(0.0f, 1.0f, 0.0f));
    glUniformMatrix4fv(modelLocation, 1, GL_FALSE, glm::value_ptr(TR));
    yl.Bind();
    yl.Draw();
    TR = glm::mat4(1.0f);
    TR = glm::rotate(TR, (float)glm::radians(30.0), glm::vec3(1.0f, 0.0f, 0.0f));
    TR = glm::rotate(TR, (float)glm::radians(-30.0), glm::vec3(0.0f, 1.0f, 0.0f));
    glUniformMatrix4fv(modelLocation, 1, GL_FALSE, glm::value_ptr(TR));
    zl.Bind();
    zl.Draw();
    // x, y, z 라인

    // 정육면체
    TR = glm::mat4(1.0f);

    TR = glm::scale(TR, glm::vec3(1.0f, 1.0f, 1.0f));

    TR = glm::translate(TR, glm::vec3(0.0f, 0.0f, 0.0f));

    TR = glm::rotate(TR, (float)glm::radians(30.0), glm::vec3(1.0f, 0.0f, 0.0f));
    TR = glm::rotate(TR, (float)glm::radians(-30.0), glm::vec3(0.0f, 1.0f, 0.0f));

    TR = glm::rotate(TR, (float)glm::radians(rotate_y), glm::vec3(0.0f, 1.0f, 0.0f));

    if (mode) { // cube mode
        for (int i = 0; i < 12; ++i) {
            if (Top_Rotate && (i == 4 || i == 5)) {
                newTR = glm::mat4(1.0f);
                newTR = glm::rotate(TR, (float)glm::radians(top_rotate), glm::vec3(0.0f, 1.0f, 0.0f));

                glUniformMatrix4fv(modelLocation, 1, GL_FALSE, glm::value_ptr(newTR));
                Fvertex[0][i].Bind();
                Fvertex[0][i].Draw();
            }
            else if (Front_Rotate && (i == 6 || i == 7)) {
                newTR = glm::mat4(1.0f);
                newTR = glm::translate(TR, glm::vec3(0.5f, -0.5f, 0.0f));
                newTR = glm::rotate(newTR, (float)glm::radians(front_rotate), glm::vec3(0.0f, 0.0f, 1.0f));
                newTR = glm::translate(newTR, glm::vec3(-0.5f, 0.5f, 0.0f));

                glUniformMatrix4fv(modelLocation, 1, GL_FALSE, glm::value_ptr(newTR));
                Fvertex[0][i].Bind();
                Fvertex[0][i].Draw();
            }
            else if (Side_Trans && (i == 0 || i == 1 || i == 10 || i == 11)) {
                newTR = glm::mat4(1.0f);
                newTR = glm::translate(TR, glm::vec3(0.0f, side_trans_y, 0.0f));

                glUniformMatrix4fv(modelLocation, 1, GL_FALSE, glm::value_ptr(newTR));
                Fvertex[0][i].Bind();
                Fvertex[0][i].Draw();
            }
            else if (Back_Scale && (i == 2 || i == 3)) {
                newTR = glm::mat4(1.0f);

                newTR = glm::translate(TR, glm::vec3(-0.5f, 0.0f, 0.0f));
                newTR = glm::scale(newTR, glm::vec3(back_scale, back_scale, back_scale));
                newTR = glm::translate(newTR, glm::vec3(0.5f, 0.0f, 0.0f));

                glUniformMatrix4fv(modelLocation, 1, GL_FALSE, glm::value_ptr(newTR));
                Fvertex[0][i].Bind();
                Fvertex[0][i].Draw();
            }
            else {
                glUniformMatrix4fv(modelLocation, 1, GL_FALSE, glm::value_ptr(TR));
                Fvertex[0][i].Bind();
                Fvertex[0][i].Draw();
            }
        }
    }
    else { // pyramid mode
        for (int i = 0; i < 6; ++i) {
            if ((Pyramid_Open||Pyramid_Open_2) && i == 2) {
                newTR = glm::mat4(1.0f);
                newTR = glm::translate(TR, glm::vec3(-0.5f, -0.5f, 0.0f));
                newTR = glm::rotate(newTR, (float)glm::radians(pyramid_open_1), glm::vec3(0.0f, 0.0f, 1.0f));
                newTR = glm::translate(newTR, glm::vec3(0.5f, 0.5f, 0.0f));

                glUniformMatrix4fv(modelLocation, 1, GL_FALSE, glm::value_ptr(newTR));
                Fvertex[1][i].Bind();
                Fvertex[1][i].Draw();
            }
            else if ((Pyramid_Open || Pyramid_Open_2) && i == 3) {
                newTR = glm::mat4(1.0f);
                newTR = glm::translate(TR, glm::vec3(0.0f, -0.5f, -0.5f));
                newTR = glm::rotate(newTR, (float)glm::radians(pyramid_open_2), glm::vec3(1.0f, 0.0f, 0.0f));
                newTR = glm::translate(newTR, glm::vec3(0.0f, 0.5f, +0.5f));

                glUniformMatrix4fv(modelLocation, 1, GL_FALSE, glm::value_ptr(newTR));
                Fvertex[1][i].Bind();
                Fvertex[1][i].Draw();
            }
            else if ((Pyramid_Open || Pyramid_Open_2) && i == 4) { // 오른쪽 앞
                newTR = glm::mat4(1.0f);
                newTR = glm::translate(TR, glm::vec3(0.5f, -0.5f, 0.0f));
                newTR = glm::rotate(newTR, (float)glm::radians(pyramid_open_3), glm::vec3(0.0f, 0.0f, 1.0f));
                newTR = glm::translate(newTR, glm::vec3(-0.5f, 0.5f, 0.0f));

                glUniformMatrix4fv(modelLocation, 1, GL_FALSE, glm::value_ptr(newTR));
                Fvertex[1][i].Bind();
                Fvertex[1][i].Draw();
            }
            else if ((Pyramid_Open || Pyramid_Open_2) && i == 5) { // 왼쪽 앞
                newTR = glm::mat4(1.0f);
                newTR = glm::translate(TR, glm::vec3(0.0f, -0.5f, 0.5f));
                newTR = glm::rotate(newTR, (float)glm::radians(pyramid_open_4), glm::vec3(1.0f, 0.0f, 0.0f));
                newTR = glm::translate(newTR, glm::vec3(0.0f, 0.5f, -0.5f));

                glUniformMatrix4fv(modelLocation, 1, GL_FALSE, glm::value_ptr(newTR));
                Fvertex[1][i].Bind();
                Fvertex[1][i].Draw();
            }
            else {
                glUniformMatrix4fv(modelLocation, 1, GL_FALSE, glm::value_ptr(TR));
                Fvertex[1][i].Bind();
                Fvertex[1][i].Draw();
            }
        }
    }

    glutSwapBuffers();
    glutPostRedisplay();
}
GLvoid Reshape(int w, int h) //--- 콜백 함수: 다시 그리기 콜백 함수
{
    glViewport(0, 0, w, h); //--- 뷰포트 변환
}

bool Rotate_y = false;
bool side_dir = true, back_dir = true;
void keyboard(unsigned char key, int x, int y) {
    switch (key) {
    case 'h': // 은면 제거 설정 / 해제
        culling = !culling;
        break;
    case 'y': // y축에 대하여 자전 / 멈춤
        Rotate_y = !Rotate_y;
        break;
    case 't': // 윗면 가운데 축 중심 회전
        mode = true;
        Top_Rotate = !Top_Rotate;
        top_rotate = 0.0f;
        break;
    case 'f': // 앞면 열고 닫음
        mode = true;
        Front_Rotate = !Front_Rotate;
        front_rotate = 0.0f;
        break;
    case 's': // 옆면 위로 올라갔다가 내려옴
        mode = true;
        Side_Trans = !Side_Trans;
        side_dir = true; side_trans_y = 0.0f;
        break;
    case 'b': // 뒷면이 작아졌다가 다시 커진다
        mode = true;
        Back_Scale = !Back_Scale;
        back_dir = true; back_scale = 1.0f;
        break;
    case 'o': // 사각뿔 모든 면들이 열리고 아래에서 만남
        mode = false;
        Pyramid_Open = !Pyramid_Open;
        pyramid_open_1 = pyramid_open_2 = pyramid_open_3 = pyramid_open_4 = 0.0f;
        break;
    case 'R': // 한 면씩 열림
        mode = false;
        Pyramid_Open_2 = !Pyramid_Open_2;
        pyramid_open_1 = pyramid_open_2 = pyramid_open_3 = pyramid_open_4 = 0.0f;
        break;
    case 'p':
        Projection = !Projection;
        break;
    }
    glutPostRedisplay();
}

void TimerFunction(int value)
{
    if (Rotate_y) {
        rotate_y++;
        if (rotate_y >= 360.0) rotate_y = 0.0f;
    }
    if (Top_Rotate) {
        top_rotate++;
        if (top_rotate >= 360.0)top_rotate = 0.0f;
    }
    if (Front_Rotate) {
        if (front_rotate <= -90.0);
        else front_rotate--;
    }
    if (Side_Trans) {
        if (side_dir) {
            side_trans_y += 0.01f;
            if (side_trans_y >= 1.0f)side_dir = !side_dir;
        }
        else {
            side_trans_y -= 0.01f;
            if (side_trans_y <= 0.0f)side_dir = !side_dir;
        }
    }
    if (Back_Scale) {
        if (back_dir) {
            back_scale -= 0.01f;
            if (back_scale <= 0.0f) {
                back_dir = !back_dir;
            }
        }
        else {
            back_scale += 0.01f;
            if (back_scale >= 1.0f) {
                back_scale = 1.0f;
                back_dir = !back_dir;
            }
        }
    }
    if (Pyramid_Open) {
        if (pyramid_open_1 > 232.0);
        else pyramid_open_1++;
        if (pyramid_open_2 < -232.0);
        else pyramid_open_2--;
        if (pyramid_open_3 < -232.0);
        else pyramid_open_3--;
        if (pyramid_open_4 > 232.0);
        else pyramid_open_4++;
    }
    if (Pyramid_Open_2) {
        if (pyramid_open_1 > 120.0) {
            if (pyramid_open_2 < -120.0) {
                if (pyramid_open_3 < -120.0) {
                    if (pyramid_open_4 > 120.0);
                    else pyramid_open_4++;
                }
                else pyramid_open_3--;
            }
            else pyramid_open_2--;
        }
        else pyramid_open_1++;
    }
    glutPostRedisplay();
    glutTimerFunc(10, TimerFunction, 1);
}

void ReadObj(FILE* objFile)
{
    faceNum = 0;

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
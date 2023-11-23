#define _CRT_SECURE_NO_WARNINGS
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

using namespace std;

random_device rd;
default_random_engine dre(rd());
uniform_real_distribution<float>uid(0.0f, 1.0f);
uniform_real_distribution<float>uid_t(-0.1, 0.1f);
uniform_int_distribution<int>uid_direction(0, 3);
uniform_real_distribution<float>uid_scale(0.02f, 0.1f);

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

float BackGround[] = { 0.0, 0.0, 0.0 };
Plane* Room;

struct Box {
    Plane* B;
    float x, y, z;
    float scale;
};
Box box[3];

struct Ball {
    GLUquadricObj* B;
    float x, y, z;
    float color_x, color_y, color_z;
    float scale;
    int direction;
};
Ball ball[5];
int ball_num = 1;

glm::vec4* vertex;
glm::vec4* face;
glm::vec3* outColor;

glm::mat4 TR = glm::mat4(1.0f);


FILE* FL;
int faceNum = 0;
bool start = true;

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

float cameraUp_x = 0.0f, cameraUp_y = 1.0f, cameraUp_z = 0.0f;
float upVectorAngle = 1.57f, upVectorSpeed = 0.01f;
bool rotate_z_left = false, rotate_z_right = false;
void collide_cube_box(Box* b) {
    if (rotate_z_left) {
        if (b->x - (b->scale / 2.0f) >= -0.5f) {
            b->x -= 0.01f;
        }
    }
    else if (rotate_z_right) {
        if (b->x + (b->scale / 2.0f) <= 0.5f) {
            b->x += 0.01f;
        }
    }
}

void ball_move(Ball* b) {
    switch (b->direction) {
    case 0: // 오른쪽 위로
        b->x += 0.002f; b->y += 0.001f;
        if (b->x + (b->scale / 2.0f) >= 0.5f)b->direction = 2;
        else if (b->y + (b->scale / 2.0f) >= 0.5f)b->direction = 3;
        break;
    case 1: // 오른쪽 아래로
        b->x += 0.002f; b->y -= 0.001f;
        if (b->x + (b->scale / 2.0f) >= 0.5f)b->direction = 3;
        else if (b->y - (b->scale / 2.0f) <= -0.5f)b->direction = 0;
        break;
    case 2: // 왼쪽 위로
        b->x -= 0.002f; b->y += 0.001f;
        if (b->x - (b->scale / 2.0f) <= -0.5f)b->direction = 0;
        else if (b->y + (b->scale / 2.0f) >= 0.5f)b->direction = 3;
        break;
    case 3: // 왼쪽 아래로
        b->x -= 0.002f; b->y -= 0.001f;
        if (b->x - (b->scale / 2.0f) <= -0.5f)b->direction = 1;
        else if (b->y - (b->scale / 2.0f) <= -0.5f)b->direction = 2;
        break;
    }
}

void main(int argc, char** argv) //--- 윈도우 출력하고 콜백함수 설정 { //--- 윈도우 생성하기
{
    srand((unsigned int)time(NULL));
    glutInit(&argc, argv); // glut 초기화
    glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGBA | GLUT_DEPTH); // 디스플레이 모드 설정
    glutInitWindowPosition(0, 0); // 윈도우의 위치 지정
    glutInitWindowSize(WINDOWX, WINDOWY); // 윈도우의 크기 지정
    glutCreateWindow("실습 23");// 윈도우 생성   (윈도우 이름)
    //--- GLEW 초기화하기
    glewExperimental = GL_TRUE;
    glewInit();

    InitShader();
    InitBuffer();

    glutKeyboardFunc(keyboard);
    glutTimerFunc(10, TimerFunction, 1);
    glutDisplayFunc(drawScene); //--- 출력 콜백 함수
    glutReshapeFunc(Reshape);
    glutMouseFunc(Mouse);
    glutMotionFunc(Motion);
    glutMainLoop();
}
float camera_z = 2.5f, camera_x = 0.0f;
int camera_rotate = 0;
float camera_rotate_y = 0.0f;
float under_rotate = 0.0f;
bool box_open = false;

GLvoid drawScene() //--- 콜백 함수: 그리기 콜백 함수 
{
    if (start) {
        start = FALSE;

        FL = fopen("Cube.obj", "rt");
        ReadObj(FL);
        fclose(FL);
        Room = (Plane*)malloc(sizeof(Plane) * faceNum);
        vectoplane(Room);
        planecolorset(Room, 0);

        for (int i = 0; i < 3; ++i) {
            FL = fopen("Cube.obj", "rt");
            ReadObj(FL);
            fclose(FL);
            box[i].B = (Plane*)malloc(sizeof(Plane) * faceNum);
            vectoplane(box[i].B);
            planecolorset(box[i].B, 0);
        }
        box[0].x = 0.0f; box[0].y = -0.425f; box[0].z = -0.4f; box[0].scale = 0.15f;
        box[1].x = 0.0f; box[1].y = -0.45f; box[1].z = 0.0f; box[1].scale = 0.1f;
        box[2].x = 0.0f; box[2].y = -0.475; box[2].z = 0.2f; box[2].scale = 0.05f;

        for (int i = 0; i < 5; ++i) {
            ball[i].B = gluNewQuadric();
            gluQuadricDrawStyle(ball[i].B, GLU_FILL);
            gluQuadricNormals(ball[i].B, GLU_SMOOTH);
            ball[i].x = uid_t(dre); ball[i].y = uid_t(dre); ball[i].z = uid_t(dre);
            ball[i].color_x = uid(dre); ball[i].color_y = uid(dre); ball[i].color_z = uid(dre);
            ball[i].direction = uid_direction(dre);
            ball[i].scale = uid_scale(dre);
        }

        glEnable(GL_DEPTH_TEST);

    } // 초기화할 데이터

    glViewport(0, 0, 800, 800);
    glClearColor(0.0, 0.0, 0.0, 0.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);    //배경

    glUseProgram(shaderID);
    glBindVertexArray(VAO);// 쉐이더 , 버퍼 배열 사용

    glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGB | GLUT_DEPTH);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);


    unsigned int color = glGetUniformLocation(shaderID, "color");
    unsigned int modelLocation = glGetUniformLocation(shaderID, "model");
    unsigned int viewLocation = glGetUniformLocation(shaderID, "view");
    unsigned int projLocation = glGetUniformLocation(shaderID, "projection");

    glm::mat4 Vw = glm::mat4(1.0f);
    glm::mat4 Cp = glm::mat4(1.0f);
    glm::mat4 Pj = glm::mat4(1.0f);
    glm::vec3 cameraPos;
    glm::vec3 cameraDirection;
    glm::vec3 cameraUp;

    Cp = glm::rotate(Cp, (float)glm::radians(camera_rotate_y), glm::vec3(0.0f, 1.0f, 0.0f));

    cameraPos = glm::vec4(camera_x, 0.0f, camera_z, 1.0f) * Cp; //--- 카메라 위치
    cameraDirection = glm::vec3(0.0f, 0.0f, 0.0f); //--- 카메라 바라보는 방향
    cameraUp = glm::vec3(cameraUp_x, cameraUp_y, cameraUp_z); //--- 카메라 위쪽 방향
    Vw = glm::mat4(1.0f);
    Vw = glm::lookAt(cameraPos, cameraDirection, cameraUp);
    glUniformMatrix4fv(viewLocation, 1, GL_FALSE, &Vw[0][0]);

    Pj = glm::mat4(1.0f);
    Pj = glm::perspective(glm::radians(45.0f), (float)WINDOWX / (float)WINDOWY, 0.1f, 100.0f);
    glUniformMatrix4fv(projLocation, 1, GL_FALSE, &Pj[0][0]);

    // 그리기 코드 
    // 부모행렬 -> 공전 -> 이동 -> 자전 -> 스케일
    // 방
    for (int i = 0; i < 12; ++i) {
        if (i == 10 || i == 11);
        else if (i == 8 || i == 9) {
            TR = glm::mat4(1.0f);
            TR = glm::translate(TR, glm::vec3(-0.5f, -0.5f, 0.0f));
            TR = glm::rotate(TR, (float)glm::radians(under_rotate), glm::vec3(0.0f, 0.0f, 1.0f));
            TR = glm::translate(TR, glm::vec3(0.5f, 0.5f, 0.0f));

            TR = glm::scale(TR, glm::vec3(1.0f, 1.0f, 1.0f));
            glUniformMatrix4fv(modelLocation, 1, GL_FALSE, glm::value_ptr(TR));
            Room[i].Bind();
            Room[i].Draw();
        }
        else {
            TR = glm::mat4(1.0f);
            TR = glm::translate(TR, glm::vec3(0.0f, 0.0f, 0.0f));

            TR = glm::scale(TR, glm::vec3(1.0f, 1.0f, 1.0f));
            glUniformMatrix4fv(modelLocation, 1, GL_FALSE, glm::value_ptr(TR));
            glUniform3f(color, 1.0f, 1.0f, 1.0f);

            Room[i].Bind();
            Room[i].Draw();
        }
    }

    // 정육면체
    for (int i = 0; i < 3; ++i) {
        for (int j = 0; j < 12; ++j) {
            TR = glm::mat4(1.0f);
            TR = glm::translate(TR, glm::vec3(box[i].x, box[i].y, box[i].z));

            TR = glm::scale(TR, glm::vec3(box[i].scale, box[i].scale, box[i].scale));
            glUniformMatrix4fv(modelLocation, 1, GL_FALSE, glm::value_ptr(TR));
            glUniform3f(color, 1.0f, 1.0f, 1.0f);
            box[i].B[j].Bind();
            box[i].B[j].Draw();
        }
    }

    // 공
    for (int i = 0; i < ball_num; ++i) {
        TR = glm::mat4(1.0f);
        TR = glm::translate(TR, glm::vec3(ball[i].x, ball[i].y, ball[i].z));

        TR = glm::scale(TR, glm::vec3(ball[i].scale, ball[i].scale, ball[i].scale));
        glUniformMatrix4fv(modelLocation, 1, GL_FALSE, glm::value_ptr(TR));

        glUniform3f(color, ball[i].color_x, ball[i].color_y, ball[i].color_z);
        gluSphere(ball[i].B, 1.0, 20, 20);
    }


    glutSwapBuffers();
    glutPostRedisplay();
}

GLvoid Reshape(int w, int h) //--- 콜백 함수: 다시 그리기 콜백 함수
{

}

void keyboard(unsigned char key, int x, int y) {
    switch (key) {
    case 'o': // 바닥 열리기
        box_open = true;
        break;
    case 'z': // z축으로 양의 방향
        camera_z += 0.01f;
        break;
    case 'Z': // z축으로 음의 방향
        camera_z -= 0.01f;
        break;
    case 'y': // y축에 대하여 시계 방향 회전
        camera_rotate_y += 0.1f;
        break;
    case 'Y': // y축에 대하여 반시계 방향 회전
        camera_rotate_y -= 0.1f;
        break;
    case 'B': // 공이 새로 생김.
        if (ball_num < 5)ball_num++;
        break;
    }
    glutPostRedisplay();
}

void TimerFunction(int value)
{
    cameraUp_x = cos(upVectorAngle);
    cameraUp_z = sin(upVectorAngle);

    if (rotate_z_left) {
        if (upVectorAngle >= 1.0f) {
            upVectorAngle -= upVectorSpeed;
        }
    }
    else if (rotate_z_right) {
        if (upVectorAngle <= 2.0f) {
            upVectorAngle += upVectorSpeed;
        }
    }

    for (int i = 0; i < 3; ++i) {
        collide_cube_box(&box[i]);
    }

    for (int i = 0; i < ball_num; ++i) {
        ball_move(&ball[i]);
    }

    if (box_open) {
        if (under_rotate >= -90.0f)under_rotate -= 1;
        for (int i = 0; i < 3; ++i) {
            box[i].y -= 0.005f;
        }
    }

    glutPostRedisplay();
    glutTimerFunc(10, TimerFunction, 1);
}

double mx, my;
void Mouse(int button, int state, int x, int y)
{
    mx = ((double)x - WINDOWX / 2.0) / (WINDOWX / 2.0);
    my = -(((double)y - WINDOWY / 2.0) / (WINDOWY / 2.0));

    
}
void Motion(int x, int y)
{
    mx = ((double)x - WINDOWX / 2.0) / (WINDOWX / 2.0);
    my = -(((double)y - WINDOWY / 2.0) / (WINDOWY / 2.0));
    
    if (mx < 0.0f) {
        rotate_z_left = true; rotate_z_right = false;
    }
    else if (mx > 0.0f) {
        rotate_z_left = false; rotate_z_right = true;
    }
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
    char count[1000];
    char bind[1000];
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
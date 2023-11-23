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
uniform_real_distribution<float>uid(0, 1);
uniform_int_distribution<int>uid_obstacle(1, 49);
uniform_int_distribution<int>uid_fall(1, 2);

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

#define TILE_COLOR1_R 0.749
#define TILE_COLOR1_G 0.843
#define TILE_COLOR1_B 0.960

#define TILE_COLOR2_R 0.960
#define TILE_COLOR2_G 0.749
#define TILE_COLOR2_B 0.905

#define BRIDGE_COLOR_R 0.921
#define BRIDGE_COLOR_G 0.858
#define BRIDGE_COLOR_B 0.203

struct Tile {
    Plane* P;
    float x, y, z;
    bool fall_switch;
    int fall;
};
Tile tile[49];
struct Pillar {
    Plane* P;
    float x, y, z;
};
Pillar pillar[4];
struct Bridge {
    Plane* P;
    float x, y, z;
    float scale_x, scale_y, scale_z;
};
Bridge bridge[3];

float BackGround[] = { 0.0, 0.0, 0.0 };
Plane* Robot[7];
Plane* mini_Robot[7];

glm::vec4* vertex;
glm::vec4* face;
glm::vec3* outColor;

glm::mat4 TR = glm::mat4(1.0f);

bool collide_robot_pillar(float rx, float rz, Pillar *p) {
    if ((rx - 0.03f) < (p->x + 0.1f) && (rx + 0.03f) > (p->x - 0.1f) && (rz - 0.03f) < (p->z + 0.1f) && (rz + 0.03f) > (p->z - 0.1f)) {
        return true;
    }
    return false;
}

bool collide_robot_obstacle(float rx, float rz, Tile* t) {
    if ((rx - 0.03f) < (t->x + 0.1f) && (rx + 0.03f) > (t->x - 0.1f) && (rz - 0.03f) < (t->z + 0.1f) && (rz + 0.03f) > (t->z - 0.1f)) {
        return true;
    }
    return false;
}

bool collide_robot_bridge(float rx, float rz, float ry, Bridge b) {
    if ((rx - 0.03f) < (b.x + 0.1f) && (rx + 0.03f) > (b.x - 0.1f) && (rz - 0.03f) < (b.z + 0.05f) && (rz + 0.03f) > (b.z - 0.05f)) {
        return true;
    }
    return false;
}

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
    /*const GLchar* fragmentShaderSource = {
       "#version 330 core\n"
       "out vec4 fColor; \n"
       "in vec3 outColor;\n"
       "void main() \n"
       "{\n"
       "fColor = vec4(outColor, 1.0); \n"
       "} \0"
    };*/


    const GLchar* fragmentShaderSource = filetobuf("fragment.glsl");

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
    glutCreateWindow("실습 22");// 윈도우 생성   (윈도우 이름)
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

float robot_x = 0.0f, robot_y = 0.2f, robot_z = 0.0f;
float robot_dirction = 0.0f;
float robot_move_range = 30.0f;
float robot_move_speed = 1.0f;
float robot_move_rad = 30.0f;
bool robot_move_change = true;

bool mini_robot_show = false;
float mini_robot_x = 0.0f, mini_robot_y = 0.1f, mini_robot_z = -0.1f;

float camera_z = 2.0f, camera_x = 0.0f;
int camera_rotate = 0;
float camera_rotate_y = 0.0f;

GLvoid drawScene() //--- 콜백 함수: 그리기 콜백 함수 
{
    if (start) {
        start = FALSE;

        for (int i = 0; i < 7; ++i) {
            FL = fopen("cube.obj", "rt");
            ReadObj(FL);
            fclose(FL);
            Robot[i] = (Plane*)malloc(sizeof(Plane) * faceNum);
            vectoplane(Robot[i]);
            planecolorset(Robot[i], 0);
        }
        for (int i = 0; i < 7; ++i) {
            FL = fopen("cube.obj", "rt");
            ReadObj(FL);
            fclose(FL);
            mini_Robot[i] = (Plane*)malloc(sizeof(Plane) * faceNum);
            vectoplane(mini_Robot[i]);
            planecolorset(mini_Robot[i], 0);
        }

        float tile_x = -0.6f;
        float tile_z = -0.6f;
        int tile_cnt = 0;
        for (int i = 0; i < 49; ++i) {
            FL = fopen("cube.obj", "rt");
            ReadObj(FL);
            fclose(FL);
            tile[i].P = (Plane*)malloc(sizeof(Plane) * faceNum);
            vectoplane(tile[i].P);
            if (i % 2 == 0) {
                planecolorset(tile[i].P, 1);
            }
            else {
                planecolorset(tile[i].P, 2);
            }
            tile[i].x = tile_x; tile[i].y = -0.1f; tile[i].z = tile_z; tile[i].fall_switch = false;
            tile_x += 0.2f; tile_cnt++;
            if (tile_cnt == 7) {
                tile_x = -0.6f; tile_z += 0.2f; tile_cnt = 0;
            }
        }

        for (int i = 0; i < 4; ++i) {
            int rand_ob = uid_obstacle(dre);
            if (rand_ob != 0 && rand_ob != 6 && rand_ob != 48 && rand_ob != 41 && rand_ob != 17) {
                tile[rand_ob].y += 0.2f; tile[rand_ob].fall = uid_fall(dre);
            }
            else i--;
        }

        for (int i = 0; i < 4; ++i) {
            FL = fopen("cube.obj", "rt");
            ReadObj(FL);
            fclose(FL);
            pillar[i].P = (Plane*)malloc(sizeof(Plane) * faceNum);
            vectoplane(pillar[i].P);
            planecolorset(pillar[i].P, 0);
        }
        pillar[0].x = -0.6f; pillar[0].y = 0.2f; pillar[0].z = -0.6f;
        pillar[1].x = 0.6f; pillar[1].y = 0.2f; pillar[1].z = -0.6f;
        pillar[2].x = -0.6f; pillar[2].y = 0.2f; pillar[2].z = 0.6f;
        pillar[3].x = 0.6f; pillar[3].y = 0.2f; pillar[3].z = 0.6f;

        for (int i = 0; i < 3; ++i) {
            FL = fopen("cube.obj", "rt");
            ReadObj(FL);
            fclose(FL);
            bridge[i].P = (Plane*)malloc(sizeof(Plane) * faceNum);
            vectoplane(bridge[i].P);
            planecolorset(bridge[i].P, 3);
        }
        bridge[0].x = -0.2f; bridge[0].y = 0.1f; bridge[0].z = 0.0f; bridge[0].scale_x = 0.2f; bridge[0].scale_y = 0.05f; bridge[0].scale_z = 0.1f;
        bridge[1].x = -0.28f; bridge[1].y = 0.05f; bridge[1].z = 0.0f; bridge[1].scale_x = 0.05f; bridge[1].scale_y = 0.1f; bridge[1].scale_z = 0.1f;
        bridge[2].x = -0.12f; bridge[2].y = 0.05f; bridge[2].z = 0.0f; bridge[2].scale_x = 0.05f; bridge[2].scale_y = 0.1f; bridge[2].scale_z = 0.1f;
        
        glEnable(GL_DEPTH_TEST);

    } // 초기화할 데이터

    glViewport(0, 0, 800, 800);
    glClearColor(0.0, 0.0, 0.0, 0.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);    //배경

    glUseProgram(shaderID);
    glBindVertexArray(VAO);// 쉐이더 , 버퍼 배열 사용

    glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGB | GLUT_DEPTH);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);


    unsigned int outColor = glGetUniformLocation(shaderID, "outColor");
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

    cameraPos = glm::vec4(camera_x, 0.5f, camera_z, 1.0f) * Cp; //--- 카메라 위치
    cameraDirection = glm::vec3(0.0f, 0.0f, 0.0f); //--- 카메라 바라보는 방향
    cameraUp = glm::vec3(0.0f, 1.0f, 0.0f); //--- 카메라 위쪽 방향
    Vw = glm::mat4(1.0f);
    Vw = glm::lookAt(cameraPos, cameraDirection, cameraUp);
    glUniformMatrix4fv(viewLocation, 1, GL_FALSE, &Vw[0][0]);

    Pj = glm::mat4(1.0f);
    Pj = glm::perspective(glm::radians(45.0f), (float)WINDOWX / (float)WINDOWY, 0.1f, 100.0f);
    glUniformMatrix4fv(projLocation, 1, GL_FALSE, &Pj[0][0]);

    // 그리기 코드 
    // 부모행렬 -> 공전 -> 이동 -> 자전 -> 스케일
    // 바닥
    for (int i = 0; i < 49; ++i) {
        TR = glm::mat4(1.0f);
        TR = glm::translate(TR, glm::vec3(tile[i].x, tile[i].y, tile[i].z));

        TR = glm::scale(TR, glm::vec3(0.2f, 0.2f, 0.2f));
        glUniformMatrix4fv(modelLocation, 1, GL_FALSE, glm::value_ptr(TR));
        for (int j = 0; j < 12; ++j) {
            
            tile[i].P[j].Bind();
            tile[i].P[j].Draw();
        }
    }
    // 기둥
    for (int i = 0; i < 4; ++i) {
        TR = glm::mat4(1.0f);
        TR = glm::translate(TR, glm::vec3(pillar[i].x, pillar[i].y, pillar[i].z));

        TR = glm::scale(TR, glm::vec3(0.15f, 0.4f, 0.15f));
        glUniformMatrix4fv(modelLocation, 1, GL_FALSE, glm::value_ptr(TR));
        for (int j = 0; j < 12; ++j) {            
            pillar[i].P[j].Bind();
            pillar[i].P[j].Draw();
        }
    }
    // 브릿지
    for (int i = 0; i < 3; ++i) {
        TR = glm::mat4(1.0f);
        TR = glm::translate(TR, glm::vec3(bridge[i].x, bridge[i].y, bridge[i].z));

        TR = glm::scale(TR, glm::vec3(bridge[i].scale_x, bridge[i].scale_y, bridge[i].scale_z));
        glUniformMatrix4fv(modelLocation, 1, GL_FALSE, glm::value_ptr(TR));
        for (int j = 0; j < 12; ++j) {            
            bridge[i].P[j].Bind();
            bridge[i].P[j].Draw();
        }
    }

    // 로봇

    // 몸통
    TR = glm::mat4(1.0f);
    TR = glm::translate(TR, glm::vec3(robot_x, robot_y, robot_z));
    TR = glm::rotate(TR, (float)glm::radians(robot_dirction), glm::vec3(0.0f, 1.0f, 0.0f));
    TR = glm::translate(TR, glm::vec3(-robot_x, -robot_y, -robot_z));

    TR = glm::translate(TR, glm::vec3(robot_x, robot_y, robot_z));
    TR = glm::scale(TR, glm::vec3(0.08f, 0.12f, 0.05f));
    glUniformMatrix4fv(modelLocation, 1, GL_FALSE, glm::value_ptr(TR));
    for (int i = 0; i < 12; ++i) {
        Robot[0][i].Bind();
        Robot[0][i].Draw();
    }
    
    if (mini_robot_show) {
        TR = glm::mat4(1.0f);
        TR = glm::translate(TR, glm::vec3(robot_x + mini_robot_x, robot_y - mini_robot_y, robot_z + mini_robot_z));
        TR = glm::rotate(TR, (float)glm::radians(robot_dirction), glm::vec3(0.0f, 1.0f, 0.0f));
        TR = glm::translate(TR, glm::vec3(-robot_x - mini_robot_x, -robot_y + mini_robot_y, -robot_z - mini_robot_z));

        TR = glm::translate(TR, glm::vec3(robot_x + mini_robot_x, robot_y - mini_robot_y, robot_z + mini_robot_z));
        TR = glm::scale(TR, glm::vec3(0.04f, 0.06f, 0.025f));
        glUniformMatrix4fv(modelLocation, 1, GL_FALSE, glm::value_ptr(TR));
        for (int i = 0; i < 12; ++i) {
            Robot[0][i].Bind();
            Robot[0][i].Draw();
        }
    }

    // 머리
    TR = glm::mat4(1.0f);
    TR = glm::translate(TR, glm::vec3(robot_x, robot_y, robot_z));
    TR = glm::rotate(TR, (float)glm::radians(robot_dirction), glm::vec3(0.0f, 1.0f, 0.0f));
    TR = glm::translate(TR, glm::vec3(-robot_x, -robot_y, -robot_z));

    TR = glm::translate(TR, glm::vec3(robot_x, robot_y + 0.1f, robot_z));
    TR = glm::scale(TR, glm::vec3(0.06f, 0.06f, 0.06f));
    glUniformMatrix4fv(modelLocation, 1, GL_FALSE, glm::value_ptr(TR));
    for (int i = 0; i < 12; ++i) {
        mini_Robot[1][i].Bind();
        mini_Robot[1][i].Draw();
    }
    if (mini_robot_show) {
        TR = glm::mat4(1.0f);
        TR = glm::translate(TR, glm::vec3(robot_x + mini_robot_x, robot_y - mini_robot_y, robot_z + mini_robot_z));
        TR = glm::rotate(TR, (float)glm::radians(robot_dirction), glm::vec3(0.0f, 1.0f, 0.0f));
        TR = glm::translate(TR, glm::vec3(-robot_x - mini_robot_x, -robot_y + mini_robot_y, -robot_z - mini_robot_z));

        TR = glm::translate(TR, glm::vec3(robot_x + mini_robot_x, robot_y + 0.05f - mini_robot_y, robot_z + mini_robot_z));
        TR = glm::scale(TR, glm::vec3(0.03f, 0.03f, 0.03f));
        glUniformMatrix4fv(modelLocation, 1, GL_FALSE, glm::value_ptr(TR));
        for (int i = 0; i < 12; ++i) {
            mini_Robot[1][i].Bind();
            mini_Robot[1][i].Draw();
        }
    }
    // 코
    TR = glm::mat4(1.0f);
    TR = glm::translate(TR, glm::vec3(robot_x, robot_y, robot_z));
    TR = glm::rotate(TR, (float)glm::radians(robot_dirction), glm::vec3(0.0f, 1.0f, 0.0f));
    TR = glm::translate(TR, glm::vec3(-robot_x, -robot_y, -robot_z));

    TR = glm::translate(TR, glm::vec3(robot_x, robot_y + 0.1f, robot_z + 0.05f));
    TR = glm::scale(TR, glm::vec3(0.01f, 0.01f, 0.05f));
    glUniformMatrix4fv(modelLocation, 1, GL_FALSE, glm::value_ptr(TR));
    for (int i = 0; i < 12; ++i) {
        Robot[2][i].Bind();
        Robot[2][i].Draw();
    }
    if (mini_robot_show) {
        TR = glm::mat4(1.0f);
        TR = glm::translate(TR, glm::vec3(robot_x + mini_robot_x, robot_y - mini_robot_y, robot_z + mini_robot_z));
        TR = glm::rotate(TR, (float)glm::radians(robot_dirction), glm::vec3(0.0f, 1.0f, 0.0f));
        TR = glm::translate(TR, glm::vec3(-robot_x - mini_robot_x, -robot_y + mini_robot_y, -robot_z - mini_robot_z));

        TR = glm::translate(TR, glm::vec3(robot_x + mini_robot_x, robot_y + 0.05f - mini_robot_y, robot_z + 0.025f + mini_robot_z));
        TR = glm::scale(TR, glm::vec3(0.005f, 0.005f, 0.025f));
        glUniformMatrix4fv(modelLocation, 1, GL_FALSE, glm::value_ptr(TR));
        for (int i = 0; i < 12; ++i) {
            mini_Robot[2][i].Bind();
            mini_Robot[2][i].Draw();
        }
    }

    // 왼팔
    TR = glm::mat4(1.0f);
    TR = glm::translate(TR, glm::vec3(robot_x, robot_y, robot_z));
    TR = glm::rotate(TR, (float)glm::radians(robot_dirction), glm::vec3(0.0f, 1.0f, 0.0f));
    TR = glm::translate(TR, glm::vec3(-robot_x, -robot_y, -robot_z));

    TR = glm::translate(TR, glm::vec3(robot_x - 0.05f, robot_y, robot_z));

    TR = glm::translate(TR, glm::vec3(0.0f, 0.05f, 0.0f));
    TR = glm::rotate(TR, (float)glm::radians(robot_move_rad), glm::vec3(1.0f, 0.0f, 0.0f));
    TR = glm::translate(TR, glm::vec3(0.0f, -0.05f, 0.0f));

    TR = glm::scale(TR, glm::vec3(0.02f, 0.14f, 0.02f));
    glUniformMatrix4fv(modelLocation, 1, GL_FALSE, glm::value_ptr(TR));
    for (int i = 0; i < 12; ++i) {
        Robot[3][i].Bind();
        Robot[3][i].Draw();
    }
    if (mini_robot_show) {
        TR = glm::mat4(1.0f);
        TR = glm::translate(TR, glm::vec3(robot_x + mini_robot_x, robot_y - mini_robot_y, robot_z + mini_robot_z));
        TR = glm::rotate(TR, (float)glm::radians(robot_dirction), glm::vec3(0.0f, 1.0f, 0.0f));
        TR = glm::translate(TR, glm::vec3(-robot_x - mini_robot_x, -robot_y + mini_robot_y, -robot_z - mini_robot_z));

        TR = glm::translate(TR, glm::vec3(robot_x - 0.025f + mini_robot_x, robot_y - mini_robot_y, robot_z + mini_robot_z));

        TR = glm::translate(TR, glm::vec3(0.0f, 0.025f, 0.0f));
        TR = glm::rotate(TR, (float)glm::radians(robot_move_rad), glm::vec3(1.0f, 0.0f, 0.0f));
        TR = glm::translate(TR, glm::vec3(0.0f, -0.025f, 0.0f));

        TR = glm::scale(TR, glm::vec3(0.01f, 0.07f, 0.01f));
        glUniformMatrix4fv(modelLocation, 1, GL_FALSE, glm::value_ptr(TR));
        for (int i = 0; i < 12; ++i) {
            mini_Robot[3][i].Bind();
            mini_Robot[3][i].Draw();
        }
    }

    // 오른팔
    TR = glm::mat4(1.0f);
    TR = glm::translate(TR, glm::vec3(robot_x, robot_y, robot_z));
    TR = glm::rotate(TR, (float)glm::radians(robot_dirction), glm::vec3(0.0f, 1.0f, 0.0f));
    TR = glm::translate(TR, glm::vec3(-robot_x, -robot_y, -robot_z));

    TR = glm::translate(TR, glm::vec3(robot_x + 0.05f, robot_y, robot_z));

    TR = glm::translate(TR, glm::vec3(0.0f, 0.05f, 0.0f));
    TR = glm::rotate(TR, (float)glm::radians(-robot_move_rad), glm::vec3(1.0f, 0.0f, 0.0f));
    TR = glm::translate(TR, glm::vec3(0.0f, -0.05f, 0.0f));

    TR = glm::scale(TR, glm::vec3(0.02f, 0.14f, 0.02f));
    glUniformMatrix4fv(modelLocation, 1, GL_FALSE, glm::value_ptr(TR));
    for (int i = 0; i < 12; ++i) {
        Robot[4][i].Bind();
        Robot[4][i].Draw();
    }
    if (mini_robot_show) {
        TR = glm::mat4(1.0f);
        TR = glm::translate(TR, glm::vec3(robot_x + mini_robot_x, robot_y - mini_robot_y, robot_z + mini_robot_z));
        TR = glm::rotate(TR, (float)glm::radians(robot_dirction), glm::vec3(0.0f, 1.0f, 0.0f));
        TR = glm::translate(TR, glm::vec3(-robot_x - mini_robot_x, -robot_y + mini_robot_y, -robot_z - mini_robot_z));

        TR = glm::translate(TR, glm::vec3(robot_x + 0.025f + mini_robot_x, robot_y - mini_robot_y, robot_z + mini_robot_z));

        TR = glm::translate(TR, glm::vec3(0.0f, 0.025f, 0.0f));
        TR = glm::rotate(TR, (float)glm::radians(-robot_move_rad), glm::vec3(1.0f, 0.0f, 0.0f));
        TR = glm::translate(TR, glm::vec3(0.0f, -0.025f, 0.0f));

        TR = glm::scale(TR, glm::vec3(0.01f, 0.07f, 0.01f));
        glUniformMatrix4fv(modelLocation, 1, GL_FALSE, glm::value_ptr(TR));
        for (int i = 0; i < 12; ++i) {
            mini_Robot[4][i].Bind();
            mini_Robot[4][i].Draw();
        }
    }

    // 왼쪽 다리
    TR = glm::mat4(1.0f);
    TR = glm::translate(TR, glm::vec3(robot_x, robot_y, robot_z));
    TR = glm::rotate(TR, (float)glm::radians(robot_dirction), glm::vec3(0.0f, 1.0f, 0.0f));
    TR = glm::translate(TR, glm::vec3(-robot_x, -robot_y, -robot_z));

    TR = glm::translate(TR, glm::vec3(robot_x - 0.02f, robot_y - 0.12f, robot_z));

    TR = glm::translate(TR, glm::vec3(0.0f, 0.05f, 0.0f));
    TR = glm::rotate(TR, (float)glm::radians(-robot_move_rad), glm::vec3(1.0f, 0.0f, 0.0f));
    TR = glm::translate(TR, glm::vec3(0.0f, -0.05f, 0.0f));

    TR = glm::scale(TR, glm::vec3(0.02f, 0.14f, 0.02f));
    glUniformMatrix4fv(modelLocation, 1, GL_FALSE, glm::value_ptr(TR));
    for (int i = 0; i < 12; ++i) {
        Robot[5][i].Bind();
        Robot[5][i].Draw();
    }
    if (mini_robot_show) {
        TR = glm::mat4(1.0f);
        TR = glm::translate(TR, glm::vec3(robot_x + mini_robot_x, robot_y - mini_robot_y, robot_z + mini_robot_z));
        TR = glm::rotate(TR, (float)glm::radians(robot_dirction), glm::vec3(0.0f, 1.0f, 0.0f));
        TR = glm::translate(TR, glm::vec3(-robot_x - mini_robot_x, -robot_y + mini_robot_y, -robot_z - mini_robot_z));

        TR = glm::translate(TR, glm::vec3(robot_x - 0.01f + mini_robot_x, robot_y - 0.06f - mini_robot_y, robot_z + mini_robot_z));

        TR = glm::translate(TR, glm::vec3(0.0f, 0.025f, 0.0f));
        TR = glm::rotate(TR, (float)glm::radians(-robot_move_rad), glm::vec3(1.0f, 0.0f, 0.0f));
        TR = glm::translate(TR, glm::vec3(0.0f, -0.025f, 0.0f));

        TR = glm::scale(TR, glm::vec3(0.01f, 0.07f, 0.01f));
        glUniformMatrix4fv(modelLocation, 1, GL_FALSE, glm::value_ptr(TR));
        for (int i = 0; i < 12; ++i) {
            mini_Robot[5][i].Bind();
            mini_Robot[5][i].Draw();
        }
    }
    // 오른쪽 다리
    TR = glm::mat4(1.0f);
    TR = glm::translate(TR, glm::vec3(robot_x, robot_y, robot_z));
    TR = glm::rotate(TR, (float)glm::radians(robot_dirction), glm::vec3(0.0f, 1.0f, 0.0f));
    TR = glm::translate(TR, glm::vec3(-robot_x, -robot_y, -robot_z));

    TR = glm::translate(TR, glm::vec3(robot_x + 0.02f, robot_y - 0.12f, robot_z));

    TR = glm::translate(TR, glm::vec3(0.0f, 0.05f, 0.0f));
    TR = glm::rotate(TR, (float)glm::radians(robot_move_rad), glm::vec3(1.0f, 0.0f, 0.0f));
    TR = glm::translate(TR, glm::vec3(0.0f, -0.05f, 0.0f));

    TR = glm::scale(TR, glm::vec3(0.02f, 0.14f, 0.02f));
    glUniformMatrix4fv(modelLocation, 1, GL_FALSE, glm::value_ptr(TR));
    for (int i = 0; i < 12; ++i) {
        Robot[6][i].Bind();
        Robot[6][i].Draw();
    }
    if (mini_robot_show) {
        TR = glm::mat4(1.0f);
        TR = glm::translate(TR, glm::vec3(robot_x + mini_robot_x, robot_y - mini_robot_y, robot_z + mini_robot_z));
        TR = glm::rotate(TR, (float)glm::radians(robot_dirction), glm::vec3(0.0f, 1.0f, 0.0f));
        TR = glm::translate(TR, glm::vec3(-robot_x - mini_robot_x, -robot_y + mini_robot_y, -robot_z - mini_robot_z));

        TR = glm::translate(TR, glm::vec3(robot_x + 0.01f + mini_robot_x, robot_y - 0.06f - mini_robot_y, robot_z + mini_robot_z));

        TR = glm::translate(TR, glm::vec3(0.0f, 0.025f, 0.0f));
        TR = glm::rotate(TR, (float)glm::radians(robot_move_rad), glm::vec3(1.0f, 0.0f, 0.0f));
        TR = glm::translate(TR, glm::vec3(0.0f, -0.025f, 0.0f));

        TR = glm::scale(TR, glm::vec3(0.01f, 0.07f, 0.01f));
        glUniformMatrix4fv(modelLocation, 1, GL_FALSE, glm::value_ptr(TR));
        for (int i = 0; i < 12; ++i) {
            mini_Robot[6][i].Bind();
            mini_Robot[6][i].Draw();
        }
    }

    glutSwapBuffers();
    glutPostRedisplay();
}

GLvoid Reshape(int w, int h) //--- 콜백 함수: 다시 그리기 콜백 함수
{

}

bool change_direction = true;
bool jump_timer = false;
bool mini_jump_timer = false;
bool on_bridge = false;
void keyboard(unsigned char key, int x, int y) {
    switch (key) {
    case 'w': // 뒤로
        if (!change_direction) {
            robot_dirction = 0.0f; mini_robot_x = 0.0f; mini_robot_z = -0.1f;
            robot_z += 0.01f;
            if (robot_z >= 0.75f)change_direction = !change_direction;
            for (int i = 0; i < 4; ++i) {
                if (collide_robot_pillar(robot_x, robot_z, &pillar[i]))robot_z -= 0.01f;
            }
            for (int i = 0; i < 49; ++i) {
                if(tile[i].y >= 0.0f && collide_robot_obstacle(robot_x, robot_z, &tile[i]))robot_z -= 0.01f;
            }
            if (!on_bridge && collide_robot_bridge(robot_x, robot_z, robot_y, bridge[0]))robot_z -= 0.01f;
        }
        else {
            robot_dirction = 180.0f; mini_robot_x = 0.0f; mini_robot_z = 0.1f;
            robot_z -= 0.01f;
            if (robot_z <= -0.75f)change_direction = !change_direction;
            for (int i = 0; i < 4; ++i) {
                if (collide_robot_pillar(robot_x, robot_z, &pillar[i])) robot_z += 0.01f;
            }
            for (int i = 0; i < 49; ++i) {
                if (tile[i].y >= 0.0f && collide_robot_obstacle(robot_x, robot_z, &tile[i]))robot_z += 0.01f;
            }
            if (!on_bridge && collide_robot_bridge(robot_x, robot_z, robot_y, bridge[0]))robot_z += 0.01f;
        }
        break;
    case 'a': // 왼쪽으로
        if (!change_direction) {
            robot_dirction = 90.0f; mini_robot_x = -0.1f; mini_robot_z = 0.0f;
            robot_x += 0.01f;
            if (robot_x >= 0.75f)change_direction = !change_direction;
            for (int i = 0; i < 4; ++i) {
                if (collide_robot_pillar(robot_x, robot_z, &pillar[i])) robot_x -= 0.01f;
            }
            for (int i = 0; i < 49; ++i) {
                if (tile[i].y >= 0.0f && collide_robot_obstacle(robot_x, robot_z, &tile[i]))robot_x -= 0.01f;
            }
            if (!on_bridge && collide_robot_bridge(robot_x, robot_z, robot_y, bridge[0]))robot_x -= 0.01f;
        }
        else {
            robot_dirction = 270.0f; mini_robot_x = 0.1f; mini_robot_z = 0.0f;
            robot_x -= 0.01f;
            if (robot_x <= -0.75f)change_direction = !change_direction;
            for (int i = 0; i < 4; ++i) {
                if (collide_robot_pillar(robot_x, robot_z, &pillar[i])) robot_x += 0.01f;
            }
            for (int i = 0; i < 49; ++i) {
                if (tile[i].y >= 0.0f && collide_robot_obstacle(robot_x, robot_z, &tile[i]))robot_x += 0.01f;
            }
            if (!on_bridge && collide_robot_bridge(robot_x, robot_z, robot_y, bridge[0]))robot_x += 0.01f;
        }
        break;
    case 's': // 앞으로
        if (!change_direction) {
            robot_dirction = 180.0f; mini_robot_x = 0.0f; mini_robot_z = 0.1f;
            robot_z -= 0.01f;
            if (robot_z <= -0.75f)change_direction = !change_direction;
            for (int i = 0; i < 4; ++i) {
                if (collide_robot_pillar(robot_x, robot_z, &pillar[i])) robot_z += 0.01f;
            }
            for (int i = 0; i < 49; ++i) {
                if (tile[i].y >= 0.0f && collide_robot_obstacle(robot_x, robot_z, &tile[i]))robot_z += 0.01f;
            }
            if (!on_bridge && collide_robot_bridge(robot_x, robot_z, robot_y, bridge[0]))robot_z += 0.01f;
        }
        else {
            robot_dirction = 0.0f; mini_robot_x = 0.0f; mini_robot_z = -0.1f;
            robot_z += 0.01f;
            if (robot_z >= 0.75f)change_direction = !change_direction;
            for (int i = 0; i < 4; ++i) {
                if (collide_robot_pillar(robot_x, robot_z, &pillar[i])) robot_z -= 0.01f;
            }
            for (int i = 0; i < 49; ++i) {
                if (tile[i].y >= 0.0f && collide_robot_obstacle(robot_x, robot_z, &tile[i]))robot_z -= 0.01f;
            }
            if (!on_bridge && collide_robot_bridge(robot_x, robot_z, robot_y, bridge[0]))robot_z -= 0.01f;
        }
        break;
    case 'd': // 오른쪽으로
        if (!change_direction) {
            robot_dirction = 270.0f; mini_robot_x = 0.1f; mini_robot_z = 0.0f;
            robot_x -= 0.01f;
            if (robot_x <= -0.75f)change_direction = !change_direction;
            for (int i = 0; i < 4; ++i) {
                if (collide_robot_pillar(robot_x, robot_z, &pillar[i])) robot_x += 0.01f;
            }
            for (int i = 0; i < 49; ++i) {
                if (tile[i].y >= 0.0f && collide_robot_obstacle(robot_x, robot_z, &tile[i]))robot_x += 0.01f;
            }
            if (!on_bridge && collide_robot_bridge(robot_x, robot_z, robot_y, bridge[0]))robot_x += 0.01f;
        }
        else {
            robot_dirction = 90.0f; mini_robot_x = -0.1f; mini_robot_z = 0.0f;
            robot_x += 0.01f;
            if (robot_x >= 0.75f)change_direction = !change_direction;
            for (int i = 0; i < 4; ++i) {
                if (collide_robot_pillar(robot_x, robot_z, &pillar[i])) robot_x -= 0.01f;
            }
            for (int i = 0; i < 49; ++i) {
                if (tile[i].y >= 0.0f && collide_robot_obstacle(robot_x, robot_z, &tile[i]))robot_x -= 0.01f;
            }
            if (!on_bridge && collide_robot_bridge(robot_x, robot_z, robot_y, bridge[0]))robot_x -= 0.01f;
        }
        break;
    case '+': // 걷는 속도 빨라짐 각도도 크게
        robot_move_speed += 0.5f; robot_move_range += 5.0f;
        break;
    case '-': // 걷는 속도 느려짐 각도도 작게
        if (robot_move_speed > 1.0f) {
            robot_move_speed -= 0.5f;
        }
        if (robot_move_range > 2.0f) {
            robot_move_range -= 5.0f;
        }
        break;
    case 'j': // 점프
        jump_timer = true;
        break;
    case 'i': // 모든 변환 리셋
        robot_dirction = 0.0f; robot_x = 0.0f; robot_y = 0.15f; robot_z = 0.0f;
        camera_x = 0.0f; camera_z = 2.0f; change_direction = true; jump_timer = false;
        robot_move_rad = 30.0f; robot_move_speed = 1.0f; robot_move_range = 30.0f;
        camera_rotate_y = 0.0f;
        break;
    case 'z': // 카메라 앞으로
        camera_z -= 0.01f;
        break;
    case 'Z': // 카메라 뒤로
        camera_z += 0.01f;
        break;
    case 'x': // 카메라 좌로
        camera_x -= 0.01f;
        break;
    case 'X': // 카메라 우로
        camera_x += 0.01f;
        break;
    case 'y': // 카메라 공전 양의 방향
        if (camera_rotate != 1) {
            camera_rotate = 1;
        }
        else {
            camera_rotate = 0;
        }
        break;
    case 'Y': // 카메라 공전 음의 방향
        if (camera_rotate != 2) {
            camera_rotate = 2;
        }
        else {
            camera_rotate = 0;
        }
        break;
    case 't':
        mini_robot_show = !mini_robot_show;
        break;
    case 'f':
        mini_jump_timer = true;
        break;
    }
    glutPostRedisplay();
}

void TimerFunction(int value)
{
    if (robot_move_change) {
        robot_move_rad -= robot_move_speed;
        if (robot_move_rad < -robot_move_range) {
            robot_move_change = !robot_move_change;
            robot_move_rad = -robot_move_range;
        }
    }
    else {
        robot_move_rad += robot_move_speed;
        if (robot_move_rad > robot_move_range) {
            robot_move_change = !robot_move_change;
            robot_move_rad = robot_move_range;
        }
    }
    
    // 점프.
    if (jump_timer) {
        robot_y += 0.01;
        if (robot_y >= 0.5f) {
            jump_timer = false;
        }

        if (robot_dirction <= 0.0f)robot_z += 0.005f;
        else if (robot_dirction <= 90.0f)robot_x += 0.005f;
        else if (robot_dirction <= 180.0f)robot_z -= 0.005f;
        else if (robot_dirction <= 270.0f)robot_x -= 0.005f;
    }
    else {
        if (robot_y <= 0.2f);
        else {
            robot_y -= 0.01;

            for (int i = 0; i < 49; ++i) {
                if (tile[i].y >= 0.0f && collide_robot_obstacle(robot_x, robot_z, &tile[i])) {
                    tile[i].fall_switch = true;
                    robot_y = tile[i].y + 0.3f;
                }
            }
            if (!on_bridge && collide_robot_bridge(robot_x, robot_z, robot_y, bridge[0])) {
                robot_y = bridge[0].y + 0.2f;
                on_bridge = true;
            }
        }
    }

    if (mini_jump_timer) {
        if (mini_robot_y > 0.0f)mini_robot_y -= 0.01f;
        else mini_jump_timer = false;
    }
    else if(!mini_jump_timer){
        if (mini_robot_y < 0.1f)mini_robot_y += 0.01f;
    }

    for (int i = 0; i < 49; ++i) {
        if (tile[i].fall_switch) {
            if (tile[i].fall == 1) {
                if (tile[i].y > -0.1f) {
                    tile[i].y -= 0.001f;
                    if (collide_robot_obstacle(robot_x, robot_z, &tile[i]))robot_y = tile[i].y + 0.3f;
                }
            }
            else if (tile[i].fall == 2) {
                tile[i].y -= 0.001f;
                if (robot_y >= 0.2f && collide_robot_obstacle(robot_x, robot_z, &tile[i]))robot_y = tile[i].y + 0.3f;
            }
        }
    }
    if (on_bridge) {
        robot_y = bridge[0].y + 0.2f;
        if (collide_robot_bridge(robot_x, robot_z, robot_y, bridge[0]));
        else on_bridge = false;
    }

    // 카메라 y축 회전
    if (camera_rotate == 1) {
        camera_rotate_y += 0.1f;
    }
    else if (camera_rotate == 2) {
        camera_rotate_y -= 0.1f;
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
    else if (a == 1) {
        for (int i = 0; i < faceNum; i += 2) {
            for (int j = 0; j < 3; ++j) {
                p[i].color[j * 3] = TILE_COLOR1_R;
                p[i].color[j * 3 + 1] = TILE_COLOR1_G;
                p[i].color[j * 3 + 2] = TILE_COLOR1_B;

                p[i + 1].color[j * 3] = TILE_COLOR1_R;
                p[i + 1].color[j * 3 + 1] = TILE_COLOR1_G;
                p[i + 1].color[j * 3 + 2] = TILE_COLOR1_B;
            }
        }
    }
    else if (a == 2) {
        for (int i = 0; i < faceNum; i += 2) {
            for (int j = 0; j < 3; ++j) {
                p[i].color[j * 3] = TILE_COLOR2_R;
                p[i].color[j * 3 + 1] = TILE_COLOR2_G;
                p[i].color[j * 3 + 2] = TILE_COLOR2_B;

                p[i + 1].color[j * 3] = TILE_COLOR2_R;
                p[i + 1].color[j * 3 + 1] = TILE_COLOR2_G;
                p[i + 1].color[j * 3 + 2] = TILE_COLOR2_B;
            }
        }
    }
    else if (a == 3) {
        for (int i = 0; i < faceNum; i += 2) {
            for (int j = 0; j < 3; ++j) {
                p[i].color[j * 3] = BRIDGE_COLOR_R;
                p[i].color[j * 3 + 1] = BRIDGE_COLOR_G;
                p[i].color[j * 3 + 2] = BRIDGE_COLOR_B;

                p[i + 1].color[j * 3] = BRIDGE_COLOR_R;
                p[i + 1].color[j * 3 + 1] = BRIDGE_COLOR_G;
                p[i + 1].color[j * 3 + 2] = BRIDGE_COLOR_B;
            }
        }
    }
}
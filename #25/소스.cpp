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
#include <fstream>

#define WINDOWX 800
#define WINDOWY 800


#define PI 3.141592

using namespace std;

random_device rd;
default_random_engine dre(rd());
uniform_real_distribution<float>uid(0.0f, 1.0f);

void make_vertexShaders();
void make_fragmentShaders();
GLuint make_shaderProgram();
GLvoid InitBuffer();
void InitShader();
GLchar* filetobuf(const char* file);
void LoadOBJ(const char* filename, vector<glm::vec3>& out_vertex, vector<glm::vec2>& out_uvs, vector<glm::vec3>& out_normals);

GLuint shaderID;
GLint width, height;

GLuint vertexShader;
GLuint fragmentShader;

GLuint VAO, VBO[2];

bool shape_select = false;

class Plane {
public:
    vector<glm::vec3> vertex;
    vector<glm::vec3> color;
    vector<glm::vec3> normals;
    vector<glm::vec2> uvs;

    void Bind() {
        glBindBuffer(GL_ARRAY_BUFFER, VBO[0]);
        glBufferData(GL_ARRAY_BUFFER, vertex.size() * sizeof(glm::vec3), vertex.data(), GL_STATIC_DRAW);
        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, 0);
        glEnableVertexAttribArray(0);

        glBindBuffer(GL_ARRAY_BUFFER, VBO[1]);
        glBufferData(GL_ARRAY_BUFFER, normals.size() * sizeof(glm::vec3), normals.data(), GL_STATIC_DRAW);
        glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 0, 0);
        glEnableVertexAttribArray(1);
    }
    void Draw() {
        glBindVertexArray(VAO);
        if (!shape_select)
            glDrawArrays(GL_TRIANGLES, 0, vertex.size());
        else 
            glDrawArrays(GL_TRIANGLE_FAN, 0, vertex.size());
    }
};

Plane Cube;
Plane Sphere;
Plane Light;

struct Dot {
    float x;
    float y;
    float z;
};
struct Circle {
    vector<Dot>dot;
    float r;
};
Dot new_dot;
Circle circle;

float BackGround[] = { 0.0, 0.0, 0.0 };

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
    glGenVertexArrays(1, &VAO);
    glGenBuffers(2, VBO);
    glBindVertexArray(VAO);
}

void main(int argc, char** argv) //--- 윈도우 출력하고 콜백함수 설정 { //--- 윈도우 생성하기
{
    srand((unsigned int)time(NULL));
    glutInit(&argc, argv); // glut 초기화
    glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGBA | GLUT_DEPTH); // 디스플레이 모드 설정
    glutInitWindowPosition(0, 0); // 윈도우의 위치 지정
    glutInitWindowSize(WINDOWX, WINDOWY); // 윈도우의 크기 지정
    glutCreateWindow("실습 25");// 윈도우 생성   (윈도우 이름)
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
float camera_z = 5.0f, camera_x = 0.0f;
int camera_rotate = 0;
float camera_rotate_y = 0.0f;
float rotate_y = 0.0f;

bool light_switch = true;
float light_x = 0.0f, light_z = 2.0f;

bool light_move_switch = false;
float light_angle = 0.0;
float light_box_angle = 0.0f;
float light_distance = 2.0f;

GLvoid drawScene() //--- 콜백 함수: 그리기 콜백 함수 
{
    if (start) {
        start = FALSE;
        LoadOBJ("cube.obj", Light.vertex, Light.uvs, Light.normals);
        LoadOBJ("cube.obj", Cube.vertex, Cube.uvs, Cube.normals);
        LoadOBJ("sphere.obj", Sphere.vertex, Sphere.uvs, Sphere.normals);


        circle.r = 20.0f;
        
        glEnable(GL_DEPTH_TEST);

    } // 초기화할 데이터

    glViewport(0, 0, 800, 800);
    glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);    //배경

    glUseProgram(shaderID);
    glBindVertexArray(VAO);// 쉐이더 , 버퍼 배열 사용

    glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGB | GLUT_DEPTH);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    unsigned int modelLocation = glGetUniformLocation(shaderID, "model");
    unsigned int viewLocation = glGetUniformLocation(shaderID, "view");
    unsigned int projLocation = glGetUniformLocation(shaderID, "projection");


    unsigned int lightPosLocation = glGetUniformLocation(shaderID, "lightPos"); 
    unsigned int lightColorLocation = glGetUniformLocation(shaderID, "lightColor"); 
    unsigned int objColorLocation = glGetUniformLocation(shaderID, "objectColor"); 
    glm::mat4 Vw = glm::mat4(1.0f);
    glm::mat4 Pj = glm::mat4(1.0f);
    glm::vec3 cameraPos;
    glm::vec3 cameraDirection;
    glm::vec3 cameraUp;

    cameraPos = glm::vec4(3.0f, 3.0f, 3.0f, 1.0f); //--- 카메라 위치
    cameraDirection = glm::vec3(0.0f, 0.0f, 0.0f); //--- 카메라 바라보는 방향
    cameraUp = glm::vec3(0.0f, 1.0f, 0.0f); //--- 카메라 위쪽 방향
    Vw = glm::mat4(1.0f);
    Vw = glm::lookAt(cameraPos, cameraDirection, cameraUp);
    glUniformMatrix4fv(viewLocation, 1, GL_FALSE, &Vw[0][0]);

    Pj = glm::mat4(1.0f);
    Pj = glm::perspective(glm::radians(45.0f), (float)WINDOWX / (float)WINDOWY, 0.1f, 100.0f);
    glUniformMatrix4fv(projLocation, 1, GL_FALSE, &Pj[0][0]);

    glUniform3f(lightPosLocation, light_x - 0.5f, 0.0f, light_z - 0.5f); //--- lightPos 값 전달: (0.0, 0.0, 5.0);
    if (light_switch) {
        glUniform3f(lightColorLocation, 1.0, 1.0, 1.0); //--- lightColor 값 전달: (1.0, 1.0, 1.0) 백색
    }
    else {
        glUniform3f(lightColorLocation, 0.0, 0.0, 0.0); //--- lightColor 값 전달: (1.0, 1.0, 1.0) 백색
    }
    // 그리기 코드 
    // 부모행렬 -> 공전 -> 이동 -> 자전 -> 스케일
    // 조명
    if (light_move_switch) {
        circle.dot.clear();
        double radian = 0.0f, degree = 0.0f;
        for (int i = 0; i < 360; ++i) {
            radian = i * PI / 100;
            new_dot.x = circle.r * cos(radian);
            new_dot.y = 0.0f;
            new_dot.z = circle.r * sin(radian);
            circle.dot.push_back(new_dot);
        }

        glBindBuffer(GL_ARRAY_BUFFER, VBO[0]);
        glBufferData(GL_ARRAY_BUFFER, circle.dot.size() * sizeof(Dot), circle.dot.data(), GL_STATIC_DRAW);
        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, 0);
        glEnableVertexAttribArray(0);

        glUniform3f(objColorLocation, 1.0f, 1.0f, 1.0f);

        glDrawArrays(GL_LINE_STRIP, 0, circle.dot.size());
    }
    TR = glm::mat4(1.0f);

    TR = glm::translate(TR, glm::vec3(light_x, 0.0f, light_z));
    TR = glm::scale(TR, glm::vec3(0.0002f, 0.0002f, 0.0002f));
    glUniformMatrix4fv(modelLocation, 1, GL_FALSE, glm::value_ptr(TR));
    glUniform3f(objColorLocation, 1.0f, 1.0f, 1.0f); //--- object Color값 전달: (1.0, 0.5, 0.3)의 색

    Sphere.Bind();
    Sphere.Draw();

    if (!shape_select) {
        TR = glm::mat4(1.0f);
        TR = glm::translate(TR, glm::vec3(0.0f, 0.0f, 0.0f));
        TR = glm::rotate(TR, (float)glm::radians(rotate_y), glm::vec3(0.0f, 1.0f, 0.0f));
        TR = glm::scale(TR, glm::vec3(0.1f, 0.1f, 0.1f));
        glUniformMatrix4fv(modelLocation, 1, GL_FALSE, glm::value_ptr(TR));
        
        glUniform3f(objColorLocation, 0.419, 0.662, 0.909); //--- object Color값 전달: (1.0, 0.5, 0.3)의 색

        Cube.Bind();
        Cube.Draw();
    }
    else {
        TR = glm::mat4(1.0f);
        TR = glm::translate(TR, glm::vec3(0.0f, 0.0f, 0.0f));
        TR = glm::rotate(TR, (float)glm::radians(rotate_y), glm::vec3(0.0f, 1.0f, 0.0f));
        TR = glm::scale(TR, glm::vec3(0.0002f, 0.0002f, 0.0002f));
        glUniformMatrix4fv(modelLocation, 1, GL_FALSE, glm::value_ptr(TR));
        
        glUniform3f(objColorLocation, 0.909, 0.419, 0.564); //--- object Color값 전달: (1.0, 0.5, 0.3)의 색

        Sphere.Bind();
        Sphere.Draw();
    }

    glutSwapBuffers();
    glutPostRedisplay();
}

GLvoid Reshape(int w, int h) //--- 콜백 함수: 다시 그리기 콜백 함수
{

}

bool rotate_y_timer = false;
void keyboard(unsigned char key, int x, int y) {
    switch (key) {
    case 'n': // 육면체 / 사각뿔 그리기
        shape_select = !shape_select;
        break;
    case 'm': // 조명 켜기 / 끄기
        light_switch = !light_switch;
        break;
    case 'y': // 객체를 y축에 대하여 회전
        rotate_y_timer = !rotate_y_timer;
        break;
    case 'r': // 조명을 y축에 대하여 양의 방형으로 공전시키기 // 조명의 공전궤도를 원으로 그린다?
        light_move_switch = !light_move_switch;
        break;
    case 'z':
        light_distance -= 0.1f;
        circle.r -= 1.0f;
        break;
    case 'Z':
        light_distance += 0.1f;
        circle.r += 1.0f;
        break;
    case 'q':
        glutDestroyWindow(1);
        break;
    }
    glutPostRedisplay();
}

void TimerFunction(int value)
{
    /*if (rotate_y_timer) {
        rotate_y++;
    }
    if (light_move_switch) {
        light_angle += 0.01f;
    }
    light_x = light_distance * sin(light_angle);
    light_z = light_distance * cos(light_angle);*/
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

void LoadOBJ(const char* filename, vector<glm::vec3>& out_vertex, vector<glm::vec2>& out_uvs, vector<glm::vec3>& out_normals)
{
    vector<int> vertexindices, uvindices, normalindices;
    vector<GLfloat> temp_vertex;
    vector<GLfloat> temp_uvs;
    vector<GLfloat> temp_normals;
    ifstream in(filename, ios::in);
    if (in.fail()) {
        cout << "Impossible to open file" << endl;
        return;
    }
    while (!in.eof()) {
        string lineHeader;
        in >> lineHeader;
        if (lineHeader == "v") {
            glm::vec3 vertex;
            in >> vertex.x >> vertex.y >> vertex.z;
            temp_vertex.push_back(vertex.x);
            temp_vertex.push_back(vertex.y);
            temp_vertex.push_back(vertex.z);
        }
        else if (lineHeader == "vt") {
            glm::vec2 uv;
            in >> uv.x >> uv.y;
            temp_uvs.push_back(uv.x);
            temp_uvs.push_back(uv.y);
        }
        else if (lineHeader == "vn") {
            glm::vec3 normal;
            in >> normal.x >> normal.y >> normal.z;
            temp_normals.push_back(normal.x);
            temp_normals.push_back(normal.y);
            temp_normals.push_back(normal.z);
        }
        else if (lineHeader == "f") {
            string vertex1, vertex2, vertex3;
            unsigned int vertexindex[3], uvindex[3], normalindex[3];
            for (int k = 0; k < 3; ++k) {
                string temp, temp2;
                int cnt{ 0 }, cnt2{ 0 };
                in >> temp;
                while (1) {
                    while ((int)temp[cnt] != 47 && cnt < temp.size()) {
                        temp2 += (int)temp[cnt];
                        cnt++;
                    }
                    if ((int)temp[cnt] == 47 && cnt2 == 0) {
                        vertexindex[k] = atoi(temp2.c_str());
                        vertexindices.push_back(vertexindex[k]);
                        cnt++; cnt2++;
                        temp2.clear();
                    }
                    else if ((int)temp[cnt] == 47 && cnt2 == 1) {
                        uvindex[k] = atoi(temp2.c_str());
                        uvindices.push_back(uvindex[k]);
                        cnt++; cnt2++;
                        temp2.clear();
                    }
                    else if (temp[cnt] = '\n' && cnt2 == 2) {
                        normalindex[k] = atoi(temp2.c_str());
                        normalindices.push_back(normalindex[k]);
                        break;
                    }
                }
            }
        }
        else {
            continue;
        }
    }
    for (int i = 0; i < vertexindices.size(); ++i) {
        unsigned int vertexIndex = vertexindices[i];
        vertexIndex = (vertexIndex - 1) * 3;
        glm::vec3 vertex = { temp_vertex[vertexIndex], temp_vertex[vertexIndex + 1], temp_vertex[vertexIndex + 2] };
        out_vertex.push_back(vertex);
    }
    for (unsigned int i = 0; i < uvindices.size(); ++i) {
        unsigned int uvIndex = uvindices[i];
        uvIndex = (uvIndex - 1) * 2;
        glm::vec2 uv = { temp_uvs[uvIndex], temp_uvs[uvIndex + 1] };
        out_uvs.push_back(uv);
    }
    for (unsigned int i = 0; i < normalindices.size(); ++i) {
        unsigned int normalIndex = normalindices[i];
        normalIndex = (normalIndex - 1) * 3;
        glm::vec3 normal = { temp_normals[normalIndex], temp_normals[normalIndex + 1], temp_normals[normalIndex + 2] };
        out_normals.push_back(normal);
    }
}
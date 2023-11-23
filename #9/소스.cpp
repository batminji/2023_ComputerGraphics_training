#define _CRT_SECURE_NO_WARNINGS //--- 프로그램 맨 앞에 선언할 것
#include <stdlib.h>
#include <stdio.h>
#include <iostream>
#include <random>
#include <math.h>
#include <gl/glew.h>
#include <gl/freeglut.h>
#include <gl/freeglut_ext.h>

#define PI 3.141592

// 문자열 읽어오기 샘플 코드
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

//--- 아래 5개 함수는 사용자 정의 함수임
void make_vertexShaders();
void make_fragmentShaders();
void make_shaderProgram();

GLvoid drawScene();
GLvoid Reshape(int w, int h);

struct RT {
    float left, right, top, bottom;
    int dir_x,dir_y;
    float speed;
    int direction;
    float distance_to_go; // 가야할 거리
    float distance_ed; // 간 거리
    float r;
    float degree;
};

RT rt[4]{
    { -0.6f, -0.3f, 0.6f, 0.3f, 1, 1, 0.01, 0 },
    { 0.3f, 0.6f, 0.6f, 0.3f, -1, 1, 0.02, 1},
    { -0.6f, -0.3f, -0.3f, -0.6f, -1, -1, 0.03 , 2},
    { 0.3f, 0.6f, -0.3f, -0.6f, 1, -1, 0.04, 3}
};

bool collide_x(RT rt) {
    if (rt.left < -1.0f || rt.right > 1.0f) return 1;
    else return 0;
}

bool collide_y(RT rt) {
    if (rt.top > 1.0f || rt.bottom < -1.0f)return 1;
    else return 0;
}

GLfloat TriShape[4][3][3] = {
    {{-0.45, 0.6,0.0}, {-0.6, 0.3, 0.0}, {-0.3, 0.3, 0.0}},
    {{0.45, 0.6, 0.0}, {0.3, 0.3, 0.0}, {0.6, 0.3, 0.0}},
    {{-0.45, -0.3, 0.0}, {-0.6, -0.6, 0.0}, {-0.3, -0.6, 0.0}},
    {{0.45, -0.3, 0.0}, {0.3, -0.6, 0.0}, {0.6, -0.6, 0.0}}
};

GLfloat colors[4][3][3] = {
    {{1.0, 0.0, 0.0}, {0.7, 0.0, 0.0}, {0.4, 0.0, 0.0}}, // red
    {{0.0, 1.0, 0.0}, {0.0, 0.7, 0.0}, {0.0, 0.4, 0.0}}, // green
    {{0.0, 0.0, 1.0}, {0.0, 0.0, 0.7}, {0.0, 0.0, 0.4}}, // blue
    {{1.0, 1.0, 0.0}, {0.7, 0.7, 0.0}, {0.5, 0.4, 0.0}} // yellow
};

GLchar* vertexSource, * fragmentSource; //--- 소스코드 저장 변수
GLuint vertexShader, fragmentShader; //--- 세이더 객체
GLuint shaderProgramID;
GLuint vao, vbo[2];

void rotation(int i) {
    if (TriShape[i][0][0] > TriShape[i][1][0] && TriShape[i][0][0] > TriShape[i][2][0]) {
        TriShape[i][0][0] = (rt[i].left + rt[i].right) / 2; TriShape[i][0][1] = rt[i].top;
        TriShape[i][1][0] = rt[i].left; TriShape[i][1][1] = rt[i].bottom;
        TriShape[i][2][0] = rt[i].right; TriShape[i][2][1] = rt[i].bottom;
    }
    else if (TriShape[i][0][0] < TriShape[i][1][0] && TriShape[i][0][0] < TriShape[i][2][0]) {
        TriShape[i][0][0] = (rt[i].left + rt[i].right) / 2; TriShape[i][0][1] = rt[i].bottom;
        TriShape[i][1][0] = rt[i].right; TriShape[i][1][1] = rt[i].top;
        TriShape[i][2][0] = rt[i].left; TriShape[i][2][1] = rt[i].top;
    }
    else if (TriShape[i][0][1] > TriShape[i][1][1] && TriShape[i][0][1] > TriShape[i][2][1]) {
        TriShape[i][0][0] = rt[i].right; TriShape[i][0][1] = (rt[i].top + rt[i].bottom) / 2;
        TriShape[i][1][0] = rt[i].left; TriShape[i][1][1] = rt[i].top;
        TriShape[i][2][0] = rt[i].left; TriShape[i][2][1] = rt[i].bottom;
    }
    else if (TriShape[i][0][1] < TriShape[i][1][1] && TriShape[i][0][1] < TriShape[i][2][1]) {
        TriShape[i][0][0] = rt[i].left; TriShape[i][0][1] = (rt[i].top + rt[i].bottom) / 2;
        TriShape[i][1][0] = rt[i].right; TriShape[i][1][1] = rt[i].bottom;
        TriShape[i][2][0] = rt[i].right; TriShape[i][2][1] = rt[i].top;
    }
}

//--- 세이더 프로그램 만들고 세이더 객체 링크하기
void make_shaderProgram()
{
    make_vertexShaders(); //--- 버텍스 세이더 만들기
    make_fragmentShaders(); //--- 프래그먼트 세이더 만들기

    //-- shader Program
    shaderProgramID = glCreateProgram();

    glAttachShader(shaderProgramID, vertexShader);
    glAttachShader(shaderProgramID, fragmentShader);
    glLinkProgram(shaderProgramID);

    //--- 세이더 삭제하기
    glDeleteShader(vertexShader);
    glDeleteShader(fragmentShader);

    //--- Shader Program 사용하기
    glUseProgram(shaderProgramID);
}
//--- 버텍스 세이더 객체 만들기
void make_vertexShaders()
{
    vertexSource = filetobuf("vertex.glsl");

    //--- 버텍스 세이더 객체 만들기
    vertexShader = glCreateShader(GL_VERTEX_SHADER);

    //--- 세이더 코드를 세이더 객체에 넣기
    glShaderSource(vertexShader, 1, (const GLchar**)&vertexSource, 0);

    //--- 버텍스 세이더 컴파일하기
    glCompileShader(vertexShader);

    //--- 컴파일이 제대로 되지 않은 경우: 에러 체크
    GLint result;
    GLchar errorLog[512];
    glGetShaderiv(vertexShader, GL_COMPILE_STATUS, &result);
    if (!result)
    {
        glGetShaderInfoLog(vertexShader, 512, NULL, errorLog);
        std::cout << "ERROR: vertex shader 컴파일 실패\n" << errorLog << std::endl;
        return;
    }
}
//--- 프래그먼트 세이더 객체 만들기
void make_fragmentShaders()
{
    fragmentSource = filetobuf("fragment.glsl");

    //--- 프래그먼트 세이더 객체 만들기
    fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);

    //--- 세이더 코드를 세이더 객체에 넣기
    glShaderSource(fragmentShader, 1, (const GLchar**)&fragmentSource, 0);

    //--- 프래그먼트 세이더 컴파일
    glCompileShader(fragmentShader);

    //--- 컴파일이 제대로 되지 않은 경우: 컴파일 에러 체크
    GLint result;
    GLchar errorLog[512];
    glGetShaderiv(fragmentShader, GL_COMPILE_STATUS, &result);
    if (!result)
    {
        glGetShaderInfoLog(fragmentShader, 512, NULL, errorLog);
        std::cout << "ERROR: fragment shader 컴파일 실패\n" << errorLog << std::endl;
        return;
    }
}

void InitBuffer()
{
    glGenVertexArrays(1, &vao); //--- VAO 를 지정하고 할당하기
    glBindVertexArray(vao); //--- VAO를 바인드하기
    glGenBuffers(2, vbo); //--- 3개의 VBO를 지정하고 할당하기

    /*glBindBuffer(GL_ARRAY_BUFFER, vbo[0]);
    glBufferData(GL_ARRAY_BUFFER, 18 * sizeof(GLfloat), rectShape[i], GL_STATIC_DRAW);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, 0);
    glEnableVertexAttribArray(0);

    glBindBuffer(GL_ARRAY_BUFFER, vbo[1]);
    glBufferData(GL_ARRAY_BUFFER, 18 * sizeof(GLfloat), colors, GL_STATIC_DRAW);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 0, 0);
    glEnableVertexAttribArray(1);*/
}

bool Diagonal = false;
void Diagonal_Timer(int value)
{
    for (int i = 0; i < 4; ++i) {
        // 우선 사각형을 옮기기
        rt[i].left += rt[i].dir_x * rt[i].speed;
        rt[i].right += rt[i].dir_x * rt[i].speed;
        rt[i].top += rt[i].dir_y * rt[i].speed;
        rt[i].bottom += rt[i].dir_y * rt[i].speed;
        // 삼각형 좌표도 옮기기
        TriShape[i][0][0] += rt[i].dir_x * rt[i].speed; TriShape[i][0][1] += rt[i].dir_y * rt[i].speed;
        TriShape[i][1][0] += rt[i].dir_x * rt[i].speed; TriShape[i][1][1] += rt[i].dir_y * rt[i].speed;
        TriShape[i][2][0] += rt[i].dir_x * rt[i].speed; TriShape[i][2][1] += rt[i].dir_y * rt[i].speed;
        // 충돌 검사 후에 충돌 했으면 방향 변경 및 삼각형 돌리기
        if (collide_x(rt[i])) rt[i].dir_x = -rt[i].dir_x;
        if (collide_y(rt[i])) rt[i].dir_y = -rt[i].dir_y;
        if (collide_x(rt[i]) || collide_y(rt[i]))rotation(i);
    }
    glutPostRedisplay();
    if (Diagonal) glutTimerFunc(30, Diagonal_Timer, 1);
}

bool Zigzag = false;
int Zigsag_cnt = 0;
void Zigzag_Timer(int value)
{
    for (int i = 0; i < 4; ++i) {
        switch (rt[i].direction) {
        case 0: // 우로 이동하다가 아래로
            if (rt[i].right >= 1.0f) {
                if (Zigsag_cnt < 5) {
                    rt[i].top -= rt[i].speed; rt[i].bottom -= rt[i].speed;
                    TriShape[i][0][1] -= rt[i].speed; TriShape[i][1][1] -= rt[i].speed; TriShape[i][2][1] -= rt[i].speed;
                    Zigsag_cnt++;
                }
                else {
                    if (rt[i].bottom < -1.0f) {
                        rt[i].direction = 3;
                        Zigsag_cnt = 0;
                        rotation(i);
                    }
                    else {
                        rt[i].direction = 1;
                        Zigsag_cnt = 0;
                        rotation(i);
                    }
                }
            }
            else {
                rt[i].left += rt[i].speed; rt[i].right += rt[i].speed;
                TriShape[i][0][0] += rt[i].speed; TriShape[i][1][0] += rt[i].speed; TriShape[i][2][0] += rt[i].speed;
            }
            break;
        case 1: // 좌로 이동하다가 아래로
            if (rt[i].left <= -1.0f) {
                if (Zigsag_cnt < 5) {
                    rt[i].top -= rt[i].speed; rt[i].bottom -= rt[i].speed;
                    TriShape[i][0][1] -= rt[i].speed; TriShape[i][1][1] -= rt[i].speed; TriShape[i][2][1] -= rt[i].speed;
                    Zigsag_cnt++;
                }
                else {
                    if (rt[i].bottom < -1.0f) {
                        rt[i].direction = 2;
                        Zigsag_cnt = 0;
                        rotation(i);
                    }
                    else {
                        rt[i].direction = 0;
                        Zigsag_cnt = 0;
                        rotation(i);
                    }
                }
            }
            else {
                rt[i].left -= rt[i].speed; rt[i].right -= rt[i].speed;
                TriShape[i][0][0] -= rt[i].speed; TriShape[i][1][0] -= rt[i].speed; TriShape[i][2][0] -= rt[i].speed;
            }
            break;
        case 2: // 우로 이동하다가 위로
            if (rt[i].right >= 1.0f) {
                if (Zigsag_cnt < 5) {
                    rt[i].top += rt[i].speed; rt[i].bottom += rt[i].speed;
                    TriShape[i][0][1] += rt[i].speed; TriShape[i][1][1] += rt[i].speed; TriShape[i][2][1] += rt[i].speed;
                    Zigsag_cnt++;
                }
                else {
                    if (rt[i].top > 1.0f) {
                        rt[i].direction = 1;
                        Zigsag_cnt = 0;
                        rotation(i);
                    }
                    else {
                        rt[i].direction = 3;
                        Zigsag_cnt = 0;
                        rotation(i);
                    }
                }
            }
            else {
                rt[i].left += rt[i].speed; rt[i].right += rt[i].speed;
                TriShape[i][0][0] += rt[i].speed; TriShape[i][1][0] += rt[i].speed; TriShape[i][2][0] += rt[i].speed;
            }
            break;
        case 3: // 좌로 이동하다가 위로
            if (rt[i].left <= -1.0f) {
                if (Zigsag_cnt < 5) {
                    rt[i].top += rt[i].speed; rt[i].bottom += rt[i].speed;
                    TriShape[i][0][1] += rt[i].speed; TriShape[i][1][1] += rt[i].speed; TriShape[i][2][1] += rt[i].speed;
                    Zigsag_cnt++;
                }
                else {
                    if (rt[i].top > 1.0f) {
                        rt[i].direction = 0;
                        Zigsag_cnt = 0;
                        rotation(i);
                    }
                    else {
                        rt[i].direction = 2;
                        Zigsag_cnt = 0;
                        rotation(i);
                    }
                }
            }
            else {
                rt[i].left -= rt[i].speed; rt[i].right -= rt[i].speed;
                TriShape[i][0][0] -= rt[i].speed; TriShape[i][1][0] -= rt[i].speed; TriShape[i][2][0] -= rt[i].speed;
            }
            break;
        }
    }
    glutPostRedisplay();
    if (Zigzag) glutTimerFunc(30, Zigzag_Timer, 1);
}

bool Rectmove = false;
void Rectmove_Timer(int value)
{
    for (int i = 0; i < 4; ++i) {
        switch (rt[i].direction) {
        case 0: // 좌로 이동
            rt[i].left -= rt[i].speed; rt[i].right -= rt[i].speed;
            TriShape[i][0][0] -= rt[i].speed; TriShape[i][1][0] -= rt[i].speed; TriShape[i][2][0] -= rt[i].speed;
            rt[i].distance_ed += rt[i].speed;
            if (rt[i].distance_ed >= rt[i].distance_to_go) {
                rt[i].distance_to_go -= 0.1; rt[i].distance_ed = 0;
                rt[i].direction = 1;
            }
            break;
        case 1: // 아래로 이동
            rt[i].top -= rt[i].speed; rt[i].bottom -= rt[i].speed;
            TriShape[i][0][1] -= rt[i].speed; TriShape[i][1][1] -= rt[i].speed; TriShape[i][2][1] -= rt[i].speed;
            rt[i].distance_ed += rt[i].speed;
            if (rt[i].distance_ed >= rt[i].distance_to_go) {
                rt[i].distance_to_go -= 0.1; rt[i].distance_ed = 0;
                rt[i].direction = 2;
            }
            break;
        case 2: // 우로 이동
            rt[i].left += rt[i].speed; rt[i].right += rt[i].speed;
            TriShape[i][0][0] += rt[i].speed; TriShape[i][1][0] += rt[i].speed; TriShape[i][2][0] += rt[i].speed;
            rt[i].distance_ed += rt[i].speed;
            if (rt[i].distance_ed >= rt[i].distance_to_go) {
                rt[i].distance_to_go -= 0.1; rt[i].distance_ed = 0;
                rt[i].direction = 3;
            }
            break;
        case 3: // 위로 이동
            rt[i].top += rt[i].speed; rt[i].bottom += rt[i].speed;
            TriShape[i][0][1] += rt[i].speed; TriShape[i][1][1] += rt[i].speed; TriShape[i][2][1] += rt[i].speed;
            rt[i].distance_ed += rt[i].speed;
            if (rt[i].distance_ed >= rt[i].distance_to_go) {
                rt[i].distance_to_go -= 0.1; rt[i].distance_ed = 0;
                rt[i].direction = 0;
            }
            break;
        }
        // 중앙에 도착하면 초기화
        if (rt[i].distance_to_go <= 0.0) {
            rt[i].left = 0.7; rt[i].right = 1.0; rt[i].top = 1.0; rt[i].bottom = 0.7;
            TriShape[i][0][0] = (rt[i].left + rt[i].right) / 2; TriShape[i][0][1] = rt[i].top;
            TriShape[i][1][0] = rt[i].left; TriShape[i][1][1] = rt[i].bottom;
            TriShape[i][2][0] = rt[i].right; TriShape[i][2][1] = rt[i].bottom;
            rt[i].distance_to_go = 1.7f; rt[i].distance_ed = 0;
            rt[i].direction = 0;
        }
    }
    glutPostRedisplay();
    if (Rectmove) glutTimerFunc(10, Rectmove_Timer, 1);
}

bool Circlemove = false;
void Circlemove_Timer(int value)
{
    double radian = 0;
    for (int i = 0; i < 4; ++i) {
        radian = rt[i].degree * PI / 100;
        TriShape[i][0][0] = rt[i].r * cos(radian); TriShape[i][0][1] = rt[i].r * sin(radian);
        TriShape[i][1][0] = TriShape[i][0][0] - 0.15f; TriShape[i][1][1] = TriShape[i][0][1] - 0.3f;
        TriShape[i][2][0] = TriShape[i][0][0] + 0.15f; TriShape[i][2][1] = TriShape[i][0][1] - 0.3f;

        rt[i].left = TriShape[i][1][0]; rt[i].right = TriShape[i][2][0];
        rt[i].top = TriShape[i][0][1]; rt[i].bottom = TriShape[i][1][1];

        rt[i].degree += rt[i].speed * 100;
        rt[i].r -= 0.001;
        if (rt[i].r < 0) {
            rt[i].left = 0.7; rt[i].right = 1.0; rt[i].top = 1.0; rt[i].bottom = 0.7;
            TriShape[i][0][0] = (rt[i].left + rt[i].right) / 2; TriShape[i][0][1] = rt[i].top;
            TriShape[i][1][0] = rt[i].left; TriShape[i][1][1] = rt[i].bottom;
            TriShape[i][2][0] = rt[i].right; TriShape[i][2][1] = rt[i].bottom;
            rt[i].r = 1.0; rt[i].degree = 0;
        }
    }
    glutPostRedisplay();
    if (Circlemove) glutTimerFunc(30, Circlemove_Timer, 1);
}

GLvoid Keyboard(unsigned char key, int x, int y)
{
    switch (key) {
    case '1': // 대각선으로 움직이기
        if (Diagonal)Diagonal = false;
        else {
            Diagonal = true; Zigzag = false; Rectmove = false; Circlemove = false;
            glutTimerFunc(30, Diagonal_Timer, 1);
        }
        break;
    case '2': // 지그재그로 움직이기
        if (Zigzag)Zigzag = false;
        else {
            Zigzag = true; Diagonal = false; Rectmove = false; Circlemove = false;
            glutTimerFunc(30, Zigzag_Timer, 1);
        }
        break;
    case '3': // 사각 스파이럴
        if (Rectmove)Rectmove = false;
        else {
            Rectmove = true; Diagonal = false; Zigzag = false; Circlemove = false;
            for (int i = 0; i < 4; ++i) {
                rt[i].left = 0.7f; rt[i].right = 1.0f; rt[i].top = 1.0f; rt[i].bottom = 0.7f;
                TriShape[i][0][0] = (rt[i].left + rt[i].right) / 2; TriShape[i][0][1] = rt[i].top;
                TriShape[i][1][0] = rt[i].left; TriShape[i][1][1] = rt[i].bottom;
                TriShape[i][2][0] = rt[i].right; TriShape[i][2][1] = rt[i].bottom;
                rt[i].distance_to_go = 1.7f; rt[i].distance_ed = 0;
                rt[i].direction = 0;
            }
            glutTimerFunc(10, Rectmove_Timer, 1);
        }
        break;
    case '4': // 원 스파이럴
        if (Circlemove)Circlemove = false;
        else {
            Circlemove = true; Diagonal = false;; Zigzag = false; Rectmove = false;
            for (int i = 0; i < 4; ++i) {
                rt[i].left = 0.7; rt[i].right = 1.0; rt[i].top = 1.0; rt[i].bottom = 0.7;
                TriShape[i][0][0] = (rt[i].left + rt[i].right) / 2; TriShape[i][0][1] = rt[i].top;
                TriShape[i][1][0] = rt[i].left; TriShape[i][1][1] = rt[i].bottom;
                TriShape[i][2][0] = rt[i].right; TriShape[i][2][1] = rt[i].bottom;
                rt[i].r = 1.0; rt[i].degree = 0;
            }
            glutTimerFunc(30, Circlemove_Timer, 1);
        }
        break;
    }
    glutPostRedisplay();
}

GLvoid drawScene() //--- 콜백 함수: 그리기 콜백 함수
{
    GLfloat rColor, gColor, bColor;
    rColor = gColor = bColor = 1.0f;
    glClearColor(rColor, gColor, bColor, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT);

    //--- 렌더링 파이프라인에 세이더 불러오기
    glUseProgram(shaderProgramID);
    //--- 사용할 VAO 불러오기
    glBindVertexArray(vao);

    for (int i = 0; i < 4; ++i) {
        glBindBuffer(GL_ARRAY_BUFFER, vbo[0]);
        glBufferData(GL_ARRAY_BUFFER, 9 * sizeof(GLfloat), TriShape[i], GL_STATIC_DRAW);
        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, 0);
        glEnableVertexAttribArray(0);

        glBindBuffer(GL_ARRAY_BUFFER, vbo[1]);
        glBufferData(GL_ARRAY_BUFFER, 9 * sizeof(GLfloat), colors[i], GL_STATIC_DRAW);
        glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 0, 0);
        glEnableVertexAttribArray(1);

        glDrawArrays(GL_TRIANGLES, 0, 3);
    }

    glutSwapBuffers(); // 화면에 출력하기
}

GLvoid Reshape(int w, int h) //--- 콜백 함수: 다시 그리기 콜백 함수
{
    glViewport(0, 0, w, h);
}

//--- 메인 함수
void main(int argc, char** argv) //--- 윈도우 출력하고 콜백함수 설정
{
    //--- 윈도우 생성하기
    glutInit(&argc, argv);
    glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGBA);
    glutInitWindowPosition(0, 0);
    glutInitWindowSize(800, 800);
    glutCreateWindow("실습 9");

    //--- GLEW 초기화하기
    glewExperimental = GL_TRUE;
    glewInit();
    make_shaderProgram();
    InitBuffer();
    glutKeyboardFunc(Keyboard);
    glutDisplayFunc(drawScene); 
    glutReshapeFunc(Reshape);

    glutMainLoop();
}
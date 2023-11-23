#define _CRT_SECURE_NO_WARNINGS //--- 프로그램 맨 앞에 선언할 것
#include <stdlib.h>
#include <stdio.h>
#include <iostream>
#include <vector>
#include <random>
#include <time.h>
#include <gl/glew.h>
#include <gl/freeglut.h>
#include <gl/freeglut_ext.h>

using namespace std;

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

struct Shape {
    float left, right, top, bottom;
    int vertex_num;  // 꼭짓점 개수
    bool move;
    bool select;
    int dir_x, dir_y;
    float speed;
};

vector<Shape> shape;
Shape s;

// GLfloat temp_shape[15][5][3] = {};

GLfloat color_point[3] = { 1.0, 0.0, 0.0 };
GLfloat color_line[2][3] = {
    {0.0, 1.0, 0.0},
    {0.0, 1.0, 0.0}
};
GLfloat color_tri[3][3] = {
    {0.0, 0.0, 1.0},
    {0.0, 0.0, 1.0},
    {0.0, 0.0, 1.0}
};
GLfloat color_rect[4][3] = {
    {1.0, 1.0, 0.0},
    {1.0, 1.0, 0.0},
    {1.0, 1.0, 0.0},
    {1.0, 1.0, 0.0}
};
GLfloat color_penta[5][3] = {
    {0.0, 1.0, 1.0},
    {0.0, 1.0, 1.0},
    {0.0, 1.0, 1.0},
    {0.0, 1.0, 1.0},
    {0.0, 1.0, 1.0}
};

GLchar* vertexSource, * fragmentSource; //--- 소스코드 저장 변수
GLuint vertexShader, fragmentShader; //--- 세이더 객체
GLuint shaderProgramID;
GLuint vao, vbo[2];

bool is_select(Shape s, double mx, double my) {
    if (mx > s.left && mx < s.right && my > s.bottom && my < s.top) return true;
    else return false;
}

bool is_collide(Shape select_s, Shape s) {
    if (select_s.left < s.right && select_s.right > s.left && select_s.top > s.bottom && select_s.bottom < s.top)return true;
    else return false;
}
bool collide_x(Shape s) {
    if (s.left < -1.0f || s.right > 1.0f) return 1;
    else return 0;
}

bool collide_y(Shape s) {
    if (s.top > 1.0f || s.bottom < -1.0f)return 1;
    else return 0;
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

void Diagonal_Timer(int value)
{
    for (int i = 0; i < shape.size(); ++i) {
        if (shape[i].move) {
            shape[i].left += shape[i].dir_x * shape[i].speed;
            shape[i].right += shape[i].dir_x * shape[i].speed;
            shape[i].top += shape[i].dir_y * shape[i].speed;
            shape[i].bottom += shape[i].dir_y * shape[i].speed;

            if (collide_x(shape[i])) shape[i].dir_x = -shape[i].dir_x;
            if (collide_y(shape[i])) shape[i].dir_y = -shape[i].dir_y;
        }
    }
    glutPostRedisplay();
    glutTimerFunc(30, Diagonal_Timer, 1);
}

bool click = false;
float default_x, default_y;
int click_index;
void Mouse(int button, int state, int x, int y)
{
    double mx = ((double)x - 400) / 400;
    double my = -(((double)y - 400) / 400);

    if (button == GLUT_LEFT_BUTTON && state == GLUT_DOWN) {
        for (int i = 0; i < shape.size(); ++i) {
            if (is_select(shape[i], mx, my)) {
                shape[i].select = true;
                click_index = i;
                click = true;
                default_x = mx - shape[i].left;
                default_y = shape[i].top - my;
            }
        }
    }
    if (button == GLUT_LEFT_BUTTON && state == GLUT_UP) {
        click = false;
        // 합치기
        for (int i = 0; i < shape.size(); ++i) {
            if (click_index != i && is_collide(shape[click_index], shape[i])) {
                shape[click_index].vertex_num = (shape[click_index].vertex_num + shape[i].vertex_num) % 5;
                if (shape[click_index].vertex_num == 0)shape[click_index].vertex_num = 5;
                shape[click_index].left = shape[click_index].left; shape[click_index].right = shape[click_index].right;
                shape[click_index].top = shape[click_index].top; shape[click_index].bottom = shape[click_index].bottom;
                shape[click_index].move = true;
                shape[click_index].dir_x = -1; shape[click_index].dir_y = -1;
                shape[click_index].speed = 0.02;
                shape[click_index].select = false;
                shape.erase(shape.begin() + i);
            }
        }
    }

    glutPostRedisplay();
}

void Motion(int x, int y)
{
    double mx = ((double)x - 400) / 400;
    double my = -(((double)y - 400) / 400);

    if (click) {
        shape[click_index].left = mx - default_x;
        shape[click_index].right = shape[click_index].left + 0.3;
        shape[click_index].top = my + default_y;
        shape[click_index].bottom = shape[click_index].top - 0.3;
    }
    
    glutPostRedisplay();
}


GLvoid Keyboard(unsigned char key, int x, int y)
{
    switch (key) {
        
    }
    glutPostRedisplay();
}

GLfloat d_point[3] = {};
GLfloat d_line[2][3] = {};
GLfloat d_tri[3][3] = {};
GLfloat d_rect[4][3] = {};
GLfloat d_penta[5][3] = {};

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
    
    for (int i = 0; i < shape.size(); ++i) {
        switch (shape[i].vertex_num) {
        case 1: // 점
            d_point[0] = (shape[i].left + shape[i].right) / 2;
            d_point[1] = (shape[i].top + shape[i].bottom) / 2;
            glBindBuffer(GL_ARRAY_BUFFER, vbo[0]);
            glBufferData(GL_ARRAY_BUFFER, 3 * sizeof(GLfloat), d_point, GL_STATIC_DRAW);
            glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, 0);
            glEnableVertexAttribArray(0);
            glBindBuffer(GL_ARRAY_BUFFER, vbo[1]);
            glBufferData(GL_ARRAY_BUFFER, 3 * sizeof(GLfloat), color_point, GL_STATIC_DRAW);
            glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 0, 0);
            glEnableVertexAttribArray(1);

            glPointSize(10.0);
            glDrawArrays(GL_POINTS, 0, 1);

            glDisableVertexAttribArray(0);
            glDisableVertexAttribArray(1);
            break;
        case 2: // 선
            d_line[0][0] = shape[i].left; d_line[0][1] = shape[i].top;
            d_line[1][0] = shape[i].right; d_line[1][1] = shape[i].bottom;
            glBindBuffer(GL_ARRAY_BUFFER, vbo[0]);
            glBufferData(GL_ARRAY_BUFFER, 6 * sizeof(GLfloat), d_line, GL_STATIC_DRAW);
            glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, 0);
            glEnableVertexAttribArray(0);
            glBindBuffer(GL_ARRAY_BUFFER, vbo[1]);
            glBufferData(GL_ARRAY_BUFFER, 6 * sizeof(GLfloat), color_line, GL_STATIC_DRAW);
            glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 0, 0);
            glEnableVertexAttribArray(1);

            glLineWidth(5.0);
            glDrawArrays(GL_LINE_LOOP, 0, 2);

            glDisableVertexAttribArray(0);
            glDisableVertexAttribArray(1);
            break;
        case 3: // 삼각형
            d_tri[0][0] = (shape[i].left + shape[i].right) / 2; d_tri[0][1] = shape[i].top;
            d_tri[1][0] = shape[i].right; d_tri[1][1] = shape[i].bottom;
            d_tri[2][0] = shape[i].left; d_tri[2][1] = shape[i].bottom;
            glBindBuffer(GL_ARRAY_BUFFER, vbo[0]);
            glBufferData(GL_ARRAY_BUFFER, 9 * sizeof(GLfloat), d_tri, GL_STATIC_DRAW);
            glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, 0);
            glEnableVertexAttribArray(0);
            glBindBuffer(GL_ARRAY_BUFFER, vbo[1]);
            glBufferData(GL_ARRAY_BUFFER, 9 * sizeof(GLfloat), color_tri, GL_STATIC_DRAW);
            glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 0, 0);
            glEnableVertexAttribArray(1);

            glDrawArrays(GL_TRIANGLES, 0, 3);

            glDisableVertexAttribArray(0);
            glDisableVertexAttribArray(1);
            break;
        case 4: // 사각형
            d_rect[0][0] = shape[i].left; d_rect[0][1] = shape[i].top;
            d_rect[1][0] = shape[i].right; d_rect[1][1] = shape[i].top;
            d_rect[2][0] = shape[i].right; d_rect[2][1] = shape[i].bottom;
            d_rect[3][0] = shape[i].left; d_rect[3][1] = shape[i].bottom;
            glBindBuffer(GL_ARRAY_BUFFER, vbo[0]);
            glBufferData(GL_ARRAY_BUFFER, 12 * sizeof(GLfloat), d_rect, GL_STATIC_DRAW);
            glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, 0);
            glEnableVertexAttribArray(0);
            glBindBuffer(GL_ARRAY_BUFFER, vbo[1]);
            glBufferData(GL_ARRAY_BUFFER, 12 * sizeof(GLfloat), color_rect, GL_STATIC_DRAW);
            glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 0, 0);
            glEnableVertexAttribArray(1);

            glDrawArrays(GL_TRIANGLE_FAN, 0, 4);

            glDisableVertexAttribArray(0);
            glDisableVertexAttribArray(1);
            break;
        case 5: // 오각형
            d_penta[0][0] = (shape[i].left + shape[i].right) / 2; d_penta[0][1] = shape[i].top;
            d_penta[1][0] = shape[i].right; d_penta[1][1] = shape[i].top - 0.1;
            d_penta[2][0] = shape[i].right - 0.05; d_penta[2][1] = shape[i].bottom;
            d_penta[3][0] = shape[i].left + 0.05; d_penta[3][1] = shape[i].bottom;
            d_penta[4][0] = shape[i].left; d_penta[4][1] = shape[i].top - 0.1;

            glBindBuffer(GL_ARRAY_BUFFER, vbo[0]);
            glBufferData(GL_ARRAY_BUFFER, 15 * sizeof(GLfloat), d_penta, GL_STATIC_DRAW);
            glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, 0);
            glEnableVertexAttribArray(0);
            glBindBuffer(GL_ARRAY_BUFFER, vbo[1]);
            glBufferData(GL_ARRAY_BUFFER, 15 * sizeof(GLfloat), color_penta, GL_STATIC_DRAW);
            glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 0, 0);
            glEnableVertexAttribArray(1);

            glDrawArrays(GL_TRIANGLE_FAN, 0, 5);

            glDisableVertexAttribArray(0);
            glDisableVertexAttribArray(1);

            break;
        }
    }

    glutSwapBuffers(); // 화면에 출력하기
}

GLvoid Reshape(int w, int h) //--- 콜백 함수: 다시 그리기 콜백 함수
{
    glViewport(0, 0, w, h);
}

void main(int argc, char** argv) //--- 윈도우 출력하고 콜백함수 설정
{
    //--- 윈도우 생성하기
    glutInit(&argc, argv);
    glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGBA);
    glutInitWindowPosition(0, 0);
    glutInitWindowSize(800, 800);
    glutCreateWindow("실습 12");

    s.vertex_num = 1; shape.push_back(s); s.vertex_num = 1; shape.push_back(s); s.vertex_num = 1; shape.push_back(s);
    s.vertex_num = 2; shape.push_back(s); s.vertex_num = 2; shape.push_back(s); s.vertex_num = 2; shape.push_back(s);
    s.vertex_num = 3; shape.push_back(s); s.vertex_num = 3; shape.push_back(s); s.vertex_num = 3; shape.push_back(s);
    s.vertex_num = 4; shape.push_back(s); s.vertex_num = 4; shape.push_back(s); s.vertex_num = 4; shape.push_back(s);
    s.vertex_num = 5; shape.push_back(s); s.vertex_num = 5; shape.push_back(s); s.vertex_num = 5; shape.push_back(s);

    for (int i = 0; i < shape.size(); ++i) {
        shape[i].left = (rand() % 400 - 200) / 300.0;
        shape[i].top = (rand() % 400 - 200) / 300.0;
        shape[i].right = shape[i].left + 0.3;
        shape[i].bottom = shape[i].top - 0.3;
        shape[i].move = false;
        shape[i].select = false;
    }
                                                      
    //--- GLEW 초기화하기
    glewExperimental = GL_TRUE;
    glewInit();
    make_shaderProgram();
    InitBuffer();
    glutMouseFunc(Mouse);
    glutMotionFunc(Motion);
    glutKeyboardFunc(Keyboard);
    glutDisplayFunc(drawScene);
    glutTimerFunc(30, Diagonal_Timer, 1);
    glutReshapeFunc(Reshape);

    glutMainLoop();
}
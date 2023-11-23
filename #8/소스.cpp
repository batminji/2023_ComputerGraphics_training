#define _CRT_SECURE_NO_WARNINGS //--- 프로그램 맨 앞에 선언할 것
#include <stdlib.h>
#include <stdio.h>
#include <iostream>
#include <random>
#include <gl/glew.h>
#include <gl/freeglut.h>
#include <gl/freeglut_ext.h>

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

GLfloat TriShape[4][3][3] = {
    {{-0.5, 0.75,0.0}, {-0.75, 0.25, 0.0}, {-0.25, 0.25, 0.0}},
    {{0.5, 0.75, 0.0}, {0.25, 0.25, 0.0}, {0.75, 0.25, 0.0}},
    {{-0.5, -0.25, 0.0}, {-0.75, -0.75, 0.0}, {-0.25, -0.75, 0.0}},
    {{0.5, -0.25, 0.0}, {0.25, -0.75, 0.0}, {0.75, -0.75, 0.0}}
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

bool plane = true;
bool line = false;

GLvoid Keyboard(unsigned char key, int x, int y)
{
    switch (key) {
    case 'a': // 면으로
        plane = true; line = false;
        break;
    case 'b': // 선으로
        plane = false; line = true;
        break;
    }
    glutPostRedisplay();
}

double mx, my;
float tri_r;

void Mouse(int button, int state, int x, int y)
{
    mx = ((double)x - 400) / 400;
    my = -(((double)y - 400) / 400);
    
    tri_r = rand() % 50;
    tri_r = tri_r / 100.0;

    if (button == GLUT_LEFT_BUTTON && state == GLUT_DOWN) {
        if (mx > -1.0f && mx < 0.0f && my > 0.0f && my < 1.0f) {
            TriShape[0][0][0] = mx; TriShape[0][0][1] = my + tri_r;
            TriShape[0][1][0] = mx - tri_r; TriShape[0][1][1] = my - tri_r;
            TriShape[0][2][0] = mx + tri_r; TriShape[0][2][1] = my - tri_r;
        }
        else if (mx > 0.0f && mx < 1.0f && my > 0.0f && my < 1.0f) {
            TriShape[1][0][0] = mx; TriShape[1][0][1] = my + tri_r;
            TriShape[1][1][0] = mx - tri_r; TriShape[1][1][1] = my - tri_r;
            TriShape[1][2][0] = mx + tri_r; TriShape[1][2][1] = my - tri_r;
        }
        else if (mx > -1.0f && mx < 0.0f && my < 0.0f && my > -1.0f) {
            TriShape[2][0][0] = mx; TriShape[2][0][1] = my + tri_r;
            TriShape[2][1][0] = mx - tri_r; TriShape[2][1][1] = my - tri_r;
            TriShape[2][2][0] = mx + tri_r; TriShape[2][2][1] = my - tri_r;
        }
        else if (mx > 0.0f && mx < 1.0f && my < 0.0f && my > -1.0f) {
            TriShape[3][0][0] = mx; TriShape[3][0][1] = my + tri_r;
            TriShape[3][1][0] = mx - tri_r; TriShape[3][1][1] = my - tri_r;
            TriShape[3][2][0] = mx + tri_r; TriShape[3][2][1] = my - tri_r;
        }
    }
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

    // 사등분 그리기
    glRectf(-1.0f, 0.002f, 1.0f, -0.002f);
    glRectf(-0.002f, 1.0f, 0.002f, -1.0f);

    for (int i = 0; i < 4; ++i) {
        glBindBuffer(GL_ARRAY_BUFFER, vbo[0]);
        glBufferData(GL_ARRAY_BUFFER, 9 * sizeof(GLfloat), TriShape[i], GL_STATIC_DRAW);
        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, 0);
        glEnableVertexAttribArray(0);

        glBindBuffer(GL_ARRAY_BUFFER, vbo[1]);
        glBufferData(GL_ARRAY_BUFFER, 9 * sizeof(GLfloat), colors[i], GL_STATIC_DRAW);
        glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 0, 0);
        glEnableVertexAttribArray(1);

        if (plane) {
            glDrawArrays(GL_TRIANGLES, 0, 3);
        }
        else if (line) {
            glLineWidth(5.0);
            glDrawArrays(GL_LINE_LOOP, 0, 3);
        }
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
    glutCreateWindow("실습 8");

    //--- GLEW 초기화하기
    glewExperimental = GL_TRUE;
    glewInit();

    //--- 세이더 읽어와서 세이더 프로그램 만들기
    make_shaderProgram(); //--- 세이더 프로그램 만들기
    InitBuffer();

    glutKeyboardFunc(Keyboard);
    glutMouseFunc(Mouse);
    glutDisplayFunc(drawScene); //--- 출력 콜백 함수
    glutReshapeFunc(Reshape);

    glutMainLoop();
}
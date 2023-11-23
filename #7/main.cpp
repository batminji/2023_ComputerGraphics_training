#define _CRT_SECURE_NO_WARNINGS //--- 프로그램 맨 앞에 선언할 것
#include <stdlib.h>
#include <stdio.h>
#include <iostream>
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

bool tri_produce = false;
int tri_cnt = 0;
GLfloat triShape[10][3][3] = {};

bool dot_produce = false;
int dot_cnt = 0;
GLfloat dotShape[10][3] = {};

bool line_produce = false;
int line_cnt = 0;
GLfloat lineShape[10][2][3] = {};

bool rect_produce = false;
int rect_cnt = 0;
GLfloat rectShape[10][6][3] = {};

const GLfloat colors[6][3] = {
   { 1.0, 0.0, 0.0 },
   { 0.0, 1.0, 0.0 },
   { 0.0, 0.0, 1.0 },
   { 1.0, 0.0, 0.0 },
   { 0.0, 1.0, 0.0 },
   { 0.0, 0.0, 1.0 }
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
    glGenBuffers(2, vbo); //--- 2개의 VBO를 지정하고 할당하기
}

GLvoid Keyboard(unsigned char key, int x, int y)
{
    switch (key) {
    case 'p':
        dot_produce = true; tri_produce = false; line_produce = false; rect_produce = false;
        break;
    case 'l':
        line_produce = true; dot_produce = false; tri_produce = false; rect_produce = false;
        break;
    case 't':
        tri_produce = true; dot_produce = false; line_produce = false; rect_produce = false;
        break;
    case 'r':
        rect_produce = true; dot_produce = false; line_produce = false; tri_produce = false;
        break;
    case 'w':
        for (int i = 0; i < dot_cnt; ++i) {
            dotShape[i][1] += 0.01;
        }
        for (int i = 0; i < line_cnt; ++i) {
            lineShape[i][0][1] += 0.01;
            lineShape[i][1][1] += 0.01;
        }
        for (int i = 0; i < tri_cnt; ++i) {
            triShape[i][0][1] += 0.01;
            triShape[i][1][1] += 0.01;
            triShape[i][2][1] += 0.01;
        }
        for (int i = 0; i < rect_cnt; ++i) {
            rectShape[i][0][1] += 0.01;
            rectShape[i][1][1] += 0.01;
            rectShape[i][2][1] += 0.01;
            rectShape[i][3][1] += 0.01;
            rectShape[i][4][1] += 0.01;
            rectShape[i][5][1] += 0.01;
        }
        break;
    case 'a':
        for (int i = 0; i < dot_cnt; ++i) {
            dotShape[i][0] -= 0.01;
        }
        for (int i = 0; i < line_cnt; ++i) {
            lineShape[i][0][0] -= 0.01;
            lineShape[i][1][0] -= 0.01;
        }
        for (int i = 0; i < tri_cnt; ++i) {
            triShape[i][0][0] -= 0.01;
            triShape[i][1][0] -= 0.01;
            triShape[i][2][0] -= 0.01;
        }
        for (int i = 0; i < rect_cnt; ++i) {
            rectShape[i][0][0] -= 0.01;
            rectShape[i][1][0] -= 0.01;
            rectShape[i][2][0] -= 0.01;
            rectShape[i][3][0] -= 0.01;
            rectShape[i][4][0] -= 0.01;
            rectShape[i][5][0] -= 0.01;
        }
        break;
    case 's':
        for (int i = 0; i < dot_cnt; ++i) {
            dotShape[i][1] -= 0.01;
        }
        for (int i = 0; i < line_cnt; ++i) {
            lineShape[i][0][1] -= 0.01;
            lineShape[i][1][1] -= 0.01;
        }
        for (int i = 0; i < tri_cnt; ++i) {
            triShape[i][0][1] -= 0.01;
            triShape[i][1][1] -= 0.01;
            triShape[i][2][1] -= 0.01;
        }
        for (int i = 0; i < rect_cnt; ++i) {
            rectShape[i][0][1] -= 0.01;
            rectShape[i][1][1] -= 0.01;
            rectShape[i][2][1] -= 0.01;
            rectShape[i][3][1] -= 0.01;
            rectShape[i][4][1] -= 0.01;
            rectShape[i][5][1] -= 0.01;
        }
        break;
    case 'd':
        for (int i = 0; i < dot_cnt; ++i) {
            dotShape[i][0] += 0.01;
        }
        for (int i = 0; i < line_cnt; ++i) {
            lineShape[i][0][0] += 0.01;
            lineShape[i][1][0] += 0.01;
        }
        for (int i = 0; i < tri_cnt; ++i) {
            triShape[i][0][0] += 0.01;
            triShape[i][1][0] += 0.01;
            triShape[i][2][0] += 0.01;
        }
        for (int i = 0; i < rect_cnt; ++i) {
            rectShape[i][0][0] += 0.01;
            rectShape[i][1][0] += 0.01;
            rectShape[i][2][0] += 0.01;
            rectShape[i][3][0] += 0.01;
            rectShape[i][4][0] += 0.01;
            rectShape[i][5][0] += 0.01;
        }
        break;
    case 'c':
        dot_cnt = 0; tri_cnt = 0; line_cnt = 0; rect_cnt = 0;
        break;
    }
    glutPostRedisplay();
}

double mx, my;
void Mouse(int button, int state, int x, int y)
{
    mx = ((double)x - 400) / 400;
    my = -(((double)y - 400) / 400);

    if (button == GLUT_LEFT_BUTTON && state == GLUT_DOWN) {
        if (tri_produce) {
            if (tri_cnt < 10) {
                triShape[tri_cnt][0][0] = mx + 0.05; triShape[tri_cnt][0][1] = my - 0.05; triShape[tri_cnt][0][2] = 0.0;
                triShape[tri_cnt][1][0] = mx; triShape[tri_cnt][1][1] = my + 0.07; triShape[tri_cnt][1][2] = 0.0;
                triShape[tri_cnt][2][0] = mx - 0.05; triShape[tri_cnt][2][1] = my - 0.05; triShape[tri_cnt][2][2] = 0.0;
                tri_cnt++;
            }
        }
        else if (dot_produce) {
            if (dot_cnt < 10) {
                dotShape[dot_cnt][0] = mx; dotShape[dot_cnt][1] = my; dotShape[dot_cnt][2] = 0.0;
                dot_cnt++;
            }
        }
        else if (line_produce) {
            if (line_cnt < 10) {
                lineShape[line_cnt][0][0] = mx - 0.1; lineShape[line_cnt][0][1] = my - 0.1; lineShape[line_cnt][0][2] = 0.0;
                lineShape[line_cnt][1][0] = mx + 0.1; lineShape[line_cnt][1][1] = my + 0.1; lineShape[line_cnt][1][2] = 0.0;
                line_cnt++;
            }
        }
        else if (rect_produce) {
            if (rect_cnt < 10) {
                rectShape[rect_cnt][0][0] = mx - 0.1; rectShape[rect_cnt][0][1] = my + 0.1; rectShape[rect_cnt][0][2] = 0.0;
                rectShape[rect_cnt][1][0] = mx + 0.1; rectShape[rect_cnt][1][1] = my - 0.1; rectShape[rect_cnt][1][2] = 0.0;
                rectShape[rect_cnt][2][0] = mx + 0.1; rectShape[rect_cnt][2][1] = my + 0.1; rectShape[rect_cnt][2][2] = 0.0;

                rectShape[rect_cnt][3][0] = mx - 0.1; rectShape[rect_cnt][3][1] = my + 0.1; rectShape[rect_cnt][3][2] = 0.0;
                rectShape[rect_cnt][4][0] = mx - 0.1; rectShape[rect_cnt][4][1] = my - 0.1; rectShape[rect_cnt][4][2] = 0.0;
                rectShape[rect_cnt][5][0] = mx + 0.1; rectShape[rect_cnt][5][1] = my - 0.1; rectShape[rect_cnt][5][2] = 0.0;
                rect_cnt++;
            }
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

    glBindBuffer(GL_ARRAY_BUFFER, vbo[1]);
    glBufferData(GL_ARRAY_BUFFER, 9 * sizeof(GLfloat), colors, GL_STATIC_DRAW);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 0, 0);
    glEnableVertexAttribArray(1);

    // 점찍기
    for (int i = 0; i < dot_cnt; ++i) {
        glBindBuffer(GL_ARRAY_BUFFER, vbo[0]);
        glBufferData(GL_ARRAY_BUFFER, 3 * sizeof(GLfloat), dotShape[i], GL_STATIC_DRAW);
        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, 0);
        glEnableVertexAttribArray(0);

        glPointSize(10.0);
        glDrawArrays(GL_POINTS, 0, 1);
    }
    // 선 그리기
    for (int i = 0; i < line_cnt; ++i) {
        glBindBuffer(GL_ARRAY_BUFFER, vbo[0]);
        glBufferData(GL_ARRAY_BUFFER, 6 * sizeof(GLfloat), lineShape[i], GL_STATIC_DRAW);
        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, 0);
        glEnableVertexAttribArray(0);

        glLineWidth(5.0);
        glDrawArrays(GL_LINE_LOOP, 0, 2);
    }

    // 삼각형 그리기
    for (int i = 0; i < tri_cnt; ++i) {
        glBindBuffer(GL_ARRAY_BUFFER, vbo[0]);
        glBufferData(GL_ARRAY_BUFFER, 9 * sizeof(GLfloat), triShape[i], GL_STATIC_DRAW);
        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, 0);
        glEnableVertexAttribArray(0);

        glDrawArrays(GL_TRIANGLES, 0, 3);
    }
    
    // 사각형 그리기
    for (int i = 0; i < rect_cnt; ++i) {
        glBindBuffer(GL_ARRAY_BUFFER, vbo[0]);
        glBufferData(GL_ARRAY_BUFFER, 18 * sizeof(GLfloat), rectShape[i], GL_STATIC_DRAW);
        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, 0);
        glEnableVertexAttribArray(0);

        glBindBuffer(GL_ARRAY_BUFFER, vbo[1]);
        glBufferData(GL_ARRAY_BUFFER, 18 * sizeof(GLfloat), colors, GL_STATIC_DRAW);
        glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 0, 0);
        glEnableVertexAttribArray(1);

        glDrawArrays(GL_TRIANGLES, 0, 6);
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
    glutCreateWindow("실습 7");

    //--- GLEW 초기화하기
    glewExperimental = GL_TRUE;
    glewInit();

    //--- 세이더 읽어와서 세이더 프로그램 만들기
    make_shaderProgram(); //--- 세이더 프로그램 만들기
    InitBuffer();

    glutMouseFunc(Mouse);
    glutKeyboardFunc(Keyboard);
    glutDisplayFunc(drawScene); //--- 출력 콜백 함수
    glutReshapeFunc(Reshape);

    glutMainLoop();
}
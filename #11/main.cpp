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

// 선 -> 삼각형
GLfloat LineToTriFrom[3][3] = {
    {-0.5, 0.5, 0.0},
    {-0.8, 0.2, 0.0},
    {-0.2, 0.8, 0.0}
};
GLfloat LineToTriTo[3][3] = {
    {-0.5, 0.8, 0.0},
    {-0.8, 0.2, 0.0},
    {-0.2, 0.2, 0.0}
};
GLfloat LineToTri[3][3];
GLfloat LineToTri_big[3][3] = {};
// 삼각형 -> 사각형
GLfloat TriToRectFrom[4][3] = {
    {0.5, 0.8, 0.0},
    {0.2, 0.2, 0.0},
    {0.8, 0.2, 0.0},
    {0.5, 0.8, 0.0}
};
GLfloat TriToRectTo[4][3] = {
    {0.3, 0.8, 0.0},
    {0.2, 0.2, 0.0},
    {0.8, 0.2, 0.0},
    {0.7, 0.8, 0.0},
};
GLfloat TriToRect[4][3];
GLfloat TriToRect_big[4][3] = {};
// 사각형 -> 오각형
GLfloat RectToPentaFrom[5][3] = {
    {-0.5, -0.2, 0.0},
    {-0.8, -0.2, 0.0},
    {-0.8, -0.8, 0.0},
    {-0.2, -0.8, 0.0},
    {-0.2, -0.2, 0.0}
};
GLfloat RectToPentaTo[5][3] = {
    {-0.5, -0.2, 0.0},
    {-0.8, -0.4, 0.0},
    {-0.7, -0.8, 0.0},
    {-0.3, -0.8, 0.0},
    {-0.2, -0.4, 0.0}
};
GLfloat RectToPenta[5][3];
GLfloat RectToPenta_big[5][3] = {};
// 오각형 -> 점
GLfloat PentaToDotFrom[5][3] = {
    {0.5, -0.2, 0.0},
    {0.2, -0.4, 0.0},
    {0.3, -0.8, 0.0},
    {0.7, -0.8, 0.0},
    {0.8, -0.4, 0.0}
};
GLfloat PentaToDotTo[5][3] = {
    {0.5, -0.5, 0.0},
    {0.5, -0.5, 0.0},
    {0.5, -0.5, 0.0},
    {0.5, -0.5, 0.0},
    {0.5, -0.5, 0.0}
};
GLfloat PentaToDot[5][3];
GLfloat PentaToDot_big[5][3] = {};

GLfloat rt_board[4][3] = {
    {-1.0, 1.0, 0.0},
    {1.0, 1.0, 0.0},
    {1.0, -1.0, 0.0},
    {-1.0, -1.0, 0.0}
};

//GLfloat colors[5][3][3] = {
//    {{1.0, 0.0, 0.0}, {0.7, 0.0, 0.0}, {0.4, 0.0, 0.0}}, // red
//    {{0.0, 1.0, 0.0}, {0.0, 0.7, 0.0}, {0.0, 0.4, 0.0}}, // green
//    {{0.0, 0.0, 1.0}, {0.0, 0.0, 0.7}, {0.0, 0.0, 0.4}}, // blue
//    {{1.0, 1.0, 0.0}, {0.7, 0.7, 0.0}, {0.4, 0.4, 0.0}}, // yellow
//    {{1.0, 0.0, 1.0}, {0.7, 0.0, 0.7}, {0.4, 0.0, 0.4}}
//};

GLfloat colors_0[4][3] = {
    {1.0, 1.0, 1.0},{1.0, 1.0, 1.0},{1.0, 1.0, 1.0}, {1.0, 1.0, 1.0}
};
GLfloat colors_1[3][3] = {
    {1.0, 0.0, 0.0}, {0.7, 0.0, 0.0}, {0.4, 0.0, 0.0}
};
GLfloat colors_2[4][3] = {
    {0.0, 1.0, 0.0}, {0.0, 0.7, 0.0}, {0.0, 0.4, 0.0}, {0.0, 0.2, 0.0}
};
GLfloat colors_3[5][3] = {
    {0.0, 0.0, 1.0}, {0.0, 0.0, 0.8}, {0.0, 0.0, 0.5},{0.0,0.0,0.4},{0.0, 0.0, 0.2}
};
GLfloat colors_4[5][3] = {
    {1.0, 1.0, 0.0}, {0.7, 0.7, 0.0}, {0.5, 0.5, 0.0},{0.4, 0.4, 0.0}, {0.2, 0.2,0.0}
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

bool l_key = false;
bool line_to_tri = false;
float line_to_tri_t = 0;
void line_to_tri_Timer(int value)
{
    for (int i = 0; i < 3; ++i) {
        LineToTri[i][0] = (1 - line_to_tri_t) * LineToTriFrom[i][0] + line_to_tri_t * LineToTriTo[i][0];
        LineToTri[i][1] = (1 - line_to_tri_t) * LineToTriFrom[i][1] + line_to_tri_t * LineToTriTo[i][1];

        LineToTri_big[i][0] = LineToTri[i][0] + 0.5f;
        LineToTri_big[i][1] = LineToTri[i][1] - 0.5f;
    }
    line_to_tri_t += 0.01f;

    if (line_to_tri_t > 1.0f) {
        for (int i = 0; i < 3; ++i) {
            LineToTri[i][0] = LineToTriTo[i][0];
            LineToTri[i][1] = LineToTriTo[i][1];
        }
        line_to_tri = false;
        l_key = false;
    }

    glutPostRedisplay();
    if (line_to_tri) glutTimerFunc(10, line_to_tri_Timer, 1);
}

bool t_key = false;
bool tri_to_rt = false;
float tri_to_rt_t = 0;
void tri_to_rt_Timer(int value)
{
    for (int i = 0; i < 4; ++i) {
        TriToRect[i][0] = (1 - tri_to_rt_t) * TriToRectFrom[i][0] + tri_to_rt_t * TriToRectTo[i][0];
        TriToRect[i][1] = (1 - tri_to_rt_t) * TriToRectFrom[i][1] + tri_to_rt_t * TriToRectTo[i][1];

        TriToRect_big[i][0] = TriToRect[i][0] -= 0.5f;
        TriToRect_big[i][1] = TriToRect[i][1] -= 0.5f;
    }
    tri_to_rt_t += 0.01f;

    if (tri_to_rt_t > 1.0f) {
        for (int i = 0; i < 4; ++i) {
            TriToRect[i][0] = TriToRectTo[i][0];
            TriToRect[i][1] = TriToRectTo[i][1];
        }
        tri_to_rt = false;
        t_key = false;
    }

    glutPostRedisplay();
    if (tri_to_rt) glutTimerFunc(10, tri_to_rt_Timer, 1);
}

bool r_key = false;
bool rt_to_penta = false;
float rt_to_penta_t = 0;
void rt_to_penta_Timer(int value)
{
    for (int i = 0; i < 5; ++i) {
        RectToPenta[i][0] = (1 - rt_to_penta_t) * RectToPentaFrom[i][0] + rt_to_penta_t * RectToPentaTo[i][0];
        RectToPenta[i][1] = (1 - rt_to_penta_t) * RectToPentaFrom[i][1] + rt_to_penta_t * RectToPentaTo[i][1];

        RectToPenta_big[i][0] = RectToPenta[i][0] + 0.5f;
        RectToPenta_big[i][1] = RectToPenta[i][1] + 0.5f;
    }
    rt_to_penta_t += 0.01f;

    if (rt_to_penta_t > 1.0f) {
        for (int i = 0; i < 5; ++i) {
            RectToPenta[i][0] = RectToPentaTo[i][0];
            RectToPenta[i][1] = RectToPentaTo[i][1];
        }
        rt_to_penta = false;
        r_key = false;
    }

    glutPostRedisplay();
    if (rt_to_penta) glutTimerFunc(10, rt_to_penta_Timer, 1);
}

bool p_key = false;
bool penta_to_dot = false;
float penta_to_dot_t = 0;
void penta_to_dot_Timer(int value)
{
    for (int i = 0; i < 5; ++i) {
        PentaToDot[i][0] = (1 - penta_to_dot_t) * PentaToDotFrom[i][0] + penta_to_dot_t * PentaToDotTo[i][0];
        PentaToDot[i][1] = (1 - penta_to_dot_t) * PentaToDotFrom[i][1] + penta_to_dot_t * PentaToDotTo[i][1];

        PentaToDot_big[i][0] = PentaToDot[i][0] - 0.5f;
        PentaToDot_big[i][1] = PentaToDot[i][1] + 0.5f;
    }
    penta_to_dot_t += 0.01f;

    if (penta_to_dot_t > 1.0f) {
        for (int i = 0; i < 5; ++i) {
            PentaToDot[i][0] = PentaToDotTo[i][0];
            PentaToDot[i][1] = PentaToDotTo[i][1];
        }
        penta_to_dot = false;
        p_key = false;
    }

    glutPostRedisplay();
    if (penta_to_dot) glutTimerFunc(10, penta_to_dot_Timer, 1);
}

GLfloat temp;
GLvoid Keyboard(unsigned char key, int x, int y)
{
    switch (key) {
    case 'l': // 선 -> 삼각형
        if (line_to_tri)line_to_tri = false;
        else {
            line_to_tri = true;
            l_key = true;
            glutTimerFunc(10, line_to_tri_Timer, 1);
        }
        break;
    case 't': // 삼각형 -> 사각형
        if (tri_to_rt)tri_to_rt = false;
        else {
            tri_to_rt = true;
            t_key = true;
            glutTimerFunc(10, tri_to_rt_Timer, 1);
        }
        break;
    case 'r': // 사각형 -> 오각형
        if (rt_to_penta)rt_to_penta = false;
        else {
            rt_to_penta = true;
            r_key = true;
            glutTimerFunc(10, rt_to_penta_Timer, 1);
        }
        break;
    case 'p': // 오각형 -> 점
        if (penta_to_dot)penta_to_dot = false;
        else {
            penta_to_dot = true;
            p_key = true;
            glutTimerFunc(10, penta_to_dot_Timer, 1);
        }
        break;
    case 'a': // 네 개의 도형이 다시 모두 그려진다.
        line_to_tri_t = 0; tri_to_rt_t = 0; rt_to_penta_t = 0; penta_to_dot = 0;
        for (int i = 0; i < 3; ++i)
            for (int j = 0; j < 3; ++j)
                LineToTri[i][j] = LineToTriFrom[i][j];
        for (int i = 0; i < 4; ++i)
            for (int j = 0; j < 3; ++j)
                TriToRect[i][j] = TriToRectFrom[i][j];
        for (int i = 0; i < 5; ++i)
            for (int j = 0; j < 3; ++j) {
                RectToPenta[i][j] = RectToPentaFrom[i][j];
                PentaToDot[i][j] = PentaToDotFrom[i][j];
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
    // 사등분 그리기
    glRectf(-1.0f, 0.002f, 1.0f, -0.002f);
    glRectf(-0.002f, 1.0f, 0.002f, -1.0f);

    // 선 -> 삼각형
    glBindBuffer(GL_ARRAY_BUFFER, vbo[0]);
    glBufferData(GL_ARRAY_BUFFER, 9 * sizeof(GLfloat), LineToTri, GL_STATIC_DRAW);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, 0);
    glEnableVertexAttribArray(0);
    glBindBuffer(GL_ARRAY_BUFFER, vbo[1]);
    glBufferData(GL_ARRAY_BUFFER, 9 * sizeof(GLfloat), colors_1, GL_STATIC_DRAW);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 0, 0);
    glEnableVertexAttribArray(1);
    glLineWidth(2.0);
    glDrawArrays(GL_LINE_LOOP, 0, 3);
    glDrawArrays(GL_TRIANGLES, 0, 3);
    glDisableVertexAttribArray(0);
    glDisableVertexAttribArray(1);

    // 삼각형 -> 사각형
    glBindBuffer(GL_ARRAY_BUFFER, vbo[0]);
    glBufferData(GL_ARRAY_BUFFER, 12 * sizeof(GLfloat), TriToRect, GL_STATIC_DRAW);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, 0);
    glEnableVertexAttribArray(0);
    glBindBuffer(GL_ARRAY_BUFFER, vbo[1]);
    glBufferData(GL_ARRAY_BUFFER, 12 * sizeof(GLfloat), colors_2, GL_STATIC_DRAW);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 0, 0);
    glEnableVertexAttribArray(1);
    glDrawArrays(GL_TRIANGLE_FAN, 0, 4);
    glDisableVertexAttribArray(0);
    glDisableVertexAttribArray(1);

    // 사각형 -> 오각형
    glBindBuffer(GL_ARRAY_BUFFER, vbo[0]);
    glBufferData(GL_ARRAY_BUFFER, 15 * sizeof(GLfloat), RectToPenta, GL_STATIC_DRAW);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, 0);
    glEnableVertexAttribArray(0);
    glBindBuffer(GL_ARRAY_BUFFER, vbo[1]);
    glBufferData(GL_ARRAY_BUFFER, 15 * sizeof(GLfloat), colors_3, GL_STATIC_DRAW);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 0, 0);
    glEnableVertexAttribArray(1);
    glDrawArrays(GL_TRIANGLE_FAN, 0, 5);
    glDisableVertexAttribArray(0);
    glDisableVertexAttribArray(1);

    // 오각형 -> 점
    glBindBuffer(GL_ARRAY_BUFFER, vbo[0]);
    glBufferData(GL_ARRAY_BUFFER, 15 * sizeof(GLfloat), PentaToDot, GL_STATIC_DRAW);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, 0);
    glEnableVertexAttribArray(0);
    glBindBuffer(GL_ARRAY_BUFFER, vbo[1]);
    glBufferData(GL_ARRAY_BUFFER, 15 * sizeof(GLfloat), colors_4, GL_STATIC_DRAW);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 0, 0);
    glEnableVertexAttribArray(1);    
    glPointSize(10.0);
    glDrawArrays(GL_POINTS, 0, 1);
    glDrawArrays(GL_TRIANGLE_FAN, 0, 5);
    glDisableVertexAttribArray(0);
    glDisableVertexAttribArray(1);

    // 가운데에 크게 그리기
    if (l_key || t_key || r_key || p_key) {
        glBindBuffer(GL_ARRAY_BUFFER, vbo[0]);
        glBufferData(GL_ARRAY_BUFFER, 12 * sizeof(GLfloat), rt_board, GL_STATIC_DRAW);
        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, 0);
        glEnableVertexAttribArray(0);

        glBindBuffer(GL_ARRAY_BUFFER, vbo[1]);
        glBufferData(GL_ARRAY_BUFFER, 15 * sizeof(GLfloat), colors_0, GL_STATIC_DRAW);
        glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 0, 0);
        glEnableVertexAttribArray(1);
        glDrawArrays(GL_TRIANGLE_FAN, 0, 4);
    }
    if (l_key) {
        glBindBuffer(GL_ARRAY_BUFFER, vbo[0]);
        glBufferData(GL_ARRAY_BUFFER, 9 * sizeof(GLfloat), LineToTri_big, GL_STATIC_DRAW);
        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, 0);
        glEnableVertexAttribArray(0);
        glBindBuffer(GL_ARRAY_BUFFER, vbo[1]);
        glBufferData(GL_ARRAY_BUFFER, 9 * sizeof(GLfloat), colors_1, GL_STATIC_DRAW);
        glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 0, 0);
        glEnableVertexAttribArray(1);
        glLineWidth(2.0);
        glDrawArrays(GL_LINE_LOOP, 0, 3);
        glDrawArrays(GL_TRIANGLES, 0, 3);
        glDisableVertexAttribArray(0);
        glDisableVertexAttribArray(1);
    }
    else if (t_key) {
        glBindBuffer(GL_ARRAY_BUFFER, vbo[0]);
        glBufferData(GL_ARRAY_BUFFER, 12 * sizeof(GLfloat), TriToRect_big, GL_STATIC_DRAW);
        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, 0);
        glEnableVertexAttribArray(0);
        glBindBuffer(GL_ARRAY_BUFFER, vbo[1]);
        glBufferData(GL_ARRAY_BUFFER, 12 * sizeof(GLfloat), colors_2, GL_STATIC_DRAW);
        glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 0, 0);
        glEnableVertexAttribArray(1);
        glDrawArrays(GL_TRIANGLE_FAN, 0, 4);
        glDisableVertexAttribArray(0);
        glDisableVertexAttribArray(1);
    }
    else if (r_key) {
        glBindBuffer(GL_ARRAY_BUFFER, vbo[0]);
        glBufferData(GL_ARRAY_BUFFER, 15 * sizeof(GLfloat), RectToPenta_big, GL_STATIC_DRAW);
        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, 0);
        glEnableVertexAttribArray(0);
        glBindBuffer(GL_ARRAY_BUFFER, vbo[1]);
        glBufferData(GL_ARRAY_BUFFER, 15 * sizeof(GLfloat), colors_3, GL_STATIC_DRAW);
        glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 0, 0);
        glEnableVertexAttribArray(1);
        glDrawArrays(GL_TRIANGLE_FAN, 0, 5);
        glDisableVertexAttribArray(0);
        glDisableVertexAttribArray(1);
    }
    else if (p_key) {
        glBindBuffer(GL_ARRAY_BUFFER, vbo[0]);
        glBufferData(GL_ARRAY_BUFFER, 15 * sizeof(GLfloat), PentaToDot_big, GL_STATIC_DRAW);
        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, 0);
        glEnableVertexAttribArray(0);
        glBindBuffer(GL_ARRAY_BUFFER, vbo[1]);
        glBufferData(GL_ARRAY_BUFFER, 15 * sizeof(GLfloat), colors_4, GL_STATIC_DRAW);
        glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 0, 0);
        glEnableVertexAttribArray(1);
        glPointSize(10.0);
        glDrawArrays(GL_POINTS, 0, 1);
        glDrawArrays(GL_TRIANGLE_FAN, 0, 5);
        glDisableVertexAttribArray(0);
        glDisableVertexAttribArray(1);
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
    glutCreateWindow("실습 11");

    for (int i = 0; i < 3; ++i)
        for (int j = 0; j < 3; ++j)
            LineToTri[i][j] = LineToTriFrom[i][j];
    for (int i = 0; i < 4; ++i)
        for (int j = 0; j < 3; ++j)
            TriToRect[i][j] = TriToRectFrom[i][j];
    for (int i = 0; i < 5; ++i)
        for (int j = 0; j < 3; ++j) {
            RectToPenta[i][j] = RectToPentaFrom[i][j];
            PentaToDot[i][j] = PentaToDotFrom[i][j];
        }

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
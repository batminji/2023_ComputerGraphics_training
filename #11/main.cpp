#define _CRT_SECURE_NO_WARNINGS //--- ���α׷� �� �տ� ������ ��
#include <stdlib.h>
#include <stdio.h>
#include <iostream>
#include <gl/glew.h>
#include <gl/freeglut.h>
#include <gl/freeglut_ext.h>

// ���ڿ� �о���� ���� �ڵ�
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

//--- �Ʒ� 5�� �Լ��� ����� ���� �Լ���
void make_vertexShaders();
void make_fragmentShaders();
void make_shaderProgram();

GLvoid drawScene();
GLvoid Reshape(int w, int h);

// �� -> �ﰢ��
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
// �ﰢ�� -> �簢��
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
// �簢�� -> ������
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
// ������ -> ��
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



GLchar* vertexSource, * fragmentSource; //--- �ҽ��ڵ� ���� ����
GLuint vertexShader, fragmentShader; //--- ���̴� ��ü
GLuint shaderProgramID;
GLuint vao, vbo[2];

//--- ���̴� ���α׷� ����� ���̴� ��ü ��ũ�ϱ�
void make_shaderProgram()
{
    make_vertexShaders(); //--- ���ؽ� ���̴� �����
    make_fragmentShaders(); //--- �����׸�Ʈ ���̴� �����

    //-- shader Program
    shaderProgramID = glCreateProgram();

    glAttachShader(shaderProgramID, vertexShader);
    glAttachShader(shaderProgramID, fragmentShader);
    glLinkProgram(shaderProgramID);

    //--- ���̴� �����ϱ�
    glDeleteShader(vertexShader);
    glDeleteShader(fragmentShader);

    //--- Shader Program ����ϱ�
    glUseProgram(shaderProgramID);
}
//--- ���ؽ� ���̴� ��ü �����
void make_vertexShaders()
{
    vertexSource = filetobuf("vertex.glsl");

    //--- ���ؽ� ���̴� ��ü �����
    vertexShader = glCreateShader(GL_VERTEX_SHADER);

    //--- ���̴� �ڵ带 ���̴� ��ü�� �ֱ�
    glShaderSource(vertexShader, 1, (const GLchar**)&vertexSource, 0);

    //--- ���ؽ� ���̴� �������ϱ�
    glCompileShader(vertexShader);

    //--- �������� ����� ���� ���� ���: ���� üũ
    GLint result;
    GLchar errorLog[512];
    glGetShaderiv(vertexShader, GL_COMPILE_STATUS, &result);
    if (!result)
    {
        glGetShaderInfoLog(vertexShader, 512, NULL, errorLog);
        std::cout << "ERROR: vertex shader ������ ����\n" << errorLog << std::endl;
        return;
    }
}
//--- �����׸�Ʈ ���̴� ��ü �����
void make_fragmentShaders()
{
    fragmentSource = filetobuf("fragment.glsl");

    //--- �����׸�Ʈ ���̴� ��ü �����
    fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);

    //--- ���̴� �ڵ带 ���̴� ��ü�� �ֱ�
    glShaderSource(fragmentShader, 1, (const GLchar**)&fragmentSource, 0);

    //--- �����׸�Ʈ ���̴� ������
    glCompileShader(fragmentShader);

    //--- �������� ����� ���� ���� ���: ������ ���� üũ
    GLint result;
    GLchar errorLog[512];
    glGetShaderiv(fragmentShader, GL_COMPILE_STATUS, &result);
    if (!result)
    {
        glGetShaderInfoLog(fragmentShader, 512, NULL, errorLog);
        std::cout << "ERROR: fragment shader ������ ����\n" << errorLog << std::endl;
        return;
    }
}

void InitBuffer()
{
    glGenVertexArrays(1, &vao); //--- VAO �� �����ϰ� �Ҵ��ϱ�
    glBindVertexArray(vao); //--- VAO�� ���ε��ϱ�
    glGenBuffers(2, vbo); //--- 3���� VBO�� �����ϰ� �Ҵ��ϱ�

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
    case 'l': // �� -> �ﰢ��
        if (line_to_tri)line_to_tri = false;
        else {
            line_to_tri = true;
            l_key = true;
            glutTimerFunc(10, line_to_tri_Timer, 1);
        }
        break;
    case 't': // �ﰢ�� -> �簢��
        if (tri_to_rt)tri_to_rt = false;
        else {
            tri_to_rt = true;
            t_key = true;
            glutTimerFunc(10, tri_to_rt_Timer, 1);
        }
        break;
    case 'r': // �簢�� -> ������
        if (rt_to_penta)rt_to_penta = false;
        else {
            rt_to_penta = true;
            r_key = true;
            glutTimerFunc(10, rt_to_penta_Timer, 1);
        }
        break;
    case 'p': // ������ -> ��
        if (penta_to_dot)penta_to_dot = false;
        else {
            penta_to_dot = true;
            p_key = true;
            glutTimerFunc(10, penta_to_dot_Timer, 1);
        }
        break;
    case 'a': // �� ���� ������ �ٽ� ��� �׷�����.
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

GLvoid drawScene() //--- �ݹ� �Լ�: �׸��� �ݹ� �Լ�
{
    GLfloat rColor, gColor, bColor;
    rColor = gColor = bColor = 1.0f;
    glClearColor(rColor, gColor, bColor, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT);

    //--- ������ ���������ο� ���̴� �ҷ�����
    glUseProgram(shaderProgramID);
    //--- ����� VAO �ҷ�����
    glBindVertexArray(vao);
    // ���� �׸���
    glRectf(-1.0f, 0.002f, 1.0f, -0.002f);
    glRectf(-0.002f, 1.0f, 0.002f, -1.0f);

    // �� -> �ﰢ��
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

    // �ﰢ�� -> �簢��
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

    // �簢�� -> ������
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

    // ������ -> ��
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

    // ����� ũ�� �׸���
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

    glutSwapBuffers(); // ȭ�鿡 ����ϱ�
}

GLvoid Reshape(int w, int h) //--- �ݹ� �Լ�: �ٽ� �׸��� �ݹ� �Լ�
{
    glViewport(0, 0, w, h);
}

void main(int argc, char** argv) //--- ������ ����ϰ� �ݹ��Լ� ����
{
    //--- ������ �����ϱ�
    glutInit(&argc, argv);
    glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGBA);
    glutInitWindowPosition(0, 0);
    glutInitWindowSize(800, 800);
    glutCreateWindow("�ǽ� 11");

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

    //--- GLEW �ʱ�ȭ�ϱ�
    glewExperimental = GL_TRUE;
    glewInit();
    make_shaderProgram();
    InitBuffer();
    glutKeyboardFunc(Keyboard);
    glutDisplayFunc(drawScene);
    glutReshapeFunc(Reshape);

    glutMainLoop();
}
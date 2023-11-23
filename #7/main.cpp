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
    glGenBuffers(2, vbo); //--- 2���� VBO�� �����ϰ� �Ҵ��ϱ�
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

    glBindBuffer(GL_ARRAY_BUFFER, vbo[1]);
    glBufferData(GL_ARRAY_BUFFER, 9 * sizeof(GLfloat), colors, GL_STATIC_DRAW);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 0, 0);
    glEnableVertexAttribArray(1);

    // �����
    for (int i = 0; i < dot_cnt; ++i) {
        glBindBuffer(GL_ARRAY_BUFFER, vbo[0]);
        glBufferData(GL_ARRAY_BUFFER, 3 * sizeof(GLfloat), dotShape[i], GL_STATIC_DRAW);
        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, 0);
        glEnableVertexAttribArray(0);

        glPointSize(10.0);
        glDrawArrays(GL_POINTS, 0, 1);
    }
    // �� �׸���
    for (int i = 0; i < line_cnt; ++i) {
        glBindBuffer(GL_ARRAY_BUFFER, vbo[0]);
        glBufferData(GL_ARRAY_BUFFER, 6 * sizeof(GLfloat), lineShape[i], GL_STATIC_DRAW);
        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, 0);
        glEnableVertexAttribArray(0);

        glLineWidth(5.0);
        glDrawArrays(GL_LINE_LOOP, 0, 2);
    }

    // �ﰢ�� �׸���
    for (int i = 0; i < tri_cnt; ++i) {
        glBindBuffer(GL_ARRAY_BUFFER, vbo[0]);
        glBufferData(GL_ARRAY_BUFFER, 9 * sizeof(GLfloat), triShape[i], GL_STATIC_DRAW);
        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, 0);
        glEnableVertexAttribArray(0);

        glDrawArrays(GL_TRIANGLES, 0, 3);
    }
    
    // �簢�� �׸���
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

    glutSwapBuffers(); // ȭ�鿡 ����ϱ�
}

GLvoid Reshape(int w, int h) //--- �ݹ� �Լ�: �ٽ� �׸��� �ݹ� �Լ�
{
    glViewport(0, 0, w, h);
}

//--- ���� �Լ�
void main(int argc, char** argv) //--- ������ ����ϰ� �ݹ��Լ� ����
{
    //--- ������ �����ϱ�
    glutInit(&argc, argv);
    glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGBA);
    glutInitWindowPosition(0, 0);
    glutInitWindowSize(800, 800);
    glutCreateWindow("�ǽ� 7");

    //--- GLEW �ʱ�ȭ�ϱ�
    glewExperimental = GL_TRUE;
    glewInit();

    //--- ���̴� �о�ͼ� ���̴� ���α׷� �����
    make_shaderProgram(); //--- ���̴� ���α׷� �����
    InitBuffer();

    glutMouseFunc(Mouse);
    glutKeyboardFunc(Keyboard);
    glutDisplayFunc(drawScene); //--- ��� �ݹ� �Լ�
    glutReshapeFunc(Reshape);

    glutMainLoop();
}
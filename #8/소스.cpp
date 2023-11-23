#define _CRT_SECURE_NO_WARNINGS //--- ���α׷� �� �տ� ������ ��
#include <stdlib.h>
#include <stdio.h>
#include <iostream>
#include <random>
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

bool plane = true;
bool line = false;

GLvoid Keyboard(unsigned char key, int x, int y)
{
    switch (key) {
    case 'a': // ������
        plane = true; line = false;
        break;
    case 'b': // ������
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
    glutCreateWindow("�ǽ� 8");

    //--- GLEW �ʱ�ȭ�ϱ�
    glewExperimental = GL_TRUE;
    glewInit();

    //--- ���̴� �о�ͼ� ���̴� ���α׷� �����
    make_shaderProgram(); //--- ���̴� ���α׷� �����
    InitBuffer();

    glutKeyboardFunc(Keyboard);
    glutMouseFunc(Mouse);
    glutDisplayFunc(drawScene); //--- ��� �ݹ� �Լ�
    glutReshapeFunc(Reshape);

    glutMainLoop();
}
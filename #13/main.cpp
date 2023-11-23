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

//GLfloat colors[5][3][3] = {
//    {{1.0, 0.0, 0.0}, {0.7, 0.0, 0.0}, {0.4, 0.0, 0.0}}, // red
//    {{0.0, 1.0, 0.0}, {0.0, 0.7, 0.0}, {0.0, 0.4, 0.0}}, // green
//    {{0.0, 0.0, 1.0}, {0.0, 0.0, 0.7}, {0.0, 0.0, 0.4}}, // blue
//    {{1.0, 1.0, 0.0}, {0.7, 0.7, 0.0}, {0.4, 0.4, 0.0}}, // yellow
//    {{1.0, 0.0, 1.0}, {0.7, 0.0, 0.7}, {0.4, 0.0, 0.4}}
//};

struct RectVertex {
    float x;
    float y;
};
RectVertex rect_vertex[4] = {
    {-0.5, 0.5},{0.5, 0.5},{0.5, -0.5},{-0.5, -0.5}
};

GLfloat RT [4][3] = {
    {rect_vertex[0].x, rect_vertex[0].y,0.0f},
    {rect_vertex[1].x, rect_vertex[1].y,0.0f},
    {rect_vertex[2].x, rect_vertex[2].y,0.0f},
    {rect_vertex[3].x, rect_vertex[3].y,0.0f}
};

RectVertex default_d[4] = {};

GLfloat color[4][3] = {
    {1.0, 0.0, 0.0},
    {0.0, 1.0, 0.0},
    {0.0, 0.0, 1.0},
    {1.0, 0.0, 0.0}
};

bool is_click(RectVertex v, double mx, double my) {
    double left, right, top, bottom;
    left = v.x - 0.1; right = v.x + 0.1;
    top = v.y + 0.1; bottom = v.y - 0.1;
    if (mx >= left && mx <= right && my >= bottom && my <= top) {
        return true;
    }
    else return false;
}

bool is_select(double mx, double my) {
    double left, right, top, bottom;
    if (RT[0][0] <= RT[3][0])left = RT[3][0];
    else left = RT[0][0];
    if (RT[1][0] <= RT[2][0])right = RT[1][0];
    else right = RT[2][0];
    if (RT[0][1] <= RT[1][1])top = RT[0][1];
    else top = RT[1][1];
    if (RT[2][1] <= RT[3][1])bottom = RT[3][1];
    else bottom = RT[2][1];

    if (mx >= left && mx <= right && my >= bottom && my <= top) {
        return true;
    }
    else return false;
}

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

bool select_vertex = false;
bool select_frag = false;
void Mouse(int button, int state, int x, int y)
{
    double mx = ((double)x - 400) / 400;
    double my = -(((double)y - 300) / 300);

    if (button == GLUT_LEFT_BUTTON && state == GLUT_DOWN) {
        for (int i = 0; i < 4; ++i) {
            if (is_click(rect_vertex[i], mx, my)) {
                rect_vertex[i].x = mx;
                rect_vertex[i].y = my;
                RT[i][0] = rect_vertex[i].x;
                RT[i][1] = rect_vertex[i].y;
                select_vertex = true;
            }
        }
        if (select_vertex == false && is_select(mx, my)) {
            select_frag = true;
            default_d[0].x = mx - rect_vertex[0].x; default_d[0].y = rect_vertex[0].y - my;
            default_d[1].x = rect_vertex[1].x - mx; default_d[1].y = rect_vertex[1].y - my;
            default_d[2].x = rect_vertex[2].x - mx; default_d[2].y = my - rect_vertex[2].y;
            default_d[3].x = mx - rect_vertex[3].x; default_d[3].y = my - rect_vertex[3].y;
        }
    }
    if (button == GLUT_LEFT_BUTTON && state == GLUT_UP) {
        select_vertex = false;
        select_frag = false;
    }

    glutPostRedisplay();
}

void Motion(int x, int y)
{
    double mx = ((double)x - 400) / 400;
    double my = -(((double)y - 300) / 300);

    if (select_vertex) {
        for (int i = 0; i < 4; ++i) {
            if (is_click(rect_vertex[i], mx, my)) {
                rect_vertex[i].x = mx;
                rect_vertex[i].y = my;
                RT[i][0] = rect_vertex[i].x;
                RT[i][1] = rect_vertex[i].y;
                // ��и��� �� �Ѿ�� ����
                switch (i) {
                case 0:
                    if (mx <= 0.0);
                    else mx = 0.0;
                    if (my >= 0.0);
                    else my = 0.0;
                    break;
                case 1:
                    if (mx >= 0.0);
                    else mx = 0.0;
                    if (my >= 0.0);
                    else my = 0.0;
                    break;
                case 2:
                    if (mx <= 0.0);
                    else mx = 0.0;
                    if (my <= 0.0);
                    else my = 0.0;
                    break;
                case 3:
                    if (mx >= 0.0);
                    else mx = 0.0;
                    if (my <= 0.0);
                    else my = 0.0;
                    break;
                }
            }
        }
    }
    if (select_frag) {
        rect_vertex[0].x = mx - default_d[0].x; rect_vertex[0].y = my + default_d[0].y;
        rect_vertex[1].x = mx + default_d[1].x; rect_vertex[1].y = my + default_d[1].y;
        rect_vertex[2].x = mx + default_d[2].x; rect_vertex[2].y = my - default_d[2].y;
        rect_vertex[3].x = mx - default_d[3].x; rect_vertex[3].y = my - default_d[3].y;

        for (int i = 0; i < 4; ++i) {
            RT[i][0] = rect_vertex[i].x;
            RT[i][1] = rect_vertex[i].y;
        }
    }
    glutPostRedisplay();
}

GLvoid Keyboard(unsigned char key, int x, int y)
{
    switch (key) {

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

    glBindBuffer(GL_ARRAY_BUFFER, vbo[0]);
    glBufferData(GL_ARRAY_BUFFER, 12 * sizeof(GLfloat), RT, GL_STATIC_DRAW);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, 0);
    glEnableVertexAttribArray(0);
    glBindBuffer(GL_ARRAY_BUFFER, vbo[1]);
    glBufferData(GL_ARRAY_BUFFER, 12 * sizeof(GLfloat), color, GL_STATIC_DRAW);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 0, 0);
    glEnableVertexAttribArray(1);
    glLineWidth(10.0);
    glDrawArrays(GL_LINE_LOOP, 0, 4);

    glDisableVertexAttribArray(0);
    glDisableVertexAttribArray(1);

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
    glutInitWindowSize(800, 600);
    glutCreateWindow("�ǽ� 13");



    //--- GLEW �ʱ�ȭ�ϱ�
    glewExperimental = GL_TRUE;
    glewInit();
    make_shaderProgram();
    InitBuffer();
    glutKeyboardFunc(Keyboard);
    glutMouseFunc(Mouse);
    glutMotionFunc(Motion);
    glutDisplayFunc(drawScene);
    glutReshapeFunc(Reshape);

    glutMainLoop();
}
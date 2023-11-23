#define _CRT_SECURE_NO_WARNINGS //--- ���α׷� �� �տ� ������ ��
#include <stdlib.h>
#include <stdio.h>
#include <iostream>
#include <random>
#include <vector>
#include <math.h>
#include <gl/glew.h>
#include <gl/freeglut.h>
#include <gl/freeglut_ext.h>

#define PI 3.141592

using namespace std;

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

bool dot_draw = true;
bool line_draw = false;

GLfloat rColor, gColor, bColor;

struct Dot {
    float x;
    float y;
};
struct Circle {
    vector<Dot>dot;
    float circle_x, circle_y;
    float r;
    float degree;
    bool first, second;
};
Dot new_dot;
Circle circle[5];
int circle_cnt = 0;
Dot mouse_new_dot;
Circle mouse_circle[5];
int mouse_circle_cnt = 0;

GLfloat colors[5][3][3] = {
    {{1.0, 0.0, 0.0}, {0.7, 0.0, 0.0}, {0.4, 0.0, 0.0}}, // red
    {{0.0, 1.0, 0.0}, {0.0, 0.7, 0.0}, {0.0, 0.4, 0.0}}, // green
    {{0.0, 0.0, 1.0}, {0.0, 0.0, 0.7}, {0.0, 0.0, 0.4}}, // blue
    {{1.0, 1.0, 0.0}, {0.7, 0.7, 0.0}, {0.4, 0.4, 0.0}}, // yellow
    {{1.0, 0.0, 1.0}, {0.7, 0.0, 0.7}, {0.4, 0.0, 0.4}}
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

bool Timer = false;
void TimerFunc(int value)
{
    double radian = 0;
    for (int i = 0; i < circle_cnt; ++i) {
        if (circle[i].first) {
            radian = circle[i].degree * PI / 100;
            new_dot.x = circle[i].circle_x + circle[i].r * cos(radian);
            new_dot.y = circle[i].circle_y + circle[i].r * sin(radian);
            circle[i].dot.push_back(new_dot);
            circle[i].r += 0.001;
            circle[i].degree += 4;
            if (circle[i].degree >= 800) {
                circle[i].first = false; circle[i].second = true;
                circle[i].degree += 100;
                circle[i].circle_x += 0.4;
            }
        }
        if (circle[i].second) {
            radian = circle[i].degree * PI / 100;
            new_dot.x = circle[i].circle_x + circle[i].r * cos(radian);
            new_dot.y = circle[i].circle_y + circle[i].r * sin(radian);
            circle[i].dot.push_back(new_dot);
            circle[i].r -= 0.001;
            circle[i].degree -= 4;
            if (circle[i].r <= 0) {
                circle[i].second = false;
            }
        }
    }
    glutPostRedisplay();
    if (Timer) glutTimerFunc(30, TimerFunc, 1);
}

bool mouse_Timer = false;
void mouse_TimerFunc(int value)
{
    double radian = 0;
    for (int i = 0; i < mouse_circle_cnt; ++i) {
        if (mouse_circle[i].first) {
            radian = mouse_circle[i].degree * PI / 100;
            mouse_new_dot.x = mouse_circle[i].circle_x + mouse_circle[i].r * cos(radian);
            mouse_new_dot.y = mouse_circle[i].circle_y + mouse_circle[i].r * sin(radian);
            mouse_circle[i].dot.push_back(mouse_new_dot);
            mouse_circle[i].r += 0.001;
            mouse_circle[i].degree += 4;
            if (mouse_circle[i].degree >= 800) {
                mouse_circle[i].first = false; mouse_circle[i].second = true;
                mouse_circle[i].degree += 100;
                mouse_circle[i].circle_x += 0.4;
            }
        }
        if (mouse_circle[i].second) {
            radian = mouse_circle[i].degree * PI / 100;
            mouse_new_dot.x = mouse_circle[i].circle_x + mouse_circle[i].r * cos(radian);
            mouse_new_dot.y = mouse_circle[i].circle_y + mouse_circle[i].r * sin(radian);
            mouse_circle[i].dot.push_back(mouse_new_dot);
            mouse_circle[i].r -= 0.001;
            mouse_circle[i].degree -= 4;
            if (mouse_circle[i].r <= 0) {
                mouse_circle[i].second = false;
            }
        }
    }
    glutPostRedisplay();
    if (mouse_Timer) glutTimerFunc(30, mouse_TimerFunc, 1);
}

GLvoid Keyboard(unsigned char key, int x, int y)
{
    switch (key) {
    case 'p': // ������
        dot_draw = true; line_draw = false;
        break;
    case 'l': // ������
        dot_draw = false; line_draw = true;
        break;
    case '1':
        mouse_circle_cnt = 0;
        rColor = rand() % 10 / 10.0; gColor = rand() % 10 / 10.0; bColor = rand() % 10 / 10.0;
        circle_cnt = 1;
        for (int i = 0; i < circle_cnt; ++i) {
            circle[i].dot.clear();
            new_dot.x = (rand() % 800 - 400) / 800.0; new_dot.y = (rand() % 800 - 400) / 800.0;
            circle[i].circle_x = new_dot.x; circle[i].circle_y = new_dot.y;
            circle[i].dot.push_back(new_dot);
            circle[i].r = 0.0f; circle[i].degree = 0.0f;
            circle[i].first = true; circle[i].second = false;
            Timer = true; glutTimerFunc(30, TimerFunc, 1);
        }
        break;
    case '2':
        mouse_circle_cnt = 0;
        rColor = rand() % 10 / 10.0; gColor = rand() % 10 / 10.0; bColor = rand() % 10 / 10.0;
        circle_cnt = 2;
        for (int i = 0; i < circle_cnt; ++i) {
            circle[i].dot.clear();
            new_dot.x = (rand() % 800 - 400) / 800.0; new_dot.y = (rand() % 800 - 400) / 800.0;
            circle[i].circle_x = new_dot.x; circle[i].circle_y = new_dot.y;
            circle[i].dot.push_back(new_dot);
            circle[i].r = 0.0f; circle[i].degree = 0.0f;
            circle[i].first = true; circle[i].second = false;
            Timer = true; glutTimerFunc(30, TimerFunc, 1);
        }
        break;
    case '3':
        mouse_circle_cnt = 0;
        rColor = rand() % 10 / 10.0; gColor = rand() % 10 / 10.0; bColor = rand() % 10 / 10.0;
        circle_cnt = 3;
        for (int i = 0; i < circle_cnt; ++i) {
            circle[i].dot.clear();
            new_dot.x = (rand() % 800 - 400) / 800.0; new_dot.y = (rand() % 800 - 400) / 800.0;
            circle[i].circle_x = new_dot.x; circle[i].circle_y = new_dot.y;
            circle[i].dot.push_back(new_dot);
            circle[i].r = 0.0f; circle[i].degree = 0.0f;
            circle[i].first = true; circle[i].second = false;
            Timer = true; glutTimerFunc(30, TimerFunc, 1);
        }
        break;
    case '4':
        mouse_circle_cnt = 0;
        rColor = rand() % 10 / 10.0; gColor = rand() % 10 / 10.0; bColor = rand() % 10 / 10.0;
        circle_cnt = 4;
        for (int i = 0; i < circle_cnt; ++i) {
            circle[i].dot.clear();
            new_dot.x = (rand() % 800 - 400) / 800.0; new_dot.y = (rand() % 800 - 400) / 800.0;
            circle[i].circle_x = new_dot.x; circle[i].circle_y = new_dot.y;
            circle[i].dot.push_back(new_dot);
            circle[i].r = 0.0f; circle[i].degree = 0.0f;
            circle[i].first = true; circle[i].second = false;
            Timer = true; glutTimerFunc(30, TimerFunc, 1);
        }
        break;
    case '5':
        mouse_circle_cnt = 0;
        rColor = rand() % 10 / 10.0; gColor = rand() % 10 / 10.0; bColor = rand() % 10 / 10.0;
        circle_cnt = 5;
        for (int i = 0; i < circle_cnt; ++i) {
            circle[i].dot.clear();
            new_dot.x = (rand() % 800 - 400) / 800.0; new_dot.y = (rand() % 800 - 400) / 800.0;
            circle[i].circle_x = new_dot.x; circle[i].circle_y = new_dot.y;
            circle[i].dot.push_back(new_dot);
            circle[i].r = 0.0f; circle[i].degree = 0.0f;
            circle[i].first = true; circle[i].second = false;
            Timer = true; glutTimerFunc(30, TimerFunc, 1);
        }
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
        rColor = rand() % 10 / 10.0; gColor = rand() % 10 / 10.0; bColor = rand() % 10 / 10.0;
        if (mouse_circle_cnt < 5) {
            mouse_circle[mouse_circle_cnt].dot.clear();
            mouse_new_dot.x = mx; 
            mouse_new_dot.y = my;
            mouse_circle[mouse_circle_cnt].circle_x = mouse_new_dot.x;
            mouse_circle[mouse_circle_cnt].circle_y = mouse_new_dot.y;
            mouse_circle[mouse_circle_cnt].dot.push_back(mouse_new_dot);
            mouse_circle[mouse_circle_cnt].r = 0.0f; 
            mouse_circle[mouse_circle_cnt].degree = 0.0f;
            mouse_circle[mouse_circle_cnt].first = true; 
            mouse_circle[mouse_circle_cnt].second = false;
            mouse_Timer = true; 
            glutTimerFunc(30, mouse_TimerFunc, 1);
            mouse_circle_cnt++;
        }
    }
}

GLvoid drawScene() //--- �ݹ� �Լ�: �׸��� �ݹ� �Լ�
{
    glClearColor(rColor, gColor, bColor, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT);

    //--- ������ ���������ο� ���̴� �ҷ�����
    glUseProgram(shaderProgramID);
    //--- ����� VAO �ҷ�����
    glBindVertexArray(vao);

    GLfloat d[3];
    GLfloat l[2][3];
    if (dot_draw) {
        for (int i = 0; i < circle_cnt; ++i) {
            for (int j = 0; j < circle[i].dot.size(); ++j) {
                d[0] = circle[i].dot[j].x; d[1] = circle[i].dot[j].y; d[2] = 0.0f;

                glBindBuffer(GL_ARRAY_BUFFER, vbo[0]);
                glBufferData(GL_ARRAY_BUFFER, 3 * sizeof(GLfloat), d, GL_STATIC_DRAW);
                glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, 0);
                glEnableVertexAttribArray(0);

                glBindBuffer(GL_ARRAY_BUFFER, vbo[1]);
                glBufferData(GL_ARRAY_BUFFER, 9 * sizeof(GLfloat), colors[i], GL_STATIC_DRAW);
                glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 0, 0);
                glEnableVertexAttribArray(1);

                glPointSize(4.0);
                glDrawArrays(GL_POINTS, 0, 1);
            }
        }
    }
    if (line_draw) {
        for (int i = 0; i < circle_cnt; ++i) {
            for (int j = 0; j < circle[i].dot.size() - 1; ++j) {
                l[0][0] = circle[i].dot[j].x; l[0][1] = circle[i].dot[j].y; l[0][2] = 0.0f;
                l[1][0] = circle[i].dot[j + 1].x; l[1][1] = circle[i].dot[j + 1].y; l[1][2] = 0.0f;

                glBindBuffer(GL_ARRAY_BUFFER, vbo[0]);
                glBufferData(GL_ARRAY_BUFFER, 6 * sizeof(GLfloat), l, GL_STATIC_DRAW);
                glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, 0);
                glEnableVertexAttribArray(0);

                glBindBuffer(GL_ARRAY_BUFFER, vbo[1]);
                glBufferData(GL_ARRAY_BUFFER, 9 * sizeof(GLfloat), colors[i], GL_STATIC_DRAW);
                glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 0, 0);
                glEnableVertexAttribArray(1);

                glLineWidth(6.0);
                glDrawArrays(GL_LINE_LOOP, 0, 2);
            }
        }
    }

    GLfloat md[3];
    GLfloat ml[2][3];
    if (dot_draw) {
        for (int i = 0; i < mouse_circle_cnt; ++i) {
            for (int j = 0; j < mouse_circle[i].dot.size(); ++j) {
                md[0] = mouse_circle[i].dot[j].x; md[1] = mouse_circle[i].dot[j].y; md[2] = 0.0f;

                glBindBuffer(GL_ARRAY_BUFFER, vbo[0]);
                glBufferData(GL_ARRAY_BUFFER, 3 * sizeof(GLfloat), md, GL_STATIC_DRAW);
                glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, 0);
                glEnableVertexAttribArray(0);

                glBindBuffer(GL_ARRAY_BUFFER, vbo[1]);
                glBufferData(GL_ARRAY_BUFFER, 9 * sizeof(GLfloat), colors[i], GL_STATIC_DRAW);
                glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 0, 0);
                glEnableVertexAttribArray(1);

                glPointSize(4.0);
                glDrawArrays(GL_POINTS, 0, 1);
            }
        }
    }
    if (line_draw) {
        for (int i = 0; i < mouse_circle_cnt; ++i) {
            for (int j = 0; j < mouse_circle[i].dot.size() - 1; ++j) {
                ml[0][0] = mouse_circle[i].dot[j].x; ml[0][1] = mouse_circle[i].dot[j].y; ml[0][2] = 0.0f;
                ml[1][0] = mouse_circle[i].dot[j + 1].x; ml[1][1] = mouse_circle[i].dot[j + 1].y; ml[1][2] = 0.0f;

                glBindBuffer(GL_ARRAY_BUFFER, vbo[0]);
                glBufferData(GL_ARRAY_BUFFER, 6 * sizeof(GLfloat), ml, GL_STATIC_DRAW);
                glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, 0);
                glEnableVertexAttribArray(0);

                glBindBuffer(GL_ARRAY_BUFFER, vbo[1]);
                glBufferData(GL_ARRAY_BUFFER, 9 * sizeof(GLfloat), colors[i], GL_STATIC_DRAW);
                glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 0, 0);
                glEnableVertexAttribArray(1);

                glLineWidth(6.0);
                glDrawArrays(GL_LINE_LOOP, 0, 2);
            }
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
    glutCreateWindow("�ǽ� 10");

    rColor = gColor = bColor = 0.5f;

    //--- GLEW �ʱ�ȭ�ϱ�
    glewExperimental = GL_TRUE;
    glewInit();
    make_shaderProgram();
    InitBuffer();
    glutKeyboardFunc(Keyboard);
    glutMouseFunc(Mouse);
    glutDisplayFunc(drawScene);
    glutReshapeFunc(Reshape);

    glutMainLoop();
}
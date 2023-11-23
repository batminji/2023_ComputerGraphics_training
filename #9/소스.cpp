#define _CRT_SECURE_NO_WARNINGS //--- ���α׷� �� �տ� ������ ��
#include <stdlib.h>
#include <stdio.h>
#include <iostream>
#include <random>
#include <math.h>
#include <gl/glew.h>
#include <gl/freeglut.h>
#include <gl/freeglut_ext.h>

#define PI 3.141592

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

struct RT {
    float left, right, top, bottom;
    int dir_x,dir_y;
    float speed;
    int direction;
    float distance_to_go; // ������ �Ÿ�
    float distance_ed; // �� �Ÿ�
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

GLchar* vertexSource, * fragmentSource; //--- �ҽ��ڵ� ���� ����
GLuint vertexShader, fragmentShader; //--- ���̴� ��ü
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

bool Diagonal = false;
void Diagonal_Timer(int value)
{
    for (int i = 0; i < 4; ++i) {
        // �켱 �簢���� �ű��
        rt[i].left += rt[i].dir_x * rt[i].speed;
        rt[i].right += rt[i].dir_x * rt[i].speed;
        rt[i].top += rt[i].dir_y * rt[i].speed;
        rt[i].bottom += rt[i].dir_y * rt[i].speed;
        // �ﰢ�� ��ǥ�� �ű��
        TriShape[i][0][0] += rt[i].dir_x * rt[i].speed; TriShape[i][0][1] += rt[i].dir_y * rt[i].speed;
        TriShape[i][1][0] += rt[i].dir_x * rt[i].speed; TriShape[i][1][1] += rt[i].dir_y * rt[i].speed;
        TriShape[i][2][0] += rt[i].dir_x * rt[i].speed; TriShape[i][2][1] += rt[i].dir_y * rt[i].speed;
        // �浹 �˻� �Ŀ� �浹 ������ ���� ���� �� �ﰢ�� ������
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
        case 0: // ��� �̵��ϴٰ� �Ʒ���
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
        case 1: // �·� �̵��ϴٰ� �Ʒ���
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
        case 2: // ��� �̵��ϴٰ� ����
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
        case 3: // �·� �̵��ϴٰ� ����
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
        case 0: // �·� �̵�
            rt[i].left -= rt[i].speed; rt[i].right -= rt[i].speed;
            TriShape[i][0][0] -= rt[i].speed; TriShape[i][1][0] -= rt[i].speed; TriShape[i][2][0] -= rt[i].speed;
            rt[i].distance_ed += rt[i].speed;
            if (rt[i].distance_ed >= rt[i].distance_to_go) {
                rt[i].distance_to_go -= 0.1; rt[i].distance_ed = 0;
                rt[i].direction = 1;
            }
            break;
        case 1: // �Ʒ��� �̵�
            rt[i].top -= rt[i].speed; rt[i].bottom -= rt[i].speed;
            TriShape[i][0][1] -= rt[i].speed; TriShape[i][1][1] -= rt[i].speed; TriShape[i][2][1] -= rt[i].speed;
            rt[i].distance_ed += rt[i].speed;
            if (rt[i].distance_ed >= rt[i].distance_to_go) {
                rt[i].distance_to_go -= 0.1; rt[i].distance_ed = 0;
                rt[i].direction = 2;
            }
            break;
        case 2: // ��� �̵�
            rt[i].left += rt[i].speed; rt[i].right += rt[i].speed;
            TriShape[i][0][0] += rt[i].speed; TriShape[i][1][0] += rt[i].speed; TriShape[i][2][0] += rt[i].speed;
            rt[i].distance_ed += rt[i].speed;
            if (rt[i].distance_ed >= rt[i].distance_to_go) {
                rt[i].distance_to_go -= 0.1; rt[i].distance_ed = 0;
                rt[i].direction = 3;
            }
            break;
        case 3: // ���� �̵�
            rt[i].top += rt[i].speed; rt[i].bottom += rt[i].speed;
            TriShape[i][0][1] += rt[i].speed; TriShape[i][1][1] += rt[i].speed; TriShape[i][2][1] += rt[i].speed;
            rt[i].distance_ed += rt[i].speed;
            if (rt[i].distance_ed >= rt[i].distance_to_go) {
                rt[i].distance_to_go -= 0.1; rt[i].distance_ed = 0;
                rt[i].direction = 0;
            }
            break;
        }
        // �߾ӿ� �����ϸ� �ʱ�ȭ
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
    case '1': // �밢������ �����̱�
        if (Diagonal)Diagonal = false;
        else {
            Diagonal = true; Zigzag = false; Rectmove = false; Circlemove = false;
            glutTimerFunc(30, Diagonal_Timer, 1);
        }
        break;
    case '2': // ������׷� �����̱�
        if (Zigzag)Zigzag = false;
        else {
            Zigzag = true; Diagonal = false; Rectmove = false; Circlemove = false;
            glutTimerFunc(30, Zigzag_Timer, 1);
        }
        break;
    case '3': // �簢 �����̷�
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
    case '4': // �� �����̷�
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
    glutCreateWindow("�ǽ� 9");

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
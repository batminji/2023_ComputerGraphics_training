#define _CRT_SECURE_NO_WARNINGS
#pragma comment(lib, "glew32.lib")
#pragma comment(lib, "freeglut.lib")
#include <iostream>
#include <gl/glew.h>
#include <gl/freeglut.h>
#include <gl/freeglut_ext.h>
#include <time.h>
#include <stdlib.h>
#include <glm/glm.hpp>
#include <glm/ext.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <random>

#define WINDOWX 800
#define WINDOWY 800

using namespace std;

random_device rd;
default_random_engine dre(rd());
uniform_real_distribution<float>uid(0, 1);

void make_vertexShaders();
void make_fragmentShaders();
GLuint make_shaderProgram();
GLvoid InitBuffer();
GLchar* filetobuf(const char* file);
void InitShader();

GLuint shaderID;
GLint width, height;

GLuint vertexShader;
GLuint fragmentShader;

GLuint VAO, VBO[2];

class Line {
public:
	GLfloat p[6];
	GLfloat color[6];

	void Bind() {
		glBindBuffer(GL_ARRAY_BUFFER, VBO[0]);
		glBufferData(GL_ARRAY_BUFFER, sizeof(p), p, GL_STATIC_DRAW);
		glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), 0);
		glEnableVertexAttribArray(0);

		glBindBuffer(GL_ARRAY_BUFFER, VBO[1]);
		glBufferData(GL_ARRAY_BUFFER, sizeof(color), color, GL_STATIC_DRAW);
		glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), 0);
		glEnableVertexAttribArray(1);
	}
	void Draw() {
		glBindVertexArray(VAO);
		glDrawArrays(GL_LINE_STRIP, 0, 2);
	}
};

class Plane {
public:
	GLfloat p[9];
	GLfloat color[9];

	void Bind() {
		glBindBuffer(GL_ARRAY_BUFFER, VBO[0]);
		glBufferData(GL_ARRAY_BUFFER, sizeof(p), p, GL_STATIC_DRAW);
		glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), 0);
		glEnableVertexAttribArray(0);

		glBindBuffer(GL_ARRAY_BUFFER, VBO[1]);
		glBufferData(GL_ARRAY_BUFFER, sizeof(color), color, GL_STATIC_DRAW);
		glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), 0);
		glEnableVertexAttribArray(1);
	}
	void Draw() {
		glBindVertexArray(VAO);
		glDrawArrays(GL_TRIANGLES, 0, 3);
	}

	void wDraw() {
		glBindVertexArray(VAO);
		glDrawArrays(GL_LINE_STRIP, 0, 3);
	}
};



float mx, my, px, py;
float cnt;
int sel;
char comm;

int dirx = 0;
int diry = 0;

float movex, movey;

BOOL Time, start = TRUE, TimeLoop;
bool Hbool, Ybool, Tbool, Fbool, F2bool, numbool, num2bool, Obool, O2bool, Cbool;
float rotatey, rotatet, rotatef, pyramid[4], rotatenum;
float BackGround[] = { 1.0, 1.0, 1.0 };
Line xl = {
	1, 0, 0,
	-1, 0, 0,

	0, 0, 0,
	0, 0, 0,
};
Line yl = {
	0, 1, 0,
	0, -1, 0,

	0, 0, 0,
	0, 0, 0,
};
Line zl = {
	0, 0, 1,
	0, 0, -1,

	0, 0, 0,
	0, 0, 0,
};
glm::vec4* vertex;
glm::vec4* face;

Plane* Fvertex[2];

GLUquadricObj* obj;
FILE* FL;
int faceNum = 0;

int Click = 0;
float a = 0;
float b = 0;

float topturn = 0;

float turn1 = 0;
float turn2 = 0;
float turn3 = 0;
float turn4 = 0;

bool move1 = false;
bool move2 = false;
bool move3 = false;
bool move4 = false;

bool ver = false;

bool wire = false;
bool topmove = false;
bool shape = true;

void ReadObj(FILE* objFile);
void keyboard(unsigned char, int, int);
void Mouse(int button, int state, int x, int y);
void Motion(int x, int y);
void TimerFunction(int value);
GLvoid drawScene(GLvoid);
GLvoid Reshape(int w, int h);
void vectoplane(Plane* p);
void planecolorset(Plane* p, int a);


void make_vertexShaders()
{
	GLchar* vertexShaderSource;

	vertexShaderSource = filetobuf("vertex.glsl");

	vertexShader = glCreateShader(GL_VERTEX_SHADER);
	glShaderSource(vertexShader, 1, &vertexShaderSource, NULL);
	glCompileShader(vertexShader);

	GLint result;
	GLchar errorLog[512];
	glGetShaderiv(vertexShader, GL_COMPILE_STATUS, &result);
	if (!result)
	{
		glGetShaderInfoLog(vertexShader, 512, NULL, errorLog);
		cerr << "ERROR: vertex shader 컴파일 실패\n" << errorLog << endl;
		exit(-1);
	}
}
void make_fragmentShaders()
{
	const GLchar* fragmentShaderSource = {
		"#version 330 core\n"
		"out vec4 fColor; \n"
		"in vec3 outColor;\n"
		"void main() \n"
		"{\n"
		"fColor = vec4(outColor, 1.0); \n"
		"} \0"
	};

	fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);
	glShaderSource(fragmentShader, 1, &fragmentShaderSource, NULL);
	glCompileShader(fragmentShader);

	GLint result;
	GLchar errorLog[512];
	glGetShaderiv(fragmentShader, GL_COMPILE_STATUS, &result);
	if (!result)
	{
		glGetShaderInfoLog(fragmentShader, 512, NULL, errorLog);
		cerr << "ERROR: fragment shader 컴파일 실패\n" << errorLog << endl;
		exit(-1);
	}

}
GLuint make_shaderProgram()
{
	GLint result;
	GLchar errorLog[512];
	GLuint ShaderProgramID;
	ShaderProgramID = glCreateProgram(); //--- 세이더 프로그램 만들기
	glAttachShader(ShaderProgramID, vertexShader); //--- 세이더 프로그램에 버텍스 세이더 붙이기
	glAttachShader(ShaderProgramID, fragmentShader); //--- 세이더 프로그램에 프래그먼트 세이더 붙이기
	glLinkProgram(ShaderProgramID); //--- 세이더 프로그램 링크하기

	glDeleteShader(vertexShader); //--- 세이더 객체를 세이더 프로그램에 링크했음으로, 세이더 객체 자체는 삭제 가능
	glDeleteShader(fragmentShader);

	glGetProgramiv(ShaderProgramID, GL_LINK_STATUS, &result); // ---세이더가 잘 연결되었는지 체크하기
	if (!result) {
		glGetProgramInfoLog(ShaderProgramID, 512, NULL, errorLog);
		cerr << "ERROR: shader program 연결 실패\n" << errorLog << endl;
		exit(-1);
	}
	glUseProgram(ShaderProgramID); //--- 만들어진 세이더 프로그램 사용하기
	//--- 여러 개의 세이더프로그램 만들 수 있고, 그 중 한개의 프로그램을 사용하려면
	//--- glUseProgram 함수를 호출하여 사용 할 특정 프로그램을 지정한다.
	//--- 사용하기 직전에 호출할 수 있다.
	return ShaderProgramID;
}

void InitShader()
{
	make_vertexShaders(); //--- 버텍스 세이더 만들기
	make_fragmentShaders(); //--- 프래그먼트 세이더 만들기
	shaderID = make_shaderProgram(); //--- 세이더 프로그램 만들기
}

GLvoid InitBuffer() {
	//--- VAO 객체 생성 및 바인딩
	glGenVertexArrays(1, &VAO);
	//--- vertex data 저장을 위한 VBO 생성 및 바인딩.
	glGenBuffers(2, VBO);
}

void main(int argc, char** argv) //--- 윈도우 출력하고 콜백함수 설정 { //--- 윈도우 생성하기
{
	srand((unsigned int)time(NULL));
	glutInit(&argc, argv); // glut 초기화
	glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGBA | GLUT_DEPTH); // 디스플레이 모드 설정
	glutInitWindowPosition(0, 0); // 윈도우의 위치 지정
	glutInitWindowSize(WINDOWX, WINDOWY); // 윈도우의 크기 지정
	glutCreateWindow("Example16");// 윈도우 생성	(윈도우 이름)
	//--- GLEW 초기화하기
	glewExperimental = GL_TRUE;
	glewInit();

	InitShader();
	InitBuffer();

	glutKeyboardFunc(keyboard);
	glutTimerFunc(10, TimerFunction, 1);
	glutDisplayFunc(drawScene); //--- 출력 콜백 함수
	glutReshapeFunc(Reshape);
	glutMainLoop();
}

GLvoid drawScene() //--- 콜백 함수: 그리기 콜백 함수 
{
	if (start) {
		start = FALSE;
		Time = FALSE; TimeLoop = TRUE;
		sel = 0;

		glEnable(GL_DEPTH_TEST);

		FL = fopen("cube.obj", "rt");
		ReadObj(FL);
		fclose(FL);
		Fvertex[0] = (Plane*)malloc(sizeof(Plane) * faceNum);
		vectoplane(Fvertex[0]);
		planecolorset(Fvertex[0], 0);

		FL = fopen("Pyramid.obj", "rt");
		ReadObj(FL);
		fclose(FL);
		Fvertex[1] = (Plane*)malloc(sizeof(Plane) * faceNum);
		vectoplane(Fvertex[1]);
		planecolorset(Fvertex[1], 1);


	} // 초기화할 데이터
	if (!Hbool)
		glEnable(GL_DEPTH_TEST);
	else
		glDisable(GL_DEPTH_TEST);

	glClearColor(BackGround[0], BackGround[1], BackGround[2], 1.0f);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT); 	//배경

	glUseProgram(shaderID);
	glBindVertexArray(VAO);// 쉐이더 , 버퍼 배열 사용

	glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGB | GLUT_DEPTH);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	glm::mat4 TR = glm::mat4(1.0f);

	for (int i = 0; i < 6; ++i) {
		for (int j = 0; j < 6; ++j) {
			xl.color[j] = 0.0;
			yl.color[j] = 0.0;
			zl.color[j] = 0.0;
		}
	}

	unsigned int modelLocation = glGetUniformLocation(shaderID, "model");
	unsigned int viewLocation = glGetUniformLocation(shaderID, "view");
	unsigned int projLocation = glGetUniformLocation(shaderID, "projection");

	glm::mat4 Vw = glm::mat4(1.0f);

	glm::vec3 cameraPos = glm::vec3(0.0f, 0.0f, 5.0f);
	glm::vec3 cameraTarget = glm::vec3(0.0f, 0.0f, 0.0f);
	glm::vec3 cameraDirection = glm::normalize(cameraPos - cameraTarget);
	glm::vec3 up = glm::vec3(0.0f, 1.0f, 0.0f);
	glm::vec3 cameraRight = glm::normalize(glm::cross(up, cameraDirection));
	glm::vec3 cameraUp = glm::cross(cameraDirection, cameraRight);

	Vw = glm::lookAt(cameraPos, cameraDirection, cameraUp);

	glUniformMatrix4fv(viewLocation, 1, GL_FALSE, &Vw[0][0]);

	glm::mat4 Pj = glm::mat4(1.0f);
	if (!ver) {
		Pj = glm::perspective(glm::radians(45.0f), (float)WINDOWX / (float)WINDOWY, 0.1f, 50.0f);
	}
	else {
		Pj = glm::ortho(-2.0f, 2.0f, -2.0f, 2.0f, 0.1f, 100.0f);
	}
	Pj = glm::translate(Pj, glm::vec3(0.0, 0.0, -2.0));
	glUniformMatrix4fv(projLocation, 1, GL_FALSE, &Pj[0][0]);
	modelLocation = glGetUniformLocation(shaderID, "model");
	glUniformMatrix4fv(modelLocation, 1, GL_FALSE, glm::value_ptr(TR));

	// 그리기 코드
	TR = glm::mat4(1.0f);
	TR = glm::rotate(TR, (float)glm::radians(-20.0), glm::vec3(1.0f, 0.0f, 0.0f));
	TR = glm::rotate(TR, (float)glm::radians(20.0), glm::vec3(0.0f, 1.0f, 0.0f));
	glUniformMatrix4fv(modelLocation, 1, GL_FALSE, glm::value_ptr(TR));
	xl.Bind();
	xl.Draw();

	TR = glm::mat4(1.0f);
	TR = glm::rotate(TR, (float)glm::radians(-20.0), glm::vec3(1.0f, 0.0f, 0.0f));
	TR = glm::rotate(TR, (float)glm::radians(20.0), glm::vec3(0.0f, 1.0f, 0.0f));
	glUniformMatrix4fv(modelLocation, 1, GL_FALSE, glm::value_ptr(TR));
	yl.Bind();
	yl.Draw();

	TR = glm::mat4(1.0f);
	TR = glm::rotate(TR, (float)glm::radians(-20.0), glm::vec3(1.0f, 0.0f, 0.0f));
	TR = glm::rotate(TR, (float)glm::radians(20.0), glm::vec3(0.0f, 1.0f, 0.0f));
	glUniformMatrix4fv(modelLocation, 1, GL_FALSE, glm::value_ptr(TR));
	zl.Bind();
	zl.Draw();
	// x, y, z 라인

	if (!Cbool) {
		for (int i = 0; i < 12; ++i) {
			if (i == 4 || i == 5) {
				TR = glm::mat4(1.0f);
				TR = glm::scale(TR, glm::vec3(0.3f, 0.3f, 0.3f));
				TR = glm::rotate(TR, (float)glm::radians(-20.0), glm::vec3(1.0f, 0.0f, 0.0f));
				TR = glm::rotate(TR, (float)glm::radians(20.0 + rotatey), glm::vec3(0.0f, 1.0f, 0.0f));
				TR = glm::translate(TR, glm::vec3(0.0, 0.5, 0.0));
				TR = glm::rotate(TR, (float)glm::radians(rotatet), glm::vec3(1.0f, 0.0f, 0.0f));
				TR = glm::translate(TR, glm::vec3(0.0, -0.5, 0.0));
				glUniformMatrix4fv(modelLocation, 1, GL_FALSE, glm::value_ptr(TR));
				Fvertex[0][i].Bind();
				Fvertex[0][i].Draw();
			}
			else if (i == 10 || i == 11 || i == 0 || i == 1) {
				TR = glm::mat4(1.0f);
				TR = glm::rotate(TR, (float)glm::radians(-20.0), glm::vec3(1.0f, 0.0f, 0.0f));
				TR = glm::rotate(TR, (float)glm::radians(20.0 + rotatey), glm::vec3(0.0f, 1.0f, 0.0f));
				TR = glm::translate(TR, glm::vec3(0.0, rotatef, 0.0));
				TR = glm::scale(TR, glm::vec3(0.3f, 0.3f, 0.3f));
				glUniformMatrix4fv(modelLocation, 1, GL_FALSE, glm::value_ptr(TR));
				Fvertex[0][i].Bind();
				Fvertex[0][i].Draw();
			}
			else if (i == 6 || i == 7 || i == 2 || i == 3) {
				TR = glm::mat4(1.0f);
				TR = glm::rotate(TR, (float)glm::radians(-20.0), glm::vec3(1.0f, 0.0f, 0.0f));
				TR = glm::rotate(TR, (float)glm::radians(20.0 + rotatey), glm::vec3(0.0f, 1.0f, 0.0f));
				TR = glm::translate(TR, glm::vec3(0.0f, rotatenum, 0.0f));
				TR = glm::scale(TR, glm::vec3(0.3f, 0.3f, 0.3f));
				glUniformMatrix4fv(modelLocation, 1, GL_FALSE, glm::value_ptr(TR));
				Fvertex[0][i].Bind();
				Fvertex[0][i].Draw();
			}
			else {
				TR = glm::mat4(1.0f);
				TR = glm::rotate(TR, (float)glm::radians(-20.0), glm::vec3(1.0f, 0.0f, 0.0f));
				TR = glm::rotate(TR, (float)glm::radians(20.0 + rotatey), glm::vec3(0.0f, 1.0f, 0.0f));
				TR = glm::scale(TR, glm::vec3(0.3f, 0.3f, 0.3f));
				glUniformMatrix4fv(modelLocation, 1, GL_FALSE, glm::value_ptr(TR));
				Fvertex[0][i].Bind();
				Fvertex[0][i].Draw();
			}
		}
	}
	else {
		for (int i = 0; i < 6; ++i) {
			if (i == 2)
			{
				TR = glm::mat4(1.0f);
				TR = glm::scale(TR, glm::vec3(0.3f, 0.3f, 0.3f));
				TR = glm::rotate(TR, (float)glm::radians(-20.0), glm::vec3(1.0f, 0.0f, 0.0f));
				TR = glm::rotate(TR, (float)glm::radians(20.0), glm::vec3(0.0f, 1.0f, 0.0f));
				TR = glm::translate(TR, glm::vec3(-0.5, -0.5, 0.0));
				TR = glm::rotate(TR, (float)glm::radians(pyramid[0]), glm::vec3(0.0f, 0.0f, 1.0f));
				TR = glm::translate(TR, glm::vec3(0.5, 0.5, 0.0));
				glUniformMatrix4fv(modelLocation, 1, GL_FALSE, glm::value_ptr(TR));
				Fvertex[1][i].Bind();
				Fvertex[1][i].Draw();
			}
			else if (i == 3)
			{
				TR = glm::mat4(1.0f);
				TR = glm::scale(TR, glm::vec3(0.3f, 0.3f, 0.3f));
				TR = glm::rotate(TR, (float)glm::radians(-20.0), glm::vec3(1.0f, 0.0f, 0.0f));
				TR = glm::rotate(TR, (float)glm::radians(20.0), glm::vec3(0.0f, 1.0f, 0.0f));
				TR = glm::translate(TR, glm::vec3(0.0, -0.5, -0.5));
				TR = glm::rotate(TR, (float)glm::radians(pyramid[1]), glm::vec3(-1.0f, 0.0f, 0.0f));
				TR = glm::translate(TR, glm::vec3(0.0, 0.5, 0.5));
				glUniformMatrix4fv(modelLocation, 1, GL_FALSE, glm::value_ptr(TR));
				Fvertex[1][i].Bind();
				Fvertex[1][i].Draw();
			}
			else if (i == 4)
			{
				TR = glm::mat4(1.0f);
				TR = glm::scale(TR, glm::vec3(0.3f, 0.3f, 0.3f));
				TR = glm::rotate(TR, (float)glm::radians(-20.0), glm::vec3(1.0f, 0.0f, 0.0f));
				TR = glm::rotate(TR, (float)glm::radians(20.0), glm::vec3(0.0f, 1.0f, 0.0f));
				TR = glm::translate(TR, glm::vec3(0.5, -0.5, 0.0));
				TR = glm::rotate(TR, (float)glm::radians(pyramid[2]), glm::vec3(0.0f, 0.0f, -1.0f));
				TR = glm::translate(TR, glm::vec3(-0.5, 0.5, 0.0));
				glUniformMatrix4fv(modelLocation, 1, GL_FALSE, glm::value_ptr(TR));

				Fvertex[1][i].Bind();
				Fvertex[1][i].Draw();
			}
			else if (i == 5)
			{
				TR = glm::mat4(1.0f);
				TR = glm::scale(TR, glm::vec3(0.3f, 0.3f, 0.3f));
				TR = glm::rotate(TR, (float)glm::radians(-20.0), glm::vec3(1.0f, 0.0f, 0.0f));
				TR = glm::rotate(TR, (float)glm::radians(20.0), glm::vec3(0.0f, 1.0f, 0.0f));
				TR = glm::translate(TR, glm::vec3(0.0, -0.5, 0.5));
				TR = glm::rotate(TR, (float)glm::radians(pyramid[3]), glm::vec3(1.0f, 0.0f, 0.0f));
				TR = glm::translate(TR, glm::vec3(0.0, 0.5, -0.5));
				glUniformMatrix4fv(modelLocation, 1, GL_FALSE, glm::value_ptr(TR));

				Fvertex[1][i].Bind();
				Fvertex[1][i].Draw();
			}
			else
			{
				TR = glm::mat4(1.0f);
				TR = glm::rotate(TR, (float)glm::radians(-20.0), glm::vec3(1.0f, 0.0f, 0.0f));
				TR = glm::rotate(TR, (float)glm::radians(20.0), glm::vec3(0.0f, 1.0f, 0.0f));
				TR = glm::scale(TR, glm::vec3(0.3f, 0.3f, 0.3f));
				glUniformMatrix4fv(modelLocation, 1, GL_FALSE, glm::value_ptr(TR));
				Fvertex[1][i].Bind();
				Fvertex[1][i].Draw();
			}
		}
	}
	glutSwapBuffers();
	glutPostRedisplay();
}
GLvoid Reshape(int w, int h) //--- 콜백 함수: 다시 그리기 콜백 함수
{
	glViewport(0, 0, w, h);
}

void keyboard(unsigned char key, int x, int y) {
	switch (key) {
	case 'h':
		Hbool = !Hbool;
		break;
	case 'y':
		Ybool = !Ybool;
		break;
	case 't':
		Tbool = !Tbool;
		break;
	case 'f':
		Fbool = !Fbool;
		break;
	case 'F':
		F2bool = !F2bool;
		break;
	case '1':
		numbool = !numbool;
		break;
	case 'o':
		Obool = !Obool;
		break;
	case 'O':
		O2bool = !O2bool;
		break;
	case 'p':
		ver = !ver;
		break;
	case 'c':
		Cbool = !Cbool;
		break;
	}
	glutPostRedisplay();
}
void TimerFunction(int value)
{
	if (Ybool)
		--rotatey;
	if (Tbool)
		rotatet += 0.3;
	if (Fbool)
		rotatef += 0.01;
	if (F2bool) {
		rotatef -= 0.01;
		if (rotatef == 0)
			F2bool = false;
	}
	if (numbool)
		rotatenum += 0.001;
	if (num2bool) {
		rotatenum -= 0.001;
		if (rotatenum == 0)
			num2bool = false;
	}
	if (Obool) {
		for (int i = 0; i < 4; ++i)
			pyramid[i] += 0.1;
	}
	if (O2bool)
	{
		for (int i = 0; i < 4; ++i)
			pyramid[i] -= 0.1;
		if (pyramid[0] == 0)
			O2bool = false;
	}

	glutPostRedisplay();
	glutTimerFunc(10, TimerFunction, 1);
}

GLchar* filetobuf(const char* file) {
	FILE* fptr;
	long length;
	char* buf;
	fptr = fopen(file, "rb");
	if (!fptr)
		return NULL;
	fseek(fptr, 0, SEEK_END);
	length = ftell(fptr);
	buf = (char*)malloc(length + 1);
	fseek(fptr, 0, SEEK_SET);
	fread(buf, length, 1, fptr);
	fclose(fptr);
	buf[length] = 0;
	return buf;
}

void ReadObj(FILE* objFile)
{
	faceNum = 0;
	//--- 1. 전체 버텍스 개수 및 삼각형 개수 세기
	char count[100];
	char bind[100];
	int vertexNum = 0;
	while (!feof(objFile)) {
		fscanf(objFile, "%s", count);
		if (count[0] == 'v' && count[1] == '\0')
			vertexNum += 1;
		else if (count[0] == 'f' && count[1] == '\0')
			faceNum += 1;
		memset(count, '\0', sizeof(count));
	}
	int vertIndex = 0;
	int faceIndex = 0;
	vertex = (glm::vec4*)malloc(sizeof(glm::vec4) * vertexNum);
	face = (glm::vec4*)malloc(sizeof(glm::vec4) * faceNum);

	fseek(objFile, 0, 0);
	while (!feof(objFile)) {
		fscanf(objFile, "%s", bind);
		if (bind[0] == 'v' && bind[1] == '\0') {
			fscanf(objFile, "%f %f %f",
				&vertex[vertIndex].x, &vertex[vertIndex].y, &vertex[vertIndex].z);
			vertIndex++;
		}
		else if (bind[0] == 'f' && bind[1] == '\0') {
			fscanf(objFile, "%f %f %f",
				&face[faceIndex].x, &face[faceIndex].y, &face[faceIndex].z);
			int x = face[faceIndex].x - 1, y = face[faceIndex].y - 1, z = face[faceIndex].z - 1;
			faceIndex++;
		}
	}
}

void vectoplane(Plane* p) {
	for (int i = 0; i < faceNum; ++i) {
		int x = face[i].x - 1, y = face[i].y - 1, z = face[i].z - 1;
		p[i].p[0] = vertex[x].x;
		p[i].p[1] = vertex[x].y;
		p[i].p[2] = vertex[x].z;

		p[i].p[3] = vertex[y].x;
		p[i].p[4] = vertex[y].y;
		p[i].p[5] = vertex[y].z;

		p[i].p[6] = vertex[z].x;
		p[i].p[7] = vertex[z].y;
		p[i].p[8] = vertex[z].z;
	}
}

void planecolorset(Plane* p, int a) {
	if (a == 0) {
		float rgb[8][3]{};
		for (int i = 0; i < 8; ++i)
		{
			for (int j = 0; j < 3; ++j)
			{
				rgb[i][j] = uid(dre);
			}
		}
		for (int k = 0; k < 3; ++k)
		{
			p[0].color[k] = rgb[0][k];
			p[1].color[k] = rgb[0][k];
			p[2].color[k] = rgb[0][k];
			p[3].color[k] = rgb[0][k];
			p[4].color[k] = rgb[2][k];
			p[5].color[k] = rgb[2][k];
			p[6].color[k] = rgb[4][k];
			p[7].color[k] = rgb[4][k];
			p[8].color[k] = rgb[0][k];
			p[9].color[k] = rgb[0][k];
			p[10].color[k] = rgb[1][k];
			p[11].color[k] = rgb[1][k];
		}
		for (int k = 3; k < 6; ++k)
		{
			p[0].color[k] = rgb[6][k - 3];
			p[1].color[k] = rgb[2][k - 3];
			p[2].color[k] = rgb[3][k - 3];
			p[3].color[k] = rgb[1][k - 3];
			p[4].color[k] = rgb[7][k - 3];
			p[5].color[k] = rgb[3][k - 3];
			p[6].color[k] = rgb[6][k - 3];
			p[7].color[k] = rgb[7][k - 3];
			p[8].color[k] = rgb[4][k - 3];
			p[9].color[k] = rgb[5][k - 3];
			p[10].color[k] = rgb[5][k - 3];
			p[11].color[k] = rgb[7][k - 3];

		}
		for (int k = 6; k < 9; ++k)
		{
			p[0].color[k] = rgb[4][k - 6];
			p[1].color[k] = rgb[6][k - 6];
			p[2].color[k] = rgb[2][k - 6];
			p[3].color[k] = rgb[3][k - 6];
			p[4].color[k] = rgb[6][k - 6];
			p[5].color[k] = rgb[7][k - 6];
			p[6].color[k] = rgb[7][k - 6];
			p[7].color[k] = rgb[5][k - 6];
			p[8].color[k] = rgb[5][k - 6];
			p[9].color[k] = rgb[1][k - 6];
			p[10].color[k] = rgb[7][k - 6];
			p[11].color[k] = rgb[3][k - 6];
		}
	}
	else if (a == 1) {
		float rgb[5][3]{};
		for (int i = 0; i < 5; ++i)
		{
			for (int j = 0; j < 3; ++j)
			{
				rgb[i][j] = uid(dre);
			}
		}
		for (int k = 0; k < 3; ++k)
		{
			p[0].color[k] = rgb[0][k];
			p[1].color[k] = rgb[0][k];
			p[2].color[k] = rgb[1][k];
			p[3].color[k] = rgb[0][k];
			p[4].color[k] = rgb[3][k];
			p[5].color[k] = rgb[2][k];
		}
		for (int k = 3; k < 6; ++k)
		{
			p[0].color[k] = rgb[2][k - 3];
			p[1].color[k] = rgb[3][k - 3];
			p[2].color[k] = rgb[0][k - 3];
			p[3].color[k] = rgb[3][k - 3];
			p[4].color[k] = rgb[2][k - 3];
			p[5].color[k] = rgb[1][k - 3];
		}
		for (int k = 6; k < 9; ++k)
		{
			p[0].color[k] = rgb[1][k - 6];
			p[1].color[k] = rgb[2][k - 6];
			p[2].color[k] = rgb[4][k - 6];
			p[3].color[k] = rgb[4][k - 6];
			p[4].color[k] = rgb[4][k - 6];
			p[5].color[k] = rgb[4][k - 6];
		}
	}
}




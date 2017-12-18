#include "Render.h"

#include <windows.h>
#include <GL\GL.h>
#include <GL\GLU.h>

#include "MyOGL.h"

#include "Camera.h"
#include "Light.h"
#include "Primitives.h"


double* GetNormal(double x1, double y1, double z1, double x2, double y2, double z2) {
	double *vec = new double[3];
	double x3 = y1*z2 - z1*y2;
	double y3 = -1*(x1*z2 - x2*z1);
	double z3 = x1*y2 - x2*y1;
	double length = sqrt(x3*x3 + y3*y3 + z3*z3);
	x3 /= length;
	y3 /= length;
	z3 /= length;
	vec[0] = x3;
	vec[1] = y3;
	vec[2] = z3;
	return vec;
}

bool textureMode = true;
bool lightMode = true;

//класс для настройки камеры
class CustomCamera : public Camera
{
public:
	//дистанция камеры
	double camDist;
	//углы поворота камеры
	double fi1, fi2;

	
	//значния масеры по умолчанию
	CustomCamera()
	{
		camDist = 15;
		fi1 = 1;
		fi2 = 1;
	}

	
	//считает позицию камеры, исходя из углов поворота, вызывается движком
	void SetUpCamera()
	{
		//отвечает за поворот камеры мышкой
		lookPoint.setCoords(0, 0, 0);

		pos.setCoords(camDist*cos(fi2)*cos(fi1),
			camDist*cos(fi2)*sin(fi1),
			camDist*sin(fi2));

		if (cos(fi2) <= 0)
			normal.setCoords(0, 0, -1);
		else
			normal.setCoords(0, 0, 1);

		LookAt();
	}

	void CustomCamera::LookAt()
	{
		//функция настройки камеры
		gluLookAt(pos.X(), pos.Y(), pos.Z(), lookPoint.X(), lookPoint.Y(), lookPoint.Z(), normal.X(), normal.Y(), normal.Z());
	}



}  camera;   //создаем объект камеры


//Класс для настройки света
class CustomLight : public Light
{
public:
	CustomLight()
	{
		//начальная позиция света
		pos = Vector3(1, 1, 3);
	}

	
	//рисует сферу и линии под источником света, вызывается движком
	void  DrawLightGhismo()
	{
		glDisable(GL_LIGHTING);

		
		glColor3d(0.9, 0.8, 0);
		Sphere s;
		s.pos = pos;
		s.scale = s.scale*0.08;
		s.Show();
		
		if (OpenGL::isKeyPressed('G'))
		{
			glColor3d(0, 0, 0);
			//линия от источника света до окружности
			glBegin(GL_LINES);
			glVertex3d(pos.X(), pos.Y(), pos.Z());
			glVertex3d(pos.X(), pos.Y(), 0);
			glEnd();

			//рисуем окруность
			Circle c;
			c.pos.setCoords(pos.X(), pos.Y(), 0);
			c.scale = c.scale*1.5;
			c.Show();
		}

	}

	void SetUpLight()
	{
		GLfloat amb[] = { 0.2, 0.2, 0.2, 0 };
		GLfloat dif[] = { 1.0, 1.0, 1.0, 0 };
		GLfloat spec[] = { .7, .7, .7, 0 };
		GLfloat position[] = { pos.X(), pos.Y(), pos.Z(), 1. };

		// параметры источника света
		glLightfv(GL_LIGHT0, GL_POSITION, position);
		// характеристики излучаемого света
		// фоновое освещение (рассеянный свет)
		glLightfv(GL_LIGHT0, GL_AMBIENT, amb);
		// диффузная составляющая света
		glLightfv(GL_LIGHT0, GL_DIFFUSE, dif);
		// зеркально отражаемая составляющая света
		glLightfv(GL_LIGHT0, GL_SPECULAR, spec);

		glEnable(GL_LIGHT0);
	}


} light;  //создаем источник света




//старые координаты мыши
int mouseX = 0, mouseY = 0;

void mouseEvent(OpenGL *ogl, int mX, int mY)
{
	int dx = mouseX - mX;
	int dy = mouseY - mY;
	mouseX = mX;
	mouseY = mY;

	//меняем углы камеры при нажатой левой кнопке мыши
	if (OpenGL::isKeyPressed(VK_RBUTTON))
	{
		camera.fi1 += 0.01*dx;
		camera.fi2 += -0.01*dy;
	}

	
	//двигаем свет по плоскости, в точку где мышь
	if (OpenGL::isKeyPressed('G') && !OpenGL::isKeyPressed(VK_LBUTTON))
	{
		LPPOINT POINT = new tagPOINT();
		GetCursorPos(POINT);
		ScreenToClient(ogl->getHwnd(), POINT);
		POINT->y = ogl->getHeight() - POINT->y;

		Ray r = camera.getLookRay(POINT->x, POINT->y);

		double z = light.pos.Z();

		double k = 0, x = 0, y = 0;
		if (r.direction.Z() == 0)
			k = 0;
		else
			k = (z - r.origin.Z()) / r.direction.Z();

		x = k*r.direction.X() + r.origin.X();
		y = k*r.direction.Y() + r.origin.Y();

		light.pos = Vector3(x, y, z);
	}

	if (OpenGL::isKeyPressed('G') && OpenGL::isKeyPressed(VK_LBUTTON))
	{
		light.pos = light.pos + Vector3(0, 0, 0.02*dy);
	}

	
}

void mouseWheelEvent(OpenGL *ogl, int delta)
{

	if (delta < 0 && camera.camDist <= 1)
		return;
	if (delta > 0 && camera.camDist >= 100)
		return;

	camera.camDist += 0.01*delta;

}

void keyDownEvent(OpenGL *ogl, int key)
{
	if (key == 'L')
	{
		lightMode = !lightMode;
	}

	if (key == 'T')
	{
		textureMode = !textureMode;
	}

	if (key == 'R')
	{
		camera.fi1 = 1;
		camera.fi2 = 1;
		camera.camDist = 15;

		light.pos = Vector3(1, 1, 3);
	}

	if (key == 'F')
	{
		light.pos = camera.pos;
	}
}

void keyUpEvent(OpenGL *ogl, int key)
{
	
}




//выполняется перед первым рендером
void initRender(OpenGL *ogl)
{
	//настройка текстур

	//4 байта на хранение пикселя
	glPixelStorei(GL_UNPACK_ALIGNMENT, 4);

	//настройка режима наложения текстур
	glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);

	//включаем текстуры
	glEnable(GL_TEXTURE_2D);
	

	//массив трехбайтных элементов  (R G B)
	RGBTRIPLE *texarray;

	//массив символов, (высота*ширина*4      4, потомучто   выше, мы указали использовать по 4 байта на пиксель текстуры - R G B A)
	char *texCharArray;
	int texW, texH;
	OpenGL::LoadBMP("texture.bmp", &texW, &texH, &texarray);
	OpenGL::RGBtoChar(texarray, texW, texH, &texCharArray);

	
	GLuint texId;
	//генерируем ИД для текстуры
	glGenTextures(1, &texId);
	//биндим айдишник, все что будет происходить с текстурой, будте происходить по этому ИД
	glBindTexture(GL_TEXTURE_2D, texId);

	//загружаем текстуру в видеопямять, в оперативке нам больше  она не нужна
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, texW, texH, 0, GL_RGBA, GL_UNSIGNED_BYTE, texCharArray);

	//отчистка памяти
	free(texCharArray);
	free(texarray);

	//наводим шмон
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST); 
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);


	//камеру и свет привязываем к "движку"
	ogl->mainCamera = &camera;
	ogl->mainLight = &light;

	// нормализация нормалей : их длины будет равна 1
	glEnable(GL_NORMALIZE);

	// устранение ступенчатости для линий
	glEnable(GL_LINE_SMOOTH); 


	//   задать параметры освещения
	//  параметр GL_LIGHT_MODEL_TWO_SIDE - 
	//                0 -  лицевые и изнаночные рисуются одинаково(по умолчанию), 
	//                1 - лицевые и изнаночные обрабатываются разными режимами       
	//                соответственно лицевым и изнаночным свойствам материалов.    
	//  параметр GL_LIGHT_MODEL_AMBIENT - задать фоновое освещение, 
	//                не зависящее от сточников
	// по умолчанию (0.2, 0.2, 0.2, 1.0)

	glLightModeli(GL_LIGHT_MODEL_TWO_SIDE, 0);
}





void Render(OpenGL *ogl)
{       	
	
	
	
	glDisable(GL_TEXTURE_2D);
	glDisable(GL_LIGHTING);

	glEnable(GL_DEPTH_TEST);
	if (textureMode)
		glEnable(GL_TEXTURE_2D);

	if (lightMode)
		glEnable(GL_LIGHTING);


	//альфаналожение
	//glEnable(GL_BLEND);
	//glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);


	//настройка материала
	GLfloat amb[] = { 0.2, 0.2, 0.1, 1. };
	GLfloat dif[] = { 0.4, 0.65, 0.5, 1. };
	GLfloat spec[] = { 0.9, 0.8, 0.3, 1. };
	GLfloat sh = 0.1f * 256;


	//фоновая
	glMaterialfv(GL_FRONT, GL_AMBIENT, amb);
	//дифузная
	glMaterialfv(GL_FRONT, GL_DIFFUSE, dif);
	//зеркальная
	glMaterialfv(GL_FRONT, GL_SPECULAR, spec);\
	//размер блика
	glMaterialf(GL_FRONT, GL_SHININESS, sh);
	glEnable(GL_COLOR_MATERIAL);
    //чтоб было красиво, без квадратиков (сглаживание освещения)
	glShadeModel(GL_SMOOTH);
	//===================================
	//Прогать тут  
	double C[]{ 0.79688,0.73438 }, R = 0.29;

	//низ
	


	glBegin(GL_QUADS);
	glColor3f(1.0f, 1.0f, 1.0f);
	glNormal3f(0, 0, -1);
	glTexCoord2d((double)248 / 512, (double)(512 - 244) / 512);
	glVertex3f(-6, 0, 0);
	glTexCoord2d((double)348 / 512, (double)(512 - 244) / 512);
	glVertex3f(-12, -3, 0);
	glTexCoord2d((double)348 / 512, (double)(512 - 511) / 512);
	glVertex3f(-12, 0, 0);
	glTexCoord2d((double)248 / 512, (double)(512 - 511) / 512);
	glVertex3f(-6, 4, 0);
	glEnd();//

	glBegin(GL_TRIANGLES);
	glColor3f(1.0f, 0.0f, 1.0f);
	glNormal3f(0, 0, -1);
	glTexCoord2d((double)395 / 512, (double)(512 - 0) / 512);
	glVertex3f(-6, 0, 0);
	glTexCoord2d((double)395 / 512, (double)(512 - 126) / 512);
	glVertex3f(-6, 4, 0);
	glTexCoord2d((double)410 / 512, (double)(512 - 201) / 512);
	glVertex3f(-4, 7, 0);
	glEnd();//


	glBegin(GL_QUADS);
	glColor3f(1.0f, 0.0f, 1.0f);
	glNormal3f(0, 0, -1);
	glTexCoord2d((double)410 / 512, (double)(512 - 201) / 512);
	glVertex3f(-4, 7, 0);//4
	glTexCoord2d((double)464 / 512, (double)(512 - 240) / 512);
	glVertex3f(0, 11, 0);//3
	glTexCoord2d((double)464 / 512, (double)(512 - 0) / 512);
	glVertex3f(0, 0, 0);//2
	glTexCoord2d((double)395 / 512, (double)(512 - 0) / 512);
	glVertex3f(-6, 0, 0);//1
	glEnd();//

	//днище вырезы
	glBegin(GL_TRIANGLE_FAN);
	int p = 0;
	glColor3d(0, 0, 0);
	glNormal3f(0, 0, -1);
	glTexCoord2d((double)281 / 512, (double)(512 - 106) / 512);
	glVertex3d(-6, 4, 0);
	for (int i = 271; i < 306; i++)
	{
		glTexCoord2d((double)(311) / 512.0, (double)((512.0 - (105 - p*0.94)) / 512.0));
		glVertex3d(-10.5 + 11.510864433221338 * cos(i * 3.14 / 180), 16.5 + 11.510864433221338 * sin(i * 3.14 / 180), 0);
		p++;
	}
	glEnd();//

	glBegin(GL_TRIANGLE_FAN);
	glColor3d(0, 0, 0);
	p = 0;
	glNormal3f(0, 0, -1);
	glTexCoord2d((double)355 / 512, (double)(512 - 86) / 512);
	glVertex3d(-17, 3, 0);
	for (int i = 236; i < 275; i++)
	{
		glTexCoord2d((double)328 / 512, (double)(512 - (73 + p*0.76)) / 512);
		glVertex3d(-10.5 + 11.510864433221338 * cos(i * 3.14 / 180), 16.5 + 11.510864433221338 * sin(i * 3.14 / 180), 0);
		p++;

	}
	glEnd();//


	glBegin(GL_TRIANGLES);
	glNormal3f(0, 0, -1);
	glTexCoord2d((double)281 / 512, (double)(512 - 106) / 512);
	glVertex3d(-6, 4, 0);
	glTexCoord2d((double)355 / 512, (double)(512 - 86) / 512);
	glVertex3d(-17, 3, 0);
	glTexCoord2d((double)307 / 512, (double)(512 - 131) / 512);
	glVertex3d(-10, 5, 0);
	glEnd();//

	//верх
	glBegin(GL_TRIANGLES);
	glColor3f(0.5f, 1.0f, 0.6f);
	glNormal3f(0, 0, 1);
	glTexCoord2d((double)240 / 512, (double)(512 - 150) / 512);
	glVertex3f(-6, 0, 7);
	glTexCoord2d((double)0, (double)(512 - 144) / 512);
	glVertex3f(-6, 4, 7);
	glTexCoord2d((double)0, (double)(512.0 - 132) / 512.0);
	glVertex3f(-4, 7, 7);
	glEnd();//


	glBegin(GL_QUADS);
	glColor3f(0.5f, 1.0f, 0.6f);
	glNormal3f(0, 0, 1);
	glTexCoord2d((double)240/512, (double)(512 - 150) / 512);
	glVertex3f(-6, 0, 7);
	glTexCoord2d((double)244/512, (double)(512-214)/512);
	glVertex3f(-12, -3, 7);
	glTexCoord2d((double)0, (double)(512 - 236) / 512);
	glVertex3f(-12, 0, 7);
	glTexCoord2d((double)0, (double)(512-144)/512);
	glVertex3f(-6, 4, 7);
	glEnd();//


	glBegin(GL_QUADS);
	glColor3f(0.5f, 1.0f, 0.6f);
	glNormal3f(0, 0, 1);
	glTexCoord2d((double)0/512, (double)(512.0-132) / 512.0);
	glVertex3f(-4, 7, 7);
	glTexCoord2d((double)0/512, (double)(512-45)/512);
	glVertex3f(0, 11, 7);
	glTexCoord2d((double)243 / 512.0, (double)(512 - 45) / 512);
	glVertex3f(0, 0, 7);
	glTexCoord2d((double)243/512.0, (double)(512.0-132) / 512.0);
	glVertex3f(-6, 0, 7);
	glEnd();//


	glBegin(GL_TRIANGLE_FAN);
	glColor3d(0, 0, 0);
	glNormal3f(0, 0, 1);
	glTexCoord2d((double)281/512, (double)(512 - 106) / 512);
	p = 0;
	glVertex3d(-6, 4, 7);
	for (int i = 271; i < 306; i++)
	{
		glTexCoord2d((double)(311) / 512.0, (double)((512.0 - (105- p*0.94) )/ 512.0));
		glVertex3d(-10.5 + 11.510864433221338 * cos(i * 3.14 / 180), 16.5 + 11.510864433221338 * sin(i * 3.14 / 180), 7);
		p++;
	}
	glEnd();//

	glBegin(GL_TRIANGLE_FAN);
	glColor3d(0, 0, 0);
	glNormal3f(0, 0, 1);
	p = 0;
	glTexCoord2d((double)355 / 512, (double)(512 - 86) / 512);
	glVertex3d(-17, 3, 7);
	for (int i = 236; i < 275; i++)
	{
		glTexCoord2d((double)328 / 512, (double)(512 - (73+p*0.76)) / 512);
		glVertex3d(-10.5 + 11.510864433221338 * cos(i * 3.14 / 180), 16.5 + 11.510864433221338 * sin(i * 3.14 / 180), 7);
		p++;
	}
	glEnd();//

	glBegin(GL_TRIANGLES);
	glNormal3f(0, 0, 1);
	glTexCoord2d((double)281 / 512, (double)(512 - 106) / 512);
	glVertex3d(-6, 4, 7);
	glTexCoord2d((double)355 / 512, (double)(512 - 86) / 512);
	glVertex3d(-17, 3, 7);
	glTexCoord2d((double)307 / 512, (double)(512 - 131) / 512);
	glVertex3d(-10, 5, 7);
	glEnd();//

	//грани
	glBegin(GL_QUADS);
	glColor3f(1.0f, 1.0f, 0.6f);
	glNormal3f(-0.7,0.7,0);
	glTexCoord2d((double)251 / 512, (double)(512 - 236) / 512);
	glVertex3d(0, 11, 0);
	glTexCoord2d((double)251 / 512, (double)(512 - 0) / 512);
	glVertex3d(0, 11, 7);
	glTexCoord2d((double)374 / 512, (double)(512 - 0) / 512);
	glVertex3d(-4, 7, 7);
	glTexCoord2d((double)374 / 512, (double)(512 - 236) / 512);
	glVertex3d(-4, 7, 0);
	glEnd();//

	

	double *norm;
	glBegin(GL_QUADS);
	norm = GetNormal(0, 0, 7, -6, 0, 0);
	glNormal3d(norm[0], norm[1], norm[2]);
	glTexCoord2d((double)112 / 512, (double)(512 - 310) / 512);
	glVertex3d(-6, 0, 0);
	glTexCoord2d((double)112 / 512, (double)(512 - 240) / 512);
	glVertex3d(-6, 0, 7);
	glTexCoord2d((double)247 / 512, (double)(512 - 240) / 512);
	glVertex3d(0, 0, 7);
	glTexCoord2d((double)247 / 512, (double)(512 - 310) / 512);
	glVertex3d(0, 0, 0);
	glEnd();

	glBegin(GL_QUADS);
	norm = GetNormal(0, 0, 7, -6, -3, 0);
	glNormal3d(norm[0], norm[1], norm[2]);
	glTexCoord2d((double)0 / 512, (double)(512 - 310) / 512);
	glVertex3d(-12, -3, 0);
	glTexCoord2d((double)0 / 512, (double)(512 - 240) / 512);
	glVertex3d(-12, -3, 7);
	glTexCoord2d((double)112 / 512, (double)(512 - 240) / 512);
	glVertex3d(-6, 0, 7);
	glTexCoord2d((double)112 / 512, (double)(512 - 310) / 512);
	glVertex3d(-6, 0, 0);
	glEnd();

	glBegin(GL_QUADS);
	norm = GetNormal(0, 0, 7, 0, 3, 0);
	glNormal3d(norm[0], norm[1], norm[2]);
	glTexCoord2d((double)256 / 512, (double)(512 - 409) / 512);
	glVertex3d(-12, 0, 0);
	glTexCoord2d((double)348 / 512, (double)(512 - 409) / 512);
	glVertex3d(-12, 0, 7);
	glTexCoord2d((double)348 / 512, (double)(512 - 470) / 512);
	glVertex3d(-12, -3, 7);
	glTexCoord2d((double)256 / 512, (double)(512 - 470) / 512);
	glVertex3d(-12, -3, 0);
	glEnd();

	glBegin(GL_QUADS);
	norm = GetNormal(0, 0, 7, 6, 4, 0);
	glNormal3d(norm[0], norm[1], norm[2]);
	glTexCoord2d((double)142 / 512, (double)(512 - 398) / 512);
	glVertex3d(-6, 4, 0);
	glTexCoord2d((double)142 / 512, (double)(512 - 314) / 512);
	glVertex3d(-6, 4, 7);
	glTexCoord2d((double)248 / 512, (double)(512 - 314) / 512);
	glVertex3d(-12, 0, 7);
	glTexCoord2d((double)248 / 512, (double)(512 - 398) / 512);
	glVertex3d(-12, 0, 0);
	glEnd();


	glBegin(GL_QUADS);
	norm = GetNormal(0, 0, 7, -11, -1, 0);
	glNormal3d(norm[0], norm[1], norm[2]);
	glTexCoord2d((double)0 / 512, (double)(512 - 398) / 512);
	glVertex3d(-17, 3, 0);
	glTexCoord2d((double)0 / 512, (double)(512 - 314) / 512);
	glVertex3d(-17, 3, 7);
	glTexCoord2d((double)142 / 512, (double)(512 - 314) / 512);
	glVertex3d(-6, 4, 7);
	glTexCoord2d((double)142 / 512, (double)(512 - 398) / 512);
	glVertex3d(-6, 4, 0);
	glEnd();

	glBegin(GL_QUADS);
	norm = GetNormal(0, 0, 7, 0, 4, 0);
	glNormal3d(norm[0], norm[1], norm[2]);
	glTexCoord2d((double)101 / 512, (double)(512 - 510) / 512);
	glVertex3d(-17, 3, 0);
	glTexCoord2d((double)101 / 512, (double)(512 - 403) / 512);
	glVertex3d(-17, 3, 7);
	glTexCoord2d((double)244 / 512, (double)(512 - 403) / 512);
	glVertex3d(-17, 7, 7);
	glTexCoord2d((double)244 / 512, (double)(512 - 510) / 512);
	glVertex3d(-17, 7, 0);
	glEnd();

	//Окружность

	//впуклость
	glBegin(GL_QUAD_STRIP);
	double x1, x2, y1, y2;
	glColor3d(0.5, 0, 0.5);
	x1 = -10.5 + 11.510864433221338 * cos(236 * 3.14 / 180);
	y1 = 16.5 + 11.510864433221338 * sin(236 * 3.14 / 180);
	int k = 0;
	for (int i = 236; i < 307; i++)
	{

		if (i > 237) {
			x1 = x2;
			y1 = y2;
		}
		if (i >= 237) {
			x2 = -10.5 + 11.510864433221338 * cos(i * 3.14 / 180);
			y2 = 16.5 + 11.510864433221338 * sin(i * 3.14 / 180);
			norm = GetNormal(0, 0, 7, x2 - x1, y2 - y2, 0);
			glNormal3f(norm[0], norm[1], norm[2]);
		}
		glTexCoord2d((double)(353) / 512, (double)(512 - (243 + k * 3.7)) / 512);
		glVertex3d(-10.5 + 11.510864433221338 * cos(i * 3.14 / 180), 16.5 + 11.510864433221338 * sin(i * 3.14 / 180), 0);
		glTexCoord2d((double)(512) / 512, (double)(512 - (243 + k * 3.7)) / 512);
		glVertex3d(-10.5 + 11.510864433221338 * cos(i * 3.14 / 180), 16.5 + 11.510864433221338 * sin(i * 3.14 / 180), 7);
		k++;
	}
	glEnd();



	//выпуклость

	glBegin(GL_TRIANGLE_FAN);
	glColor3d(0.5, 0, 0.2);
	glNormal3f(0, 0, -1);
	glTexCoord2d((double)464/512, (double)240/512);
	glVertex3d(0, 11, 0);
	p = 1;
	for (int i = -58; i < 58; i++)
	{
		if(p>1)
			glTexCoord2d(C[0] + (R-0.02)*cos((10-i)* M_PI/180), C[1]+0.035 + (R-0.02)*sin((10-i)*M_PI/180));
		glVertex3d(-3.5 + 6.519202405202649 * cos(i * 3.14 / 180), 5.5 + 6.519202405202649 * sin(i * 3.14 / 180), 0);
		p++;
	}
	glEnd();//

	

	glBegin(GL_TRIANGLE_FAN);
	glColor3d(0.5, 0, 0.2);
	glNormal3f(0, 0, 1);
	p = 1;
	glTexCoord2d((double)150/512, (double)(512 - 98) / 512);
	glVertex3d(-3.5, 5.5, 7);
	for (int i = -58; i < 58; i++)
	{
		if(p>1)
			glTexCoord2d((double)(220 - p*2.12) / 512.0, (double)(512.0 - 0) / 512.0);
		glVertex3d(-3.5 + 6.519202405202649 * cos(i * 3.14 / 180), 5.5 + 6.519202405202649 * sin(i * 3.14 / 180), 7);
		p++;
	}
	glEnd();//

	glBegin(GL_QUAD_STRIP);
	glColor3d(0.5, 0, 0.5);
	x1 = -3.5 + 6.519202405202649 * cos(-58 * 3.14 / 180);
	y1 = 5.5 + 6.519202405202649 * sin(-58 * 3.14 / 180);
	int l= 0;
	for (int i = -58; i < 58.5; i++)
	{
		if (i > -59) {
			x1 = x2;
			y1 = y2;
		}
		if (i >= -59) {
			x2 = -3.5 + 6.519202405202649 * cos(i * 3.14 / 180);
			y2 = 5.5 + 6.519202405202649 * sin(i * 3.14 / 180);
			norm = GetNormal(0, 0, 7, x1-x2, y1-y2 , 0);
			glNormal3f(norm[0], norm[1], norm[2]);
		}
		glTexCoord2d((double)(0 + l * 2) / 512, (double)(512 - 240) / 512);
		glVertex3d(-3.5 + 6.519202405202649 * cos(i * 3.14 / 180), 5.5 + 6.519202405202649 * sin(i * 3.14 / 180), 0);
		glTexCoord2d((double)(0 + l * 2) / 512, (double)(512 - 0) / 512);
		glVertex3d(-3.5 + 6.519202405202649 * cos(i * 3.14 / 180), 5.5 + 6.519202405202649 * sin(i * 3.14 / 180), 7);
		l++;
	}
	glEnd();






	
	//текст сообщения вверху слева, если надоест - закоментировать, или заменить =)
	char c[250];  //максимальная длина сообщения
	sprintf_s(c, "(T)Текстуры - %d\n(L)Свет - %d\n\nУправление светом:\n"
		"G - перемещение в горизонтальной плоскости,\nG+ЛКМ+перемещение по вертикальной линии\n"
		"R - установить камеру и свет в начальное положение\n"
		"F - переместить свет в точку камеры", textureMode, lightMode);
	ogl->message = std::string(c);




}   //конец тела функции

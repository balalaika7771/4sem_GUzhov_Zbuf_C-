#include <cmath>
#include <iostream>
#include <stdlib.h>
#include <windows.h>
#include <vector>
#include <stdio.h>
#include <string>
#include <fstream>
using namespace std;
const double M_PI = 40;

class TFigure {}; //смещение для рисунка
HWND hwnd = GetConsoleWindow(); //Берём ориентир на консольное окно (В нём будем рисовать)
HDC dc = GetDC(hwnd); //Цепляемся к консольному окну
#define MAXDIST 1000.0 //Максимальная глубина сцены
#define MAXYLINES 1000 //Максимальное количество линий в сцене.
#define clBk RGB(15,15,15); //Цвет по умолчанию
float zoom = 1;
int RRRX = 100, RRRY = 100;
typedef struct Point3d POINT3D;
typedef struct Cell CELL;
void line(const HDC& dc, const int x1, const int y1, const int x2, const int y2) {
	MoveToEx(dc, x1, y1, 0);
	LineTo(dc, x2, y2);
}

COLORREF paint[12] = { RGB(rand() % 250,rand() % 250,rand() % 250),RGB(rand() % 250, rand() % 250,rand() % 250),RGB(rand() % 250,rand() % 250,rand() % 250) ,RGB(rand() % 250,rand() % 250,rand() % 250) ,RGB(rand() % 250,rand() % 250,rand() % 250) ,RGB(rand() % 250, rand() % 250, rand() % 250) ,RGB(rand() % 250, rand() % 250, rand() % 250) ,RGB(rand() % 250, rand() % 250, rand() % 250) ,RGB(rand() % 250,rand() % 250,rand() % 250),RGB(rand() % 250,rand() % 250,rand() % 250),RGB(rand() % 250,rand() % 250,rand() % 250),RGB(rand() % 250,rand() % 250,rand() % 250) };

struct Point3d {
	double x, y, z;
};

struct Cell {
	double z;
	COLORREF color;
};

class Triangle {
public:
	COLORREF color;
	POINT3D p[3];
	Triangle(POINT3D p1, POINT3D p2, POINT3D p3, COLORREF c) {
		p[0] = p1; p[1] = p2; p[2] = p3;
		color = c;
	}
};

class ZBuffer {
public:
	CELL* buff[MAXYLINES];
	int sX, sY;	// Размер Z-Буфера
	ZBuffer(int, int);
	~ZBuffer();
	void PutTriangle(Triangle&);
	void Show();
	void Clear();
};

ZBuffer::ZBuffer(int ax, int ay) {
	sX = ax; sY = ay;
	for (int i = 0; i < sY; i++) {
		//Выделение памяти под ячейки
		buff[i] = (struct Cell*)malloc(sX * sizeof(struct Cell));
	}
}

//Деструктор Z-буфера.
ZBuffer :: ~ZBuffer() {
	for (int i = 0; i < sY; i++)
		free(buff[i]); //Освобождение памяти
}
ZBuffer* zb;
ZBuffer* zb2;

//C:\Qt\cube.obj

bool ravenstvo(ZBuffer* zb, ZBuffer* zb2) {
	for (int j = 0; j < zb->sY; j++) {
		for (int i = 0; i < zb->sX; i++) {
			if ((*(zb->buff[j] + i)).color != (*(zb2->buff[j] + i)).color) {

				for (int j = 0; j < zb->sY; j++) {
					for (int i = 0; i < zb->sX; i++) {
						(*(zb2->buff[j] + i)).color = (*(zb->buff[j] + i)).color;

					}
				}

				return true;
			}
		}
	}
	Sleep(30);
	return false;
}

//Функция, выводящая на экран содержимое заполненного Z-буфера.
void ZBuffer::Show() {
	if (ravenstvo(zb, zb2)) {
		RECT rect;
		GetWindowRect(hwnd, &rect);
		HBRUSH ki = CreateSolidBrush(RGB(15, 15, 15));
		FillRect(dc, &rect, ki);
		for (int j = 0; j < zb->sY; j++)
			for (int i = 0; i < zb->sX; i++)
				if ((*(zb->buff[j] + i)).color != RGB(15, 15, 15)) {
					SetPixel(dc, i, j, (*(zb->buff[j] + i)).color);
				}
		std::cout << ".";
	}
}

//Функция, 'очищающая' Z-буфер.
void ZBuffer::Clear() {
	for (int j = 0; j < sY; j++)
		for (int i = 0; i < sX; i++)
			//Инициализируем ячейки Z-буфера
			(*(buff[j] + i)).z = MAXDIST, (*(buff[j] + i)).color = clBk;
}

void ZBuffer::PutTriangle(Triangle& t) {
	int ymax, ymin, ysc, e1, e, i;
	int x[3], y[3];
	//Заносим x,y из t в массивы для последующей работы с ними
	for (i = 0; i < 3; i++)
		x[i] = int(t.p[i].x), y[i] = int(t.p[i].y);
	//Определяем максимальный и минимальный y
	ymax = ymin = y[0];
	if (ymax < y[1]) ymax = y[1]; else if (ymin > y[1]) ymin = y[1];
	if (ymax < y[2]) ymax = y[2]; else if (ymin > y[2]) ymin = y[2];
	ymin = (ymin < 0) ? 0 : ymin;
	ymax = (ymax < sY) ? ymax : sY;
	int ne;
	int x1, x2, xsc1, xsc2;
	double z1, z2, tc, z;
	//Следующий участок кода перебирает все строки сцены
	//и определяет глубину каждого пикселя
	//для соответствующего треугольника
	for (ysc = ymin; ysc < ymax; ysc++) {
		ne = 0;
		for (int e = 0; e < 3; e++) {
			e1 = e + 1;
			if (e1 == 3) e1 = 0;
			if (y[e] < y[e1]) {
				if (y[e1] <= ysc || ysc < y[e]) continue;
			}
			else if (y[e] > y[e1]) {
				if (y[e1] > ysc || ysc >= y[e]) continue;
			}
			else continue;
			tc = double(y[e] - ysc) / (y[e] - y[e1]);
			if (ne)
				x2 = x[e] + int(tc * (x[e1] - x[e])),
				z2 = t.p[e].z + tc * (t.p[e1].z - t.p[e].z);
			else
				x1 = x[e] + int(tc * (x[e1] - x[e])),
				z1 = t.p[e].z + tc * (t.p[e1].z - t.p[e].z),
				ne = 1;
		}
		if (x2 < x1) e = x1, x1 = x2, x2 = e, tc = z1, z1 = z2, z2 = tc;
		xsc1 = (x1 < 0) ? 0 : x1;
		xsc2 = (x2 < sX) ? x2 : sX;
		for (int xsc = xsc1; xsc < xsc2; xsc++) {
			tc = double(x1 - xsc) / (x1 - x2);
			z = z1 + tc * (z2 - z1);
			//Если полученная глубина пиксела меньше той,
			//что находится в Z-Буфере - заменяем храняшуюся на новую.
			if (z < (*(buff[ysc] + xsc)).z)
				(*(buff[ysc] + xsc)).color = t.color,
				(*(buff[ysc] + xsc)).z = z;
		}
	}
}



class TPoint3D :public TFigure {
	double x, y, z;
public:
	virtual void setx(const double value) { x = value; }
	virtual void sety(const double value) { y = value; }
	virtual void setz(const double value) { z = value; }
	virtual void set(const double value1, const double value2, const double value3) {
		x = value1, y = value2, z = value3;
	}

	virtual double getx() const { return x * zoom; }
	virtual double gety() const { return y * zoom; }
	virtual double getz() const { return z * zoom; }

};

class TPoint : public TFigure {
	double x, y;
public:
	virtual void setx(const double value) { x = value; }
	virtual void sety(const double value) { y = value; }

	virtual double getx() const { return x; }
	virtual double gety() const { return y; }
};


class TCube3D {
	std::vector<TPoint3D> w;  //мировые координаты
	std::vector<int>  trian;
	std::vector<HPEN> penz;
	std::vector<TPoint3D> WN;

public:
	
	TCube3D() {
		ifstream in;
		string file_name;

		cout << "\nEnter file name: ";
		cin >> file_name;
		// C:\Qt\cube.obj

		in.open(file_name.c_str());
		if (!in.is_open())
		{
			cout << "\nCan not open file for reading!\n";
			return;
		}
		string str;
		while (!in.eof())
		{
			getline(in, str);
			if (str[str.length()] == '\n') str[str.length()] = ' ';
			if (str[0] == 'v') {
				int x, y, z;
				string s[3];
				getline(in, s[0]);
				getline(in, s[1]);
				getline(in, s[2]);
				x = atoi(s[0].c_str());
				y = atoi(s[1].c_str());
				z = atoi(s[2].c_str());
				TPoint3D point;
				point.set(x, y, z);
				w.push_back(point);
				WN.push_back(point);
			}
			if (str[0] == 'k') {
				int p1, p2,p3;
				string p[3];
				getline(in, p[0]);
				getline(in, p[1]);
				getline(in, p[2]);
			
				p1 = atoi(p[0].c_str());
				p2 = atoi(p[1].c_str());
				p3 = atoi(p[2].c_str());
			
				trian.push_back(p1);
				trian.push_back(p2);
				trian.push_back(p3);

			}
		}
		
	}

	/*ПЕРЕВОД МИРОВЫХ КООРИНАТ В ВИДОВЫЕ*/
	virtual void view_transformation(double LX, double LY, double LZ) {


		for (int i = 0; i < w.size(); i++) {

			double KX1, KY1, KZ1;
			KX1 = w[i].getx();
			KY1 = w[i].gety() * cos(LX) + w[i].getz() * sin(LX);
			KZ1 = -w[i].gety() * sin(LX) + w[i].getz() * cos(LX);

			double KX2, KY2, KZ2;
			KX2 = KX1 * cos(LY) - KZ1 * sin(LY);
			KY2 = KY1;
			KZ2 = KX1 * sin(LY) + KZ1 * cos(LY);

			double KX3, KY3, KZ3;
			KX3 = KX2 * cos(LZ) + KY2 * sin(LZ);
			KY3 = -KX2 * sin(LZ) + KY2 * cos(LZ);
			KZ3 = KZ2;
			KX3 = KX3 + RRRX;
			KY3 = KY3 + RRRY;
			

			TPoint3D N;
			N.setx(KX3);
			N.sety(KY3);
			N.setz(KZ3);
			WN[i] = N;
			
		}
	}
	
	virtual void draw(ZBuffer *zb) const {
		int j = 0;

		for (int i = 0; i < trian.size(); i = i + 3) {
			POINT3D p[3] = { {WN[trian[i]].getx(),WN[trian[i]].gety(),WN[trian[i]].getz()},{WN[trian[i+1]].getx(),WN[trian[i + 1]].gety(),WN[trian[i + 1]].getz()},{WN[trian[i + 2]].getx(),WN[trian[i + 2]].gety(),WN[trian[i + 2]].getz()} };
			Triangle t1(p[0], p[1], p[2],paint[j++]);
			zb->PutTriangle(t1);
		}
	}

	
	
};


int main() {

	
	ifstream in;
	string file_name;
	
	zb = new ZBuffer(1000, 1000); //Создаем Z-буфер с нужными размерами
	zb->Clear();
	zb2 = new ZBuffer(1000, 1000); //Создаем Z-буфер с нужными размерами
	zb2->Clear();


	TCube3D cub;
	double teta = 0, phi = 0, zer = 0;   //углы для поворота


	for (int i = 0; i < 10000; i++) {
		
		if (GetAsyncKeyState(38)) //нажат ентер
		{
			phi += (M_PI / (360));

		}
		if (GetAsyncKeyState(40)) //нажат пробел
		{
		phi -= (M_PI / (360));
		}
		if (GetAsyncKeyState(39)) {
		teta += (M_PI / (360));
		}
		if (GetAsyncKeyState(37)) {
		teta -=(M_PI / (360));
		}
		if (GetAsyncKeyState(87)) {
		RRRY -= 10;
		}
		if (GetAsyncKeyState(83)) {
		RRRY += 10;
		}
		if (GetAsyncKeyState(65)) {
		RRRX -= 10;
		}
		if (GetAsyncKeyState(68)) {
		RRRX += 10;
		}
		if (GetAsyncKeyState(90)) {
		zoom += 0.1;
		}
		if (GetAsyncKeyState(88)) {
		zoom -= 0.1;
		}
		if (GetAsyncKeyState(77)) {
		zer += (M_PI / (360));
		}
		if (GetAsyncKeyState(78)) {
		zer -= (M_PI / (360));
		}
		cub.view_transformation(phi, teta, zer);
		cub.draw(zb);
		zb->Show();
		zb->Clear();
		

		

	}


	ReleaseDC(hwnd, dc);
}
//C:\Qt\cube.obj
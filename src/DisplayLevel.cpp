#include "DisplayLevel.hpp"
#include <qpainter.h>
#include <QPaintEvent>
#include <Q3Frame>

DisplayLevel::DisplayLevel(QWidget* parent)
	: Q3Frame(parent), w(0)
{
	setMinimumHeight(20);
	setFrameStyle(Q3Frame::Panel|Q3Frame::Sunken);
	setMargin(2);
	m=margin();
}

void DisplayLevel::paintEvent(QPaintEvent* e)
{
	Q3Frame::paintEvent(e);
	QPainter paint;
	paint.begin(this);
	paint.setPen(Qt::black);
	paint.setBrush(Qt::gray);
	paint.drawRect(m,m,w,height()-2*m);
	paint.end();
}

void DisplayLevel::setZero(void)
{
	w=0;
}

void DisplayLevel::samples(short* buffer, int n)
{
	short min=32767;
	short max=-32768;
	for(int i=0; i<n; i++) {
		short s=buffer[i];
		if(s>max) {
			max=s;
		}
		if(s<min) {
			min=s;
		}
	}
	double level=(max-min)/65536.0;
	w=static_cast<int>(level*(width()-2*m));
	update();
}

#include "DisplayLevel.hpp"
#include <QPaintEvent>
#include <QPainter>

DisplayLevel::DisplayLevel(QWidget* parent)
	: QFrame(parent), margin(2), length(0)
{
	setMinimumHeight(20);
	setFrameStyle(QFrame::Panel | QFrame::Sunken);
	setContentsMargins(margin, margin, margin, margin);
}

void DisplayLevel::paintEvent(QPaintEvent* e)
{
	QFrame::paintEvent(e);
	QPainter paint;
	paint.begin(this);
	paint.setPen(Qt::black);
	paint.setBrush(Qt::gray);
	paint.drawRect(margin, margin, length, height() - 2 * margin);
	paint.end();
}

void DisplayLevel::setZero(void)
{
	length = 0;
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
	length = static_cast<int>(level * (width() - 2 * margin));
	update();
}

#ifndef DISPLAYLEVEL_HPP
#define DISPLAYLEVEL_HPP

#include <qwidget.h>
#include <qframe.h>

class DisplayLevel : public QFrame {
	Q_OBJECT
public:
	DisplayLevel(QWidget* parent);
	virtual void paintEvent(QPaintEvent* e);
	void setZero(void);
private:
	int m;
	int w;
public slots:
        void samples(short* buffer, int n);
};

#endif

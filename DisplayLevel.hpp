#ifndef DISPLAYLEVEL_HPP
#define DISPLAYLEVEL_HPP

#include <qwidget.h>
#include <qframe.h>

class DisplayLevel : public QFrame {
	Q_OBJECT
public:
	DisplayLevel(QWidget* parent);
	virtual void paintEvent(QPaintEvent* e);
private:
	int m;
	int w;
public slots:
        void samples(signed short* buffer, unsigned int n);
};

#endif

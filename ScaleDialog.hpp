// HamFax -- an application for sending and receiving amateur radio facsimiles
// Copyright (C) 2001 Christof Schmittt, DH1CS <cschmit@suse.de>
//  
// This program is free software; you can redistribute it and/or
// modify it under the terms of the GNU General Public License
// as published by the Free Software Foundation; either version 2
// of the License, or (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//  
// You should have received a copy of the GNU General Public License
// along with this program; if not, write to the Free Software
// Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.

#ifndef SCALEDIALOG_HPP
#define SCALEDIALOG_HPP

#include <qdialog.h>
#include <qlabel.h>
#include <qspinbox.h>

class ScaleDialog : public QDialog {
	Q_OBJECT
public:
	ScaleDialog(QWidget* parent);
	void init(void);
	unsigned int width;
	unsigned int height;
private:
	QSpinBox* ioc;
	QLabel* widthLabel;
	QLabel* heightLabel;
private slots:
        void newIOC(int ioc);
	void okClicked(void);
	void cancelClicked(void);
};

#endif

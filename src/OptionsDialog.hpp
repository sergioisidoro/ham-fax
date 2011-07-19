// hamfax -- an application for sending and receiving amateur radio facsimiles
// Copyright (C) 2001,2002
// Christof Schmitt, DH1CS <cschmitt@users.sourceforge.net>
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

#ifndef OPTIONSDIALOG_HPP
#define OPTIONSDIALOG_HPP

#include <QComboBox>
#include <QDialog>
#include <QGridLayout>
#include <QLineEdit>
#include <QString>
#include "config.h"

class OptionsDialog : public QDialog {
	Q_OBJECT
public:
	OptionsDialog(QWidget* parent);
private:
	QLineEdit* addItem(const QString& name,  const QString& config);
	QGridLayout *settings;
	int row;
	QLineEdit* devDSP;
	QLineEdit* devPTT;
	QLineEdit* devPTC;
	QComboBox* speedPTC;
#ifdef HAVE_LIBHAMLIB
	QLineEdit* hamlibModel;
	QLineEdit* hamlibParams;
#endif
private slots:
        void okClicked(void);
	void cancelClicked(void);
};

#endif

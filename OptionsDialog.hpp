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

#ifndef OPTIONSDIALOG_HPP
#define OPTIONSDIALOG_HPP

#include <qdialog.h>
#include <qstring.h>
#include <qlineedit.h>

class OptionsDialog : public QDialog {
	Q_OBJECT
public:
	OptionsDialog(QWidget* parent);
	void init(void);
	QString devDSPName;
	QString devPTTName;
	QString devPTCName;
private:
	QLineEdit* devDSP;
	QLineEdit* devPTT;
	QLineEdit* devPTC;
private slots:
        void dspChanged(const QString& s);
	void pttChanged(const QString& s);
	void ptcChanged(const QString& s);
        void okClicked(void);
	void cancelClicked(void);
};

#endif

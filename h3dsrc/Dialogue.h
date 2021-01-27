// abmap
// Copyright (C) 2019 Helen Ginn
// 
// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
// 
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
// 
// You should have received a copy of the GNU General Public License
// along with this program.  If not, see <https://www.gnu.org/licenses/>.
// 
// Please email: vagabond @ hginn.co.uk for more details.

#ifndef __abmap__Dialogue__
#define __abmap__Dialogue__

#include <QFileDialog>
#include <QMessageBox>

inline std::string openDialogue(QWidget *w, QString title, QString pattern,
                                bool write = false, bool dir = false)
{
	QFileDialog *f = new QFileDialog(w, title, pattern);
	                                 
	if (!dir)
	{
		f->setFileMode(QFileDialog::AnyFile);
	}
	else
	{
		f->setFileMode(QFileDialog::Directory);
	}

	f->setOptions(QFileDialog::DontUseNativeDialog);
	
	if (write)
	{
		f->setAcceptMode(QFileDialog::AcceptSave);
	}
	f->show();

    QStringList fileNames;

    if (f->exec())
    {
        fileNames = f->selectedFiles();
    }
    
    if (fileNames.size() < 1)
    {
		return "";
    }

	f->deleteLater();
	std::string filename = fileNames[0].toStdString();

	return filename;
}

inline bool checkFileIsValid(std::string filename, bool write)
{
	if (filename.length() == 0)
	{
		// no error message
		return false;
	}

	FILE *fp = fopen(filename.c_str(), (write ? "w" : "r"));
	std::string problem;

	if (fp == NULL)
	{
		if (errno == EACCES)
		{
			problem = "Permission denied";
		}
		else
		{
			problem = strerror(errno);
		}
	}
	
	if (fp != NULL)
	{
		fclose(fp);
	}
	
	if (problem.length())
	{
		QMessageBox msg;
		msg.setText("Failed to open file: " + QString::fromStdString(problem));
		msg.exec();
		return false;
	}
	
	return true;
}

#endif

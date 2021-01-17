// breathalyser
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

#ifndef __breathalyser__dictator__
#define __breathalyser__dictator__

#include <vector>
#include <QObject>

class Dictator : public QObject
{
Q_OBJECT
public:
	Dictator();
	
	void setArgs(std::vector<std::string> args)
	{
		_args = args;
	}
	
public slots:
	void run();
	void jobDone();
	void incrementJob();
protected:
	virtual bool processRequest(std::string first, std::string last) = 0;
	virtual void setup() {};

private:
	bool processNextArg(std::string arg);

	std::vector<std::string> _args;
	int _currentJob;
	QThread *_w;
};

#endif

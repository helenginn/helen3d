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
#include <map>

class Dictator : public QObject
{
Q_OBJECT
public:
	Dictator();

	bool prepareWorkForObject(QObject *object);
	
	void startThread();
	
	void setArgs(std::vector<std::string> args)
	{
		_args = args;
	}
	
	void addArg(std::string arg)
	{
		_args.push_back(arg);
	}
	
	static std::string valueForKey(std::string key)
	{
		return _properties[key];
	}

public slots:
	void run();
	void jobDone();
	void incrementJob();
protected:
	virtual bool processRequest(std::string first, std::string last) = 0;
	virtual void setup() {};
	virtual void help() = 0;
	virtual void finished() {};

	static std::map<std::string, std::string> _properties;
	QThread *_w;
private:
	bool processNextArg(std::string arg);

	std::vector<std::string> _args;
	int _currentJob;
};

#endif

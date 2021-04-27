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

#include "Dictator.h"
#include <iostream>
#include <QThread>
#include <QApplication>

std::map<std::string, std::string> Dictator::_properties;

void splitCommand(std::string command, std::string *first, std::string *last)
{
	size_t equal = command.find('=');
	if (equal == std::string::npos)
	{
		*first = command;
		*last = "";
		return;
	}

	*first = command.substr(0, equal);
	
	if (command.length() <= equal + 1)
	{
		*last = "";
		return;
	}

	*last = command.substr(equal + 1, std::string::npos);
}

Dictator::Dictator()
{
	_w = 0;
	_currentJob = -1;
}

void Dictator::jobDone()
{
	disconnect(_w, SIGNAL(finished()), this, SLOT(jobDone()));
	disconnect(this, SIGNAL(refine()), nullptr, nullptr);
	std::cout << "Done long job." << std::endl;
	incrementJob();
}

bool Dictator::processNextArg(std::string arg)
{
	std::cout << std::endl;
	std::cout << "Processing next arg: " << "\"" << arg << 
	"\"" << std::endl;
	std::string first, last;
	splitCommand(arg, &first, &last);

	bool should_continue = processRequest(first, last);
	
	return should_continue;
}

void Dictator::incrementJob()
{
	if (_args.size() == 0)
	{
		help();
		return;
	}

	bool result = true;
	
	while (result)
	{
		_currentJob++;
		
		if ((size_t)_currentJob >= _args.size())
		{
			finished();
			break;
		}

		result = processNextArg(_args[_currentJob]);
	}
}

void Dictator::run()
{
	setup();
	_currentJob = -1;
	incrementJob();
}

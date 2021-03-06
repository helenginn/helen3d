// Vagabond : bond-based macromolecular model refinement
// Copyright (C) 2017-2018 Helen Ginn
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

#ifndef __vagabond__RefinementStepSearch__
#define __vagabond__RefinementStepSearch__

#include <stdio.h>
#include "RefinementStrategy.h"

class RefinementStepSearch : public RefinementStrategy
{
public:
	RefinementStepSearch() : RefinementStrategy()
	{
		afterCycleFunction = NULL;
		afterCycleObject = NULL;
	};

	void setAfterCycleFunction(Getter function, void *evaluatedObject)
	{
		afterCycleFunction = function;
		afterCycleObject = evaluatedObject;
	}

	virtual void refine();

private:
	double minimizeParameter(int i, double *bestScore);
	double minimizeTwoParameters(int whichParam1, int whichParam2, double *bestScore);

	Getter afterCycleFunction;
	void *afterCycleObject;

};

#endif /* defined(__vagabond__RefinementStepSearch__) */

#include "car.h"
#include "roadfighter.h"
#include "misc.h"
#include "helper-class.h"
#include "framerate.h"
#include "soundmanager.h"

Car::Car()
{

}

Car::Car(char *n,double s)
:InteractiveObject(n)
{
	speed = s;
	myState = CAR_READY;
	active = no;
	objectType = "Car";
}

Car::~Car()
{

}

void Car::initImages()
{
	int i, destroyIndex = 0;
	for(i = 0; i < getNumFrames(); i++)
	{
		if(i >= CAR_DESTROY_FRAME_START && i <= CAR_DESTROY_FRAME_END)
		{
			destroyFrames[destroyIndex++] = frames[i];
		}
	}
}

void Car::setSpeed(double s)
{
	speed = s;
}

double Car::getSpeed()
{
	return speed;
}

void Car::incSpeed()
{
	if(getCarState() == CAR_RUNNING)
	{
		if(speed < 250)
		{
			speed += 1.0;
		}
	}
}

void Car::setCarState(carState s)
{
	myState = s;
}

carState Car::getCarState()
{
	return myState;
}

void Car::setCarType(carType c)
{
	myType = c;
}

carType Car::getCarType()
{
	return myType;
}

void Car::setLastX(double lX)
{
	lastX = lX;
}

double Car::getLastX()
{
	return lastX;
}

void Car::setLastY(double lY)
{
	lastY = lY;
}

double Car::getLastY()
{
	return lastY;
}

void Car::setSlideDirection(slideDirectionType sDir)
{
	slideDirection = sDir;
}

slideDirectionType Car::getSlideDirection()
{
	return slideDirection;
}

void Car::setActive(Logical isActive)
{
	active = isActive;
}

Logical Car::isActive()
{
	return active;
}

void Car::move()
{
	if(getCarState() == CAR_READY || getCarState() == CAR_RUNNING || getCarState() == CAR_SLIDING || getCarState() == CAR_DODGING)
	{
		int i;
		unsigned int deltaTime = FR->getTimeElapsed();
		double speedInMilliseconds = (speed / 1.25) / 1000; // pixels per millisecond
		double distanceTraveled = deltaTime * speedInMilliseconds; // distance = speed * time
		RoadFighterViewport *vp = (RoadFighterViewport*)VP;

		//top of car
		LineClass carTopLine(getXPosWC(), getYPosWC(),getXPosWC() + getWidth(), getYPosWC());

		//check left side
		for (i = vp->track->getNumTurns(); i >= 0; i--)
		{
			if(carTopLine.intersects(vp->track->leftSideBarrier[i]))
			{
				if(getCarState() == CAR_DODGING)
				{
					setCarState(CAR_RUNNING);
				}
				else
				{
					setXPosWC(getXPosWC() + 1 * deltaTime);
				}
			}
		}

		//check right side
		for (i = vp->track->getNumTurns(); i >= 0; i--)
		{
			if(carTopLine.intersects(vp->track->rightSideBarrier[i]))
			{
				if(getCarState() == CAR_DODGING)
				{
					setCarState(CAR_RUNNING);
				}
				else
				{
					setXPosWC(getXPosWC() - 1 * deltaTime);
				}
			}
		}

		if(deltaTime > 0.0)
		{
			if(getCarState() != CAR_READY)
			{
				double moveRate = distanceTraveled;//speed / 100.0;
				moveRate *= 1.3;
				//if(yPos < vp->getTotalPixLine() + 100)
				//Even not on screen, still have to prevent car from going outside road
				if(getOnScreen())
					yPos += moveRate;
			}
			//lprintf("timeelapsed = %lf, moverate = %lf\n", timeElapsed, moveRate);
		}
		if(getCarType() == TRUCK_CAR)
		{
			if(getOnScreen())
			{
				SM->play(ROADFIGHER_TRUCK, yes, yes);
			}
			else
			{
				SM->stop(ROADFIGHER_TRUCK);
			}
		}
	}

	//double moveRate = (1.25) * speed / 100.0;
	//setYPosWC(getYPosWC() + moveRate);
}

void Car::moveLeft(double howMuch)
{

	int i;
	RECT ex = getFullExtents();
	double timeElapsed = FR->getTimeElapsed() / 11;

	RectangleClass rect(ex.left - howMuch, ex.top, ex.right - howMuch, ex.bottom);
	RoadFighterViewport *vp = (RoadFighterViewport*)VP;

	for(i = 0; i < getHeight() / BOUNDARY_CELL_SIZE; i++)
	{
		if(vp->getTotalRows() - getCurRow() - i < 0)
		{
			return;
		}
	}

	for (i = vp->track->getNumTurns(); i >= 0; i--)
	{
		if(vp->track->leftSideBarrier[i].intersectsRect(rect))
		{
			destroy();
			return;
		}
	}
	setXPosWC(getXPosWC() - howMuch * timeElapsed);
}

void Car::moveRight(double howMuch)
{
	int i;
	RECT ex = getFullExtents();
	double timeElapsed = FR->getTimeElapsed() / 11;

	RectangleClass rect(ex.left + howMuch, ex.top, ex.right + howMuch, ex.bottom);
	RoadFighterViewport *vp = (RoadFighterViewport*)VP;

	for(i = 0; i < getHeight() / BOUNDARY_CELL_SIZE; i++)
	{
		if(vp->getTotalRows() - getCurRow() - i < 0)
		{
			return;
		}
	}

	for (i = vp->track->getNumTurns(); i >= 0; i--)
	{
		if(vp->track->rightSideBarrier[i].intersectsRect(rect))
		{
			destroy();
			return;
		}
	}
	setXPosWC(getXPosWC() + howMuch * timeElapsed);
}

void Car::slide()
{
	myState = CAR_SLIDING;
	lastX = getXPosWC();
}

void Car::sliding()
{
	switch(getSlideDirection())
	{
	case DIRECTION_LEFT:
		if(getXPosWC() > getLastX() - 10)
		{
			moveLeft(2);
		}
		else
		{
			setSlideDirection(DIRECTION_NONE);
			setCarState(CAR_RUNNING);
		}
		break;

	case DIRECTION_RIGHT:
		if(getXPosWC() < getLastX() + 10)
		{
			moveRight(2);
		}
		else
		{
			setSlideDirection(DIRECTION_NONE);
			setCarState(CAR_RUNNING);
		}
		break;
	}
}

Logical Car::isSliding()
{
	return myState == CAR_SLIDING;
}

void Car::destroy()
{
	active = no;
	myState = CAR_DESTROYING;
}

void Car::destroying()
{
	static int count = -1;
	static Timer frameTimer;
	static Logical timerInitialized = no;

	frameTimer.forceTickBasedTimer();
	if(!timerInitialized)
	{
		frameTimer.start();
		timerInitialized = yes;
	}

	int timeElapsed = frameTimer.getTicks();

	if(count == -1)
	{
		count = 0;
	}

	//Stop the player car
	speed = 0.0;

	if(count == 0 && timeElapsed > 100 || timeElapsed > 300)
	{
		timerInitialized = no;
		lprintf("destroying\n");
		if(count < CAR_DESTROY_FRAME_END - CAR_DESTROY_FRAME_START + 1)
		{
			currentFrame = destroyFrames[count++];
		}
		else
		{
			myState = CAR_DESTROYED;
			count = -1;
		}
	}
}

Logical Car::isRivalCar()
{
	if(getCarType() == RED_CAR || getCarType() == BLUE_CAR || getCarType() == BLUEFIGHTER_CAR || getCarType() == BLUEAGGRESSIVE_CAR)
		return yes;
	else
		return no;
}

void Car::spawnAt(double x, double y)
{
	active = yes;
	state = ALIVE;
	currentFrame = frames[CAR_ALIVE_FRAME];
	setXPosWC(x);
	setYPosWC(y);
}

#include "StudentWorld.h"
#include "GameConstants.h"
#include <iostream>
#include <string>
#include <sstream>
#include <iomanip>
#include <cmath>
using namespace std;


GameWorld* createStudentWorld(string assetPath)
{
	return new StudentWorld(assetPath);
}



StudentWorld::StudentWorld(string assetPath)
: GameWorld(assetPath), m_nCitizens(0), m_nZombies(0), levelFinished(false)
{
}

StudentWorld::~StudentWorld()
{
	cleanUp();
}

int StudentWorld::init()
{
	Level lev(assetPath());
	ostringstream oss;
	oss << "level0" << getLevel() << ".txt";
	Level::LoadResult result = lev.loadLevel(oss.str());
	if (result == Level::load_fail_file_not_found)
		return GWSTATUS_PLAYER_WON;
	else if (result == Level::load_fail_bad_format)
		return GWSTATUS_LEVEL_ERROR;
	else if (result == Level::load_success)
	{
		for (int i = 0; i < LEVEL_WIDTH; i++)
		{
			for (int j = 0; j < LEVEL_HEIGHT; j++)
			{
				Level::MazeEntry ge = lev.getContentsOf(i, j);
				switch (ge)
				{
				case Level::wall:
					li.push_back(new Wall(i*SPRITE_WIDTH, j*SPRITE_HEIGHT, this));
					break;
				case Level::player:
					penelope = new Penelope(i*SPRITE_WIDTH, j*SPRITE_HEIGHT, this);
					break;
				case Level::exit:
					li.push_back(new Exit(i*SPRITE_WIDTH, j*SPRITE_HEIGHT, this));
					break;
				case Level::citizen:
					m_nCitizens++;
					li.push_back(new Citizen(i*SPRITE_WIDTH, j*SPRITE_HEIGHT, this));
					break;
				case Level::dumb_zombie:
					li.push_back(new Dumbzombie(i*SPRITE_WIDTH, j*SPRITE_HEIGHT, this));
					m_nZombies++;
					break;
				case Level::smart_zombie:
					li.push_back(new Smartzombie(i*SPRITE_WIDTH, j*SPRITE_HEIGHT, this));
					m_nZombies++;
					break;
				case Level::pit:
					li.push_back(new Pit(i*SPRITE_WIDTH, j*SPRITE_HEIGHT, this));
					break;
				case Level::landmine_goodie:
					li.push_back(new LandmineGoodie(i*SPRITE_WIDTH, j*SPRITE_HEIGHT, this));
					break;
				case Level::vaccine_goodie:
					li.push_back(new VaccineGoodie(i*SPRITE_WIDTH, j*SPRITE_HEIGHT, this));
					break;
				case Level::gas_can_goodie:
					li.push_back(new GascanGoodie(i*SPRITE_WIDTH, j*SPRITE_HEIGHT, this));
					break;
				}
			}
		}
		return GWSTATUS_CONTINUE_GAME;
	}
	return GWSTATUS_CONTINUE_GAME;
}

int StudentWorld::move()
{
	penelope->doSomething();
	if (penelope->isDead())
	{
		playSound(SOUND_PLAYER_DIE);
		decLives();
		return GWSTATUS_PLAYER_DIED;
	}
	for (list<Actor*>::iterator p = li.begin(); p != li.end(); p++)
	{
		(*p)->doSomething();

		if (penelope->isDead())
		{
			playSound(SOUND_PLAYER_DIE);
			decLives();
			return GWSTATUS_PLAYER_DIED;
		}
		if (levelFinished)
		{
			playSound(SOUND_LEVEL_FINISHED);
			return GWSTATUS_FINISHED_LEVEL;
		}
	}
	
	list<Actor*>::iterator p = li.begin();
	while(p != li.end())
	{
		if ((*p)->isDead())
		{
			delete *p;
			p = li.erase(p);
			continue;
		}
		p++;
	}

	ostringstream oss;
	oss.fill('0');
	oss << "Score: ";
	if (getScore() >= 0)
	{
		oss << setw(6) << getScore();
	}
	else
	{
		oss << '-' << setw(5) << abs(getScore());
	}
	oss << "  Level: " << getLevel(); 
	oss << "  Lives: " << getLives();
	oss << "  Vacc: " << penelope->getNumVaccines();
	oss << "  Flames: " << penelope->getNumFlameCharges();
	oss << "  Mines: " << penelope->getNumLandmines();
	oss << "  Infected: " << penelope->infectionCount();
	setGameStatText(oss.str());
	return GWSTATUS_CONTINUE_GAME;
}

void StudentWorld::cleanUp()
{
	list<Actor*>::iterator p = li.begin();
	while (p != li.end())
	{
		delete *p;
		p = li.erase(p);
	}
	delete penelope;
	penelope = nullptr;
	m_nCitizens = 0;
	m_nZombies = 0;
	levelFinished = false;
}

void StudentWorld::addActor(Actor * a)
{
	li.push_back(a);
}

bool StudentWorld::isBlocked(double dest_x, double dest_y, Actor* itself)
{
	if (itself != penelope && abs(penelope->getX() - dest_x) <= 15 && abs(penelope->getY() - dest_y) <= 15)
		return true;

	for (list<Actor*>::iterator p = li.begin(); p != li.end(); p++)
	{
		if (*p == itself)
			continue;
		if ((*p)->blockMovement())
			if (abs((*p)->getX() - dest_x) <= 15 && abs((*p)->getY() - dest_y) <= 15)
				return true;
	}
	return false;
}

bool StudentWorld::isFlameBlockedAt(double x, double y)
{
	for (list<Actor*>::iterator p = li.begin(); p != li.end(); p++)
	{
		if ((*p)->blockFlame() && distance(x, y, (*p)->getX(), (*p)->getY()) <= 10)
		{
			return true;
		}
	}
	return false;
}


int StudentWorld::getnCitizens()
{
	return m_nCitizens;
}

int StudentWorld::getnZombies()
{
	return m_nZombies;
}

void StudentWorld::decrementCitizens()
{
	m_nCitizens--;
}

void StudentWorld::changenZombies(int n)
{
	m_nZombies += n;
}

void StudentWorld::recordLevelFinishedIfAllCitizensGone()
{
	levelFinished = true;
}

void StudentWorld::penelopePickupGoods(Goodie* g)
{
	penelope->pickUpGoodieIfAppropriate(g);
}

double StudentWorld::distanceWithPenelope(Actor* itself)
{
	return distance(itself, penelope);
}

double StudentWorld::distanceWithPenelope(double x, double y)
{
	return distance(x, y, penelope->getX(), penelope->getY());
}

bool StudentWorld::findNearestAgent(Actor* itself, Direction& dir1, Direction& dir2)
{
	dir1 = -1;
	dir2 = -1;
	double min_dist = distanceWithPenelope(itself);
	if (min_dist <= 80)
	{
		getDirTo(itself, penelope, dir1, dir2);
	}
	for (list<Actor*>::iterator p = li.begin(); p != li.end(); p++)
	{
		if ((*p)->needSave())
		{
			if (distance(itself, *p) < min_dist)
				getDirTo(itself, *p, dir1, dir2);
		}
	}
	if (min_dist <= 80)
		return true;
	return false;
}

void StudentWorld::findAgentToVomit(Actor* itself)
{

	Direction dir = itself->getDirection();
	double vomit_x = itself->getX();
	double vomit_y = itself->getY();
	switch (dir)
	{
	case GraphObject::up:
		vomit_y += SPRITE_HEIGHT;
		break;
	case GraphObject::down:
		vomit_y -= SPRITE_HEIGHT;
		break;
	case GraphObject::right:
		vomit_x += SPRITE_WIDTH;
		break;
	case GraphObject::left:
		vomit_x -= SPRITE_WIDTH;
		break;
	}
	if (distanceWithPenelope(vomit_x, vomit_y) <= 10)
	{
		int rand = randInt(0, 2);
		if (rand == 0)
		{
			li.push_back(new Vomit(vomit_x, vomit_y, this, dir));
			playSound(SOUND_ZOMBIE_VOMIT);
		}
	}
	for (list<Actor*>::iterator p = li.begin(); p != li.end(); p++)
	{
		if ((*p)->needSave() && distance(vomit_x, vomit_y, (*p)->getX(), (*p)->getY()) <= 10)
		{
			int rand = randInt(0, 2);
			if(rand == 0)
			{
				li.push_back(new Vomit(vomit_x, vomit_y, this, dir));
				playSound(SOUND_ZOMBIE_VOMIT);
			}
		}
	}
}

double StudentWorld::findNearestZombie(Actor* itself)
{
	double min_dist = 999;
	for (list<Actor*>::iterator p = li.begin(); p != li.end(); p++)
	{
		if ((*p)->canVomit())
		{
			double dist = distance(itself, *p);
			if (dist < min_dist)
				min_dist = dist;
		}
	}

	return min_dist;
}

double StudentWorld::findNearestZombie(double x, double y)
{
	double min_dist = 999;
	for (list<Actor*>::iterator p = li.begin(); p != li.end(); p++)
	{
		if ((*p)->canVomit())
		{
			double dist = distance(x, y, (*p)->getX(), (*p)->getY());
			if (dist < min_dist)
				min_dist = dist;
		}
	}

	return min_dist;
}


void StudentWorld::activateOnAppropriateActors(Actor* a)
{
	if (a != penelope && distanceWithPenelope(a) <= 10)
		a->activateIfAppropriate(penelope);
	for (list<Actor*>::iterator p = li.begin(); p != li.end(); p++)
	{
		if (a == *p)
			continue;
		if (distance(a, *p) <= 10)
		{
			a->activateIfAppropriate(*p);
		}
	}
}

void StudentWorld::getDirTo(Actor* itself, Actor* target, Direction& dir1, Direction& dir2)
{
	dir1 = -1;
	dir2 = -1;
	if (itself->getX() == target->getX())
	{
		if (itself->getY() < target->getY())
			dir1 = GraphObject::up;
		else
			dir1 = GraphObject::down;
	}
	else if (itself->getY() == target->getY())
	{
		if (itself->getX() < target->getX())
			dir2 = GraphObject::right;
		else
			dir2 = GraphObject::left;
	}
	else
	{
		if (itself->getY() < target->getY())
			dir1 = GraphObject::up;
		else
			dir1 = GraphObject::down;

		if (itself->getX() < target->getX())
			dir2 = GraphObject::right;
		else
			dir2 = GraphObject::left;
	}
}

void StudentWorld::getDirToPenelope(Actor* itself, Direction& dir1, Direction& dir2)
{
	getDirTo(itself, penelope, dir1, dir2);
}


void StudentWorld::createVaccineGoodie(double x, double y)
{
	if (distanceWithPenelope(x, y) <= 10)
		return;
	for (list<Actor*>::iterator p = li.begin(); p != li.end(); p++)
	{
		if (distance(x, y, (*p)->getX(), (*p)->getY()) <= 10)
		{
			return;
		}
	}
	li.push_back(new VaccineGoodie(x, y, this));
}

double StudentWorld::distance(Actor* p1, Actor* p2)
{
	return sqrt((p1->getX() - p2->getX()) * (p1->getX() - p2->getX()) + (p1->getY() - p2->getY()) * (p1->getY() - p2->getY()));
}

double StudentWorld::distance(double x1, double y1, double x2, double y2)
{
	return sqrt((x1 - x2)*(x1 - x2) + (y1 - y2)*(y1 - y2));
}
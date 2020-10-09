#ifndef STUDENTWORLD_H_
#define STUDENTWORLD_H_

#include "GameWorld.h"
#include <string>
#include <list>
#include "Level.h"
#include "Actor.h"

class Actor;
class Goodie;
class Penelope;
class Pit;

class StudentWorld : public GameWorld
{
public:
	StudentWorld(std::string assetPath);
	virtual ~StudentWorld();
	virtual int init();
	virtual int move();
	virtual void cleanUp();
	
	// Add an actor to the world.
	void addActor(Actor* a);

	// determine the ability to move
	bool isBlocked(double dest_x, double dest_y, Actor* itself);

	// Is creation of a flame blocked at the indicated location?
	bool isFlameBlockedAt(double x, double y);

	// get number of 
	int getnCitizens();
	int getnZombies();

	// change number of
	void decrementCitizens();
	void changenZombies(int n);

	// Indicate that the player has finished the level if all citizens
	  // are gone.
	void recordLevelFinishedIfAllCitizensGone();

	// make penelope pick up goods
	void penelopePickupGoods(Goodie* g);

	// determine the distance with penelope
	double distanceWithPenelope(Actor* itself);
	double distanceWithPenelope(double x, double y);

	// determine the direction to the nearest agent within 80 pixels
	bool findNearestAgent(Actor* itself, Direction& dir1, Direction& dir2);

	// determine the distance with nearest zombie
	double findNearestZombie(Actor* itself);
	double findNearestZombie(double x, double y);

	// used by zombie to determine whether it will vomit
	void findAgentToVomit(Actor* itself);

	// get direction to penelope
	void getDirToPenelope(Actor* itself, Direction& dir1, Direction& dir2);
	
	// create Vaccine Goodie around it somewhere it dies
	void createVaccineGoodie(double x, double y);

	// For each actor overlapping a, activate a if appropriate.
	void activateOnAppropriateActors(Actor* a);

private:
	// used by various public direction functions
	void getDirTo(Actor* itself, Actor* target, Direction& dir1, Direction& dir2);

	// compute the distance between two actors
	double distance(Actor* p1, Actor* p2);
	double distance(double x1, double y1, double x2, double y2);

	// private data members
	std::list<Actor*> li;
	Penelope* penelope;
	int m_nCitizens;
	int m_nZombies;
	bool levelFinished;
};

#endif // STUDENTWORLD_H_

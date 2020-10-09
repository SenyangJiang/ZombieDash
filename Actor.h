#ifndef ACTOR_H_
#define ACTOR_H_

#include "GraphObject.h"
#include "StudentWorld.h"

class StudentWorld;
class Goodie;

// base class for all objects in the game
class Actor : public GraphObject
{
public:
	Actor(int imageID, double startX, double startY, StudentWorld* sp, int startDirection = right, int depth = 0);
	// Get this actor's world
	StudentWorld* getWorld() const;
	// Action to perform for each tick.
	virtual void doSomething() = 0;
	// If this is an activated object, perform its effect on a (e.g., for an
	  // Exit have a use the exit).
	virtual void activateIfAppropriate(Actor* a);
	// Is this actor dead?
	virtual bool isDead() const;
	// Mark this actor as dead.
	virtual void setDead();
	// If this object can be infected by vomit, change its InfectionStatus
	virtual void changeInfectionStatus(bool b);
	// Does this object block agent movement?
	virtual bool blockMovement() const;
	// Does this object block flames?
	virtual bool blockFlame() const;
	// Is this object infected by Vomit?
	virtual bool isInfected() const;
	// Does this object need to be saved by Penelope?
	virtual bool needSave() const;
	// Can this object explode?
	virtual bool canExplode() const;
	// Can this object vomit?
	virtual bool canVomit() const;
	// Is this object hot?
	virtual bool isHot() const;
	// Can this object drop vaccine?
	virtual bool canDropVaccine() const;
	// Does this object attract citizens?
	virtual bool attractCitizens() const;
	// Does this object collectible?
	virtual bool collectible() const;
private:
	StudentWorld* m_studentWorld;
};

// base class for objects that have lives
class Haslife : public Actor
{
public:
	Haslife(int imageID, double startX, double startY, StudentWorld* sp, int startDirection = right, int depth = 0);
	// set this object to Dead
	virtual void setDead();
	// return true if this object is dead
	virtual bool isDead() const;
private:
	bool m_isDead;
};

class Movable : public Haslife
{
public:
	Movable(int imageID, double startX, double startY, StudentWorld* sp);
	// return true for every Movable object
	virtual bool blockMovement() const;
	// sets dest_x and dest_y according to dir and len
	void setDestination(double& dest_x, double& dest_y, Direction dir, int len);
};

class Agent : public Movable
{
public:
	Agent(int imageID, double startX, double startY, StudentWorld* sp);
	// Set infectionStatus to b
	virtual void changeInfectionStatus(bool b);
	// return whether this object is infected
	virtual bool isInfected() const;
	// return infectioncount
	int infectionCount();
	// set infectioncount to num
	void setInfectionCount(int num);
	// increase infectioncount by amount
	void increaseInfectionCount(int amount);
private:
	bool m_infectionStatus;
	int m_infectionCount;
};

class Wall : public Actor
{
public:
	Wall(double startX, double startY, StudentWorld* sp);
	// Wall does not do anything during a tick
	virtual void doSomething();
	// return true for every Wall object
	virtual bool blockMovement() const;
	// return true for every Wall object
	virtual bool blockFlame() const;
};

class Penelope : public Agent
{
public:
	Penelope(double startX, double startY, StudentWorld* sp);
	// Penelope perform action according to the key input
	virtual void doSomething();
	// pick up g
	void pickUpGoodieIfAppropriate(Goodie* g);
	// return true for penelope objects
	virtual bool attractCitizens() const;
	// Increase the number of vaccines the object has.
	void increaseVaccines(int amount);
	// Increase the number of flame charges the object has.
	void increaseFlameCharges(int amount);
	// Increase the number of landmines the object has.
	void increaseLandmines(int amount);
	// How many vaccines does the object have?
	int getNumVaccines() const;
	// How many flame charges does the object have?
	int getNumFlameCharges() const;
	// How many landmines does the object have?
	int getNumLandmines() const;
private:
	int m_nLandmines;
	int m_nFlames;
	int m_nVaccines;
};

class Citizen : public Agent
{
public:
	Citizen(double startX, double startY, StudentWorld* sp);
	// move according to distance and direction to penelope and zombie
	virtual void doSomething();
	// return true for Citizen objects
	virtual bool needSave() const;
private:
	// attempt move toward penelope
		// if no direction is possible, return false, otherwise move and return true
	bool MoveToPenelope();
	// move to get farthest away from zombie
	void MoveAwayFromZombie(double dist_z);
	bool m_paralyzed;
};

class Zombie : public Movable
{
public:
	Zombie(double startX, double smartY, StudentWorld* sp);
	// move according to its movement plan
		// if agent nearby, vomit
	virtual void doSomething();
	// return true for every Zombie object
	virtual bool canVomit() const;
private:
	// pure virtual function implemented differently between dumb and smart zombie
	virtual void pickNewDir() = 0;
	bool m_paralyzed;
	int m_movePlan;
};

class Dumbzombie : public Zombie
{
public:
	Dumbzombie(double startX, double startY, StudentWorld* sp);
	// return true for every DumbZombie Object
	virtual bool canDropVaccine() const;
private:
	// random picking directions
	virtual void pickNewDir();
};

class Smartzombie : public Zombie
{
public:
	Smartzombie(double startX, double startY, StudentWorld* sp);
private:
	// if agent nearby, pick direction to get closer
		// otherwise, randomly pick directions
	virtual void pickNewDir();
};


class Exit : public Actor
{
public:
	Exit(double startX, double startY, StudentWorld* sp);
	// exit all citizens overlapping with it
	virtual void activateIfAppropriate(Actor* a);
	// exit all citizens, then if there is no more citizens and
		// penelope overlaps with it, inform StudentWorld LevelComplete
	virtual void doSomething();
	// return true for every Exit object
	virtual bool blockFlame() const;
};

class Pit : public Actor
{
public:
	Pit(double startX, double startY, StudentWorld* sp);
	// call activateIfAppropriate to kill all agents and zombies on it
	virtual void doSomething();
	// kill all agents, zombies overlapping with it
	virtual void activateIfAppropriate(Actor* a);
};

class Goodie : public Haslife
{
public:
	Goodie(int imageID, double startX, double startY, StudentWorld* sp);
	// return true for every Goddie class
	virtual bool collectible() const;
	// if penelope overlap with it, it gives penelope certain utility
	virtual void activateIfAppropriate(Actor* a);
	// call activateIfAppropriate
	virtual void doSomething();
	// implemented differently to give penelope different utilities
	virtual void givePenelopeSomething(Penelope* p) = 0;
};

class VaccineGoodie : public Goodie
{
public:
	VaccineGoodie(double startX, double startY, StudentWorld* sp);
	// increase penelope's vaccine count by 1
	virtual void givePenelopeSomething(Penelope* p);
};

class LandmineGoodie : public Goodie
{
public:
	LandmineGoodie(double startX, double startY, StudentWorld* sp);
	// increase penelope's landmine count by 2
	virtual void givePenelopeSomething(Penelope* p);
};

class GascanGoodie : public Goodie
{
public:
	GascanGoodie(double startX, double startY, StudentWorld* sp);
	// increase penelope's flame count by 5
	virtual void givePenelopeSomething(Penelope* p);
};

class Projectile : public Haslife
{
public:
	Projectile(int imageID, double startX, double startY, StudentWorld* sp, int startDirection);
	// increase its life span, if reaches 2, set itself to dead
		// otherwise call activateIfAppropriate
	virtual void doSomething();
private:
	int m_lifespan;
};

class Vomit : public Projectile
{
public:
	Vomit(double startX, double startY, StudentWorld* sp, int startDirection);
	// infect agents overlapping with it
	virtual void activateIfAppropriate(Actor* a);
};


class Flame : public Projectile
{
public:
	Flame(double startX, double startY, StudentWorld* sp, int startDirection);
	// return true for every Flame object
	virtual bool isHot() const;
	// kill agents, zombies, goodies, explode landmines that overlaps with it 
	virtual void activateIfAppropriate(Actor* a);
};

class Landmine : public Haslife
{
public:
	Landmine(double startX, double startY, StudentWorld* sp);
	// if not active, reduce its safety tick
		// otherwise, call activateIfAppropriate
	virtual void doSomething();
	// if agents, zombies, flames overlaps with it, call explode
	virtual void activateIfAppropriate(Actor* a);
	// return true for every landmine object
	virtual bool canExplode() const;
	// create pit at the spot and create flames around it 
	void explode();
private:
	int m_safety_tick;
	bool m_active;
};
#endif // ACTOR_H_
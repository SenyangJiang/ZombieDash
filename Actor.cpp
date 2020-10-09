#include "Actor.h"
#include "StudentWorld.h"

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////base classes
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////

/*implementation for Primary base class derived from GraphObject: Actor*/
Actor::Actor(int imageID, double startX, double startY, StudentWorld* sp, int startDirection, int depth)
	: GraphObject(imageID, startX, startY, startDirection, depth), m_studentWorld(sp) {}

StudentWorld* Actor::getWorld() const
{
	return m_studentWorld;
}

void Actor::activateIfAppropriate(Actor * a)
{
	return;
}


bool Actor::needSave() const
{
	return false;
}

bool Actor::isDead() const
{
	return false;
}

void Actor::setDead()
{
	return;
}


bool Actor::blockMovement() const
{
	return false;
}

bool Actor::blockFlame() const
{
	return false;
}

bool Actor::canExplode() const
{
	return false;
}

void Actor::changeInfectionStatus(bool b)
{
	return;
}

bool Actor::isInfected() const
{
	return false;
}

bool Actor::canVomit() const
{
	return false;
}

bool Actor::isHot() const
{
	return false;
}

bool Actor::canDropVaccine() const
{
	return false;
}

bool Actor::attractCitizens() const
{
	return false;
}
bool Actor::collectible() const
{
	return false;
}
/*end of Actor*/

/*Second Level base class drived from Actor: Haslife*/
Haslife::Haslife(int imageID, double startX, double startY, StudentWorld* sp, int startDirection, int depth)
	: Actor(imageID, startX, startY, sp, startDirection, depth), m_isDead(false) {}

bool Haslife::isDead() const
{
	return m_isDead;
}
void Haslife::setDead()
{
	m_isDead = true;
}
/*end of Haslife*/

/*Third Level base class derived from Haslife: Movable*/
Movable::Movable(int imageID, double startX, double startY, StudentWorld* sp)
	: Haslife(imageID, startX, startY, sp) {}


bool Movable::blockMovement() const
{
	return true;
}

void Movable::setDestination(double& dest_x, double& dest_y, Direction dir, int len)
{
	switch (dir)
	{
	case up:
		dest_y += len;
		break;
	case down:
		dest_y -= len;
		break;
	case right:
		dest_x += len;
		break;
	case left:
		dest_x -= len;
		break;
	}
}

/*end of Movable*/


/*Fourth Level base class derived from Movable: Agent, Zombie*/

/*implementation for Agent*/
Agent::Agent(int imageID, double startX, double startY, StudentWorld* sp)
	: Movable(imageID, startX, startY, sp), m_infectionStatus(false), m_infectionCount(0) {}


void Agent::changeInfectionStatus(bool b)
{
	m_infectionStatus = b;
}

bool Agent::isInfected() const
{
	return m_infectionStatus;
}

int Agent::infectionCount()
{
	return m_infectionCount;
}

void Agent::setInfectionCount(int num)
{
	m_infectionCount = num;
}

void Agent::increaseInfectionCount(int amount)
{
	m_infectionCount += amount;
}
/*end of Agent*/

/*implementation for Zombie*/
Zombie::Zombie(double startX, double startY, StudentWorld* sp)
	: Movable(IID_ZOMBIE, startX, startY, sp), m_movePlan(0), m_paralyzed(false) {}


void Zombie::doSomething()
{
	if (isDead())
		return;
	else if (m_paralyzed)
	{
		m_paralyzed = false;
		return;
	}
	else
	{
		getWorld()->findAgentToVomit(this);
		if (m_movePlan == 0)
		{
			m_movePlan = randInt(3, 10);
			pickNewDir();
		}
		double dest_x = getX();
		double dest_y = getY();
		Direction dir = getDirection();
		setDestination(dest_x, dest_y, dir, 1);
		if (!getWorld()->isBlocked(dest_x, dest_y, this))
		{
			moveTo(dest_x, dest_y);
			m_movePlan--;
		}
		else
			m_movePlan = 0;
		m_paralyzed = true;
		return;
	}
}

bool Zombie::canVomit() const
{
	return true;
}
/*end of Zombie*/
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////// derived classes
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/*implementation for class derived from Actor: Wall and Pit*/

/*implementation for Wall*/
Wall::Wall(double startX, double startY, StudentWorld* sp)
	: Actor(IID_WALL, startX, startY, sp) {}

void Wall::doSomething()
{
	return;
}

bool Wall::blockMovement() const
{
	return true;
}
bool Wall::blockFlame() const
{
	return true;
}
/*end of Wall*/

/*implementation for Exit*/
Exit::Exit(double startX, double startY, StudentWorld* sp)
	: Actor(IID_EXIT, startX, startY, sp, right, 1) {}


void Exit::activateIfAppropriate(Actor * a)
{
	if(a->needSave() && !a->isDead())
	{
		getWorld()->increaseScore(500);
		a->setDead();
		getWorld()->playSound(SOUND_CITIZEN_SAVED);
		getWorld()->decrementCitizens();
	}
}

void Exit::doSomething()
{
	getWorld()->activateOnAppropriateActors(this);
	if (getWorld()->getnCitizens() == 0 && getWorld()->distanceWithPenelope(this) <= 10)
		getWorld()->recordLevelFinishedIfAllCitizensGone();
	return;
}
bool Exit::blockFlame() const
{
	return true;
}
/*end of Exit*/

/*implementation for Pit*/
Pit::Pit(double startX, double startY, StudentWorld* sp)
	: Actor(IID_PIT, startX, startY, sp) {}

void Pit::doSomething()
{
	getWorld()->activateOnAppropriateActors(this);
}

void Pit::activateIfAppropriate(Actor* a)
{
	if (a->isDead())
		return;
	if (a->attractCitizens())
		a->setDead();
	if (a->needSave())
	{
		getWorld()->playSound(SOUND_CITIZEN_DIE);
		getWorld()->increaseScore(-1000);
		getWorld()->decrementCitizens();
		a->setDead();
	}
	if (a->canVomit())
	{
		getWorld()->playSound(SOUND_ZOMBIE_DIE);
		if (a->canDropVaccine())
		{
			getWorld()->increaseScore(1000);
			getWorld()->changenZombies(-1);
			int i = randInt(1, 40);
			switch (i)
			{
			case 1:
				getWorld()->createVaccineGoodie(a->getX() + SPRITE_WIDTH, a->getY());
				break;
			case 2:
				getWorld()->createVaccineGoodie(a->getX() - SPRITE_WIDTH, a->getY());
				break;
			case 3:
				getWorld()->createVaccineGoodie(a->getX(), a->getY() + SPRITE_HEIGHT);
				break;
			case 4:
				getWorld()->createVaccineGoodie(a->getX(), a->getY() - SPRITE_HEIGHT);
				break;
			}
		}
		else
		{
			getWorld()->increaseScore(2000);
		}
		a->setDead();
		getWorld()->changenZombies(-1);
	}
}
/*classes derived from Zombie: Dumbzombie, SmartZombie*/

/*implementation for Dumbzombie*/
Dumbzombie::Dumbzombie(double startX, double startY, StudentWorld* sp)
	: Zombie(startX, startY, sp) {}

bool Dumbzombie::canDropVaccine() const
{
	return true;
}

void Dumbzombie::pickNewDir()
{
	int dir = randInt(0, 3);
	switch (dir)
	{
	case 0:
		setDirection(up);
		break;
	case 1:
		setDirection(down);
		break;
	case 2:
		setDirection(left);
		break;
	case 3:
		setDirection(right);
		break;
	}
}
/*end of Dumbzombie*/

/*implementation for Smartzombie*/
Smartzombie::Smartzombie(double startX, double startY, StudentWorld* sp)
	: Zombie(startX, startY, sp) {}

void Smartzombie::pickNewDir()
{
	Direction dir1;
	Direction dir2;
	if (getWorld()->findNearestAgent(this, dir1, dir2))
	{
		if (dir1 == -1)
		{
			setDirection(dir2);
		}
		else if (dir2 == -1)
		{
			setDirection(dir1);
		}
		else
		{
			int i = randInt(0, 1);
			if (i == 0)
				setDirection(dir1);
			else
				setDirection(dir2);
		}
	}
	else
	{
		int dir = randInt(0, 3);
		switch (dir)
		{
		case 0:
			setDirection(up);
			break;
		case 1:
			setDirection(down);
			break;
		case 2:
			setDirection(left);
			break;
		case 3:
			setDirection(right);
			break;
		}
	}
}
/*end of Smartzombie*/

/*classes derived from Agent: Penelope, Citizen*/

/*implementation for Penelope*/
Penelope::Penelope(double startX, double startY, StudentWorld* sp)
	: Agent(IID_PLAYER, startX, startY, sp), m_nFlames(0), m_nLandmines(0), m_nVaccines(0) {}

void Penelope::doSomething()
{
	if (isDead())
		return;
	if (isInfected())
	{
		if (infectionCount() == 500)
		{
			setDead();
			return;
		}
		else
		{
			increaseInfectionCount(1);
		}
	}
	int ch;
	if (getWorld()->getKey(ch))
	{
		double dest_X = getX();
		double dest_Y = getY();
		switch (ch)
		{
		case KEY_PRESS_SPACE:
			if (getNumFlameCharges() > 0)
			{
				increaseFlameCharges(-1);
				getWorld()->playSound(SOUND_PLAYER_FIRE);
				Direction dir = getDirection();
				double flame_x = getX();
				double flame_y = getY();
				for (int i = 1; i <= 3; i++)
				{
					switch (dir)
					{
					case up:
						flame_y += SPRITE_HEIGHT;
						break;
					case down:
						flame_y -= SPRITE_HEIGHT;
						break;
					case right:
						flame_x += SPRITE_WIDTH;
						break;
					case left:
						flame_x -= SPRITE_WIDTH;
						break;
					}
					if (getWorld()->isFlameBlockedAt(flame_x, flame_y))
						break;
					getWorld()->addActor(new Flame(flame_x, flame_y, getWorld(), dir));
				}
			}
			break;
		case KEY_PRESS_TAB:
			if (getNumLandmines() > 0)
			{
				increaseLandmines(-1);
				getWorld()->addActor(new Landmine(getX(), getY(), getWorld()));
			}
			break;
		case KEY_PRESS_ENTER:
			if (getNumVaccines() > 0)
			{
				increaseVaccines(-1);
				changeInfectionStatus(false);
				setInfectionCount(0);
			}
			break;
		case KEY_PRESS_LEFT:
			setDirection(left);
			setDestination(dest_X, dest_Y, getDirection(), 4);
			if (!getWorld()->isBlocked(dest_X, dest_Y, this))
				moveTo(dest_X, dest_Y);
			break;
		case KEY_PRESS_RIGHT:
			setDirection(right);
			setDestination(dest_X, dest_Y, getDirection(), 4);
			if (!getWorld()->isBlocked(dest_X, dest_Y, this))
				moveTo(dest_X, dest_Y);
			break;
		case KEY_PRESS_UP:
			setDirection(up);
			setDestination(dest_X, dest_Y, getDirection(), 4);
			if (!getWorld()->isBlocked(dest_X, dest_Y, this))
				moveTo(dest_X, dest_Y);
			break;
		case KEY_PRESS_DOWN:
			setDirection(down);
			setDestination(dest_X, dest_Y, getDirection(), 4);
			if (!getWorld()->isBlocked(dest_X, dest_Y, this))
				moveTo(dest_X, dest_Y);
			break;
		}
	}
}
void Penelope::pickUpGoodieIfAppropriate(Goodie * g)
{
	g->givePenelopeSomething(this);
}
bool Penelope::attractCitizens() const
{
	return true;
}

void Penelope::increaseVaccines(int amount)
{
	m_nVaccines += amount;
}
void Penelope::increaseFlameCharges(int amount)
{
	m_nFlames += amount;
}
void Penelope::increaseLandmines(int amount)
{
	m_nLandmines += amount;
}
int Penelope::getNumVaccines() const
{
	return m_nVaccines;
}
int Penelope::getNumFlameCharges() const
{
	return m_nFlames;
}
int Penelope::getNumLandmines() const
{
	return m_nLandmines;
}
/*end of Penelope*/

/*implementation for Citizen*/
Citizen::Citizen(double startX, double startY, StudentWorld* sp)
	: Agent(IID_CITIZEN, startX, startY, sp) {}

void Citizen::doSomething()
{
	if (isDead())
		return;
	if (isInfected())
	{
		if (infectionCount() == 500)
		{
			setDead();
			getWorld()->playSound(SOUND_ZOMBIE_BORN);
			getWorld()->increaseScore(-1000);
			getWorld()->decrementCitizens();
			getWorld()->changenZombies(1);
			int i = randInt(1, 10);
			if (i <= 7)
				getWorld()->addActor(new Dumbzombie(getX(), getY(), getWorld()));
			else
				getWorld()->addActor(new Smartzombie(getX(), getY(), getWorld()));
			return;
		}
		else
		{
			increaseInfectionCount(1);
		}
	}
	if (m_paralyzed)
	{
		m_paralyzed = false;
		return;
	}
	else
	{
		m_paralyzed = true;
		double dist_p = getWorld()->distanceWithPenelope(this);
		double dist_z = getWorld()->findNearestZombie(this);
		if ((getWorld()->getnZombies() == 0 || dist_p < dist_z )&& dist_p <= 80)
		{
			if (MoveToPenelope())
			{
				return;
			}
		}
		if (getWorld()->getnZombies() != 0 && dist_z <= 80)
		{
			MoveAwayFromZombie(dist_z);
		}
	}
}

bool Citizen::needSave() const
{
	return true;
}

bool Citizen::MoveToPenelope()
{
	Direction dir1 = -1;
	Direction dir2 = -1;
	getWorld()->getDirToPenelope(this, dir1, dir2);
	double dest_x = getX();
	double dest_y = getY();

	if (dir1 == -1)
	{
		setDestination(dest_x, dest_y, dir2, 2);
		if (!getWorld()->isBlocked(dest_x, dest_y, this))
		{
			setDirection(dir2);
			moveTo(dest_x, dest_y);
			return true;
		}
		return false;
		
	}
	if (dir2 == -1)
	{
		setDestination(dest_x, dest_y, dir1, 2);
		if (!getWorld()->isBlocked(dest_x, dest_y, this))
		{
			setDirection(dir1);
			moveTo(dest_x, dest_y);
			return true;
		}
		return false;
		
	}
	int i = randInt(0, 1);
	if (i == 0)
	{
		setDestination(dest_x, dest_y, dir1, 2);
		if (!getWorld()->isBlocked(dest_x, dest_y, this))
		{
			setDirection(dir1);
			moveTo(dest_x, dest_y);
			return true;
		}
		else
		{
			dest_x = getX();
			dest_y = getY();
			setDestination(dest_x, dest_y, dir2, 2);
			if (!getWorld()->isBlocked(dest_x, dest_y, this))
			{
				setDirection(dir2);
				moveTo(dest_x, dest_y);
				return true;
			}
			return false;
		}
	}
	else
	{
		setDestination(dest_x, dest_y, dir2, 2);
		if (!getWorld()->isBlocked(dest_x, dest_y, this))
		{
			setDirection(dir2);
			moveTo(dest_x, dest_y);
			return true;
		}
		else
		{
			dest_x = getX();
			dest_y = getY();
			setDestination(dest_x, dest_y, dir1, 2);
			if (!getWorld()->isBlocked(dest_x, dest_y, this))
			{
				setDirection(dir1);
				moveTo(dest_x, dest_y);
				return true;
			}
			return false;
		}
	}
	return false;
}

void Citizen::MoveAwayFromZombie(double dist_z)
{
	double max_dist = -1;
	double curr_dist;
	Direction dir = 0;
	double curr_x = getX();
	double curr_y = getY();

	double dest_x = curr_x;
	double dest_y = curr_y;
	setDestination(dest_x, dest_y, up, 2);
	if (!getWorld()->isBlocked(dest_x, dest_y, this))
	{
		curr_dist = getWorld()->findNearestZombie(dest_x, dest_y);
		if (curr_dist > dist_z && curr_dist > max_dist)
		{
			max_dist = curr_dist;
			dir = up;
		}
	}

	dest_x = curr_x;
	dest_y = curr_y;
	setDestination(dest_x, dest_y, down, 2);
	if (!getWorld()->isBlocked(dest_x, dest_y, this))
	{
		curr_dist = getWorld()->findNearestZombie(dest_x, dest_y);
		if (curr_dist > dist_z && curr_dist > max_dist)
		{
			max_dist = curr_dist;
			dir = down;
		}
	}

	dest_x = curr_x;
	dest_y = curr_y;
	setDestination(dest_x, dest_y, left, 2);
	if (!getWorld()->isBlocked(dest_x, dest_y, this))
	{
		curr_dist = getWorld()->findNearestZombie(dest_x, dest_y);
		if (curr_dist > dist_z && curr_dist > max_dist)
		{
			max_dist = curr_dist;
			dir = left;
		}
	}

	dest_x = curr_x;
	dest_y = curr_y;
	setDestination(dest_x, dest_y, right, 2);
	if (!getWorld()->isBlocked(dest_x, dest_y, this))
	{
		curr_dist = getWorld()->findNearestZombie(dest_x, dest_y);
		if (curr_dist > dist_z && curr_dist > max_dist)
		{
			max_dist = curr_dist;
			dir = right;
		}
	}

	if (max_dist != -1)
	{
		setDirection(dir);
		dest_x = curr_x;
		dest_y = curr_y;
		setDestination(dest_x, dest_y, dir, 2);
		moveTo(dest_x, dest_y);
	}
}
/*end of Citizen*/


// implementation for all goodies
Goodie::Goodie(int imageID, double startX, double startY, StudentWorld* sp)
	: Haslife(imageID, startX, startY, sp, right, 1) {}

bool Goodie::collectible() const
{
	return true;
}

void Goodie::activateIfAppropriate(Actor * a)
{
	if (a->attractCitizens())
	{
		getWorld()->increaseScore(50);
		getWorld()->penelopePickupGoods(this);
		setDead();
		getWorld()->playSound(SOUND_GOT_GOODIE);
	}
}

void Goodie::doSomething()
{
	getWorld()->activateOnAppropriateActors(this);
}

VaccineGoodie::VaccineGoodie(double startX, double startY, StudentWorld * sp)
	: Goodie(IID_VACCINE_GOODIE, startX, startY, sp) {}

void VaccineGoodie::givePenelopeSomething(Penelope* p)
{
	p->increaseVaccines(1);
}

LandmineGoodie::LandmineGoodie(double startX, double startY, StudentWorld * sp)
	: Goodie(IID_LANDMINE_GOODIE, startX, startY, sp) {}

void LandmineGoodie::givePenelopeSomething(Penelope* p)
{
	p->increaseLandmines(2);
}

GascanGoodie::GascanGoodie(double startX, double startY, StudentWorld * sp)
	: Goodie(IID_GAS_CAN_GOODIE, startX, startY, sp) {}

void GascanGoodie::givePenelopeSomething(Penelope* p)
{
	p->increaseFlameCharges(5);
}

Projectile::Projectile(int imageID, double startX, double startY, StudentWorld * sp, int startDirection)
	: Haslife(imageID, startX, startY, sp, startDirection), m_lifespan(0) {}

void Projectile::doSomething()
{
	if (isDead())
		return;
	m_lifespan++;
	if (m_lifespan == 2)
	{
		setDead();
		return;
	}
	getWorld()->activateOnAppropriateActors(this);
}
// implementation for Vomit
Vomit::Vomit(double startX, double startY, StudentWorld* sp, int startDirection)
	: Projectile(IID_VOMIT, startX, startY, sp, startDirection) {}

void Vomit::activateIfAppropriate(Actor * a)
{
	if (a->attractCitizens())
		a->changeInfectionStatus(true);
	if (a->needSave())
	{
		if (!a->isInfected())
		{
			getWorld()->playSound(SOUND_CITIZEN_INFECTED);
			a->changeInfectionStatus(true);
		}
	}
}


Flame::Flame(double startX, double startY, StudentWorld* sp, int startDirection)
	: Projectile(IID_FLAME, startX, startY, sp, startDirection) {}

bool Flame::isHot() const
{
	return true;
}

void Flame::activateIfAppropriate(Actor* a)
{
	if (a->isDead())
		return;
	if (a->attractCitizens() || a->collectible())
		a->setDead();
	if (a->canExplode())
		a->activateIfAppropriate(this);
	if (a->canVomit())
	{
		getWorld()->playSound(SOUND_ZOMBIE_DIE);
		getWorld()->changenZombies(-1);
		if (a->canDropVaccine())
		{
			getWorld()->increaseScore(1000);
			int i = randInt(1, 40);
			switch (i)
			{
			case 1:
				getWorld()->createVaccineGoodie(a->getX() + SPRITE_WIDTH, a->getY());
				break;
			case 2:
				getWorld()->createVaccineGoodie(a->getX() - SPRITE_WIDTH, a->getY());
				break;
			case 3:
				getWorld()->createVaccineGoodie(a->getX(), a->getY() + SPRITE_HEIGHT);
				break;
			case 4:
				getWorld()->createVaccineGoodie(a->getX(), a->getY() - SPRITE_HEIGHT);
				break;
			}
		}
		else
		{
			getWorld()->increaseScore(2000);
		}
		a->setDead();
	}
	if (a->needSave())
	{
		getWorld()->increaseScore(-1000);
		getWorld()->playSound(SOUND_CITIZEN_DIE);
		getWorld()->decrementCitizens();
		a->setDead();
	}
}

Landmine::Landmine(double startX, double startY, StudentWorld * sp)
	: Haslife(IID_LANDMINE ,startX, startY, sp, right, 1), m_safety_tick(30), m_active(false) {}

void Landmine::doSomething()
{
	if(isDead())
		return;
	if (!m_active)
	{
		m_safety_tick--;
		if (m_safety_tick == 0)
			m_active = true;
		return;
	}
	else
	{
		getWorld()->activateOnAppropriateActors(this);
	}
}

void Landmine::activateIfAppropriate(Actor * a)
{
	if (a->needSave() || a->attractCitizens() || a->canVomit() || a->isHot())
		explode();
}


bool Landmine::canExplode() const
{
	return true;
}

void Landmine::explode()
{
	setDead();
	getWorld()->playSound(SOUND_LANDMINE_EXPLODE);
	for (int i = -1; i <= 1; i++)
	{
		for (int j = -1; j <= 1; j++)
		{
			if (i == 0 && j == 0)
				continue;
			if(!getWorld()->isFlameBlockedAt(getX() + i * SPRITE_WIDTH, getY() + j * SPRITE_HEIGHT))
				getWorld()->addActor(new Flame(getX() + i * SPRITE_WIDTH, getY() + j * SPRITE_HEIGHT, getWorld(), up));
		}
	}
	getWorld()->addActor(new Pit(getX(), getY(), getWorld()));
}

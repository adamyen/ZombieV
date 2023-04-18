#include "BehaviorBot.hpp"
#include "System/GameWorld.hpp"
#include "Zombie.hpp"

BehaviorTree::BehaviorTree()
{}






BehaviorBot::BehaviorBot() :
    HunterBase(0, 0),
    m_target(nullptr)
{

}

BehaviorBot::BehaviorBot(float x, float y) :
    HunterBase(x, y),
    m_target(nullptr),
    m_getTargetCount(0)
{
    Light light;
    light.color = sf::Color(255, 127, 0);
    light.intensity = 1.0f;
    light.radius  = 0;
    _shootLight = GameRender::getLightEngine().addDurableLight(light);

}

void BehaviorBot::update(GameWorld& world)
{
    computeControls(world);

    _currentWeapon->update();
    _time += DT;

    if (_state == SHOOTING)
    {
        if (!_currentWeapon->fire(&world, this))
        {
            _changeState(IDLE);
            if (_currentWeapon->isMagEmpty())
            {
                _changeAnimation(_currentWeapon->getReloadAnimation(), false);
                _state = RELOADING;
            }
        }
    }

    if (_state == RELOADING && _currentAnimation.isDone())
    {
        _currentWeapon->reload();
        _state = IDLE;
    }

    _shootLight->radius = 0;
    if (_state == SHOOTING)
    {
        bool wait = _lastState==SHOOTING;
        _changeAnimation(_currentWeapon->getShootAnimation(), wait);
        _shootLight->radius = 350;
    }
    else if (_state == MOVING)
    {
        bool wait = !(_lastState==IDLE);
        _changeAnimation(_currentWeapon->getMoveAnimation(), wait);
    }
    else
    {
        _changeAnimation(_currentWeapon->getIdleAnimation());
    }

    _shootLight->position = _currentWeapon->getFireOutPosition(this);
    /*_flashlight->position = _shootLight->position;
    _littleLight->position = _shootLight->position;
    _flashlight->angle = getAngle()+PI;*/
}

void BehaviorBot::computeControls(GameWorld& world)
{
    if (m_target)
    {
        Vec2 vTarget(m_target->getCoord(), getCoord());
        Vec2 direction(cos(_angle), sin(_angle));
        Vec2 directionNormal(-direction.y, direction.x);

        float dist = vTarget.getNorm();
        float vx = vTarget.x/dist;
        float vy = vTarget.y/dist;

        float dot2 = vx*directionNormal.x + vy*directionNormal.y;
        float coeff = 0.25f;

        float absDot = std::abs(dot2);
        coeff *= absDot;

        // if (absDot<0.25f || dist < 100)
        // {
        //     if (dist < 300)
        //     {
        //         _changeState(SHOOTING);
        //         if (dist < 100)
        //         {
        //             _feetTime += DT;
        //             move(vx*_speed, vy*_speed);
        //         }
        //     }
        //     else
        //     {
        //         float speedFactor = 0.25f;
        //         move(-vx*_speed*speedFactor, -vy*_speed*speedFactor);
        //         _feetTime += DT*speedFactor;
        //     }
        // }
        // else
        // {
        //     _changeState(IDLE);
        // }

        rootSelector(&world, absDot, dist, vx, vy);

        _angle += dot2>0?-coeff:coeff;

        if (m_target->isDying())
            m_target = nullptr;
    }
    else
    {
        _changeState(IDLE);
        // getTarget(&world);
        findZombieTask(&world);
    }

    m_coord = getBodyCoord();
}

void BehaviorBot::getTarget(GameWorld* world)
{
    
}

void BehaviorBot::hit(WorldEntity* entity, GameWorld* gameWorld)
{
    switch(entity->getType())
    {
        case(EntityTypes::ZOMBIE) :
        {
            m_target = entity;
            break;
        }
        default:
            break;
    }
}

void BehaviorBot::initialize()
{
    //HunterBase::init();
}

void BehaviorBot::rootSelector(GameWorld* world, float absDot, float dist, float vx, float vy)
{
    bool firstSeq = targetAndKillSequence(absDot, dist, vx, vy);
    if (!firstSeq)
    {
        bool secondSeq = findAndKillSequence(world, vx, vy);
        if (!secondSeq)
        {
            idleTask();
        }
    }
}


// ======== Sequence ========

bool BehaviorBot::targetAndKillSequence(float absDot, float dist, float vx, float vy)
{
    bool isZombieTargeted = checkHasTargetTask();
    if (isZombieTargeted)
    {
        bool isZombieTooClose = checkIsTooCloseDecorator(absDot, dist, vx, vy);
        if (!isZombieTooClose)
        {
            bool isZombieTooFar = checkIsTooFarDecorator(absDot, dist, vx, vy);
            if (!isZombieTooFar)
            {
                return shootTask();
            }
        }
    }
    return false;
}

bool BehaviorBot::findAndKillSequence(GameWorld* world, float vx, float vy)
{
    bool foundZombie = findZombieTask(world);
    if (foundZombie)
    {
        bool isInShootingDistance = moveTowardTask(vx, vy);
        if (isInShootingDistance)
        {
            return shootTask();
        }
    }
    return false;
}


// ======== Decorator ========

bool BehaviorBot::checkIsTooCloseDecorator(float absDot, float dist, float vx, float vy)
{
    if (absDot<0.25f || dist < 100)
    {
        if (dist < 100)
        {
            scootTask(vx, vy);
            return false;
        }
    }
    return true;
}

bool BehaviorBot::checkIsTooFarDecorator(float absDot, float dist, float vx, float vy)
{
    if (absDot<0.25f || dist < 100)
    {
        if (dist > 300)
        {
            moveTowardTask(vx, vy);
            return false;
        }
    }
    return true;
}


// ======== Task ========

bool BehaviorBot::checkHasTargetTask()
{
    if (m_target)
        return true;
    else
        return false;
}

bool BehaviorBot::shootTask()
{
    _changeState(SHOOTING);
    return true;
}

bool BehaviorBot::scootTask(float vx, float vy)
{
    _feetTime += DT;
    move(vx*_speed, vy*_speed);
    return true;
}

bool BehaviorBot::moveTowardTask(float vx, float vy)
{
    float speedFactor = 0.25f;
    move(-vx*_speed*speedFactor, -vy*_speed*speedFactor);
    _feetTime += DT*speedFactor;
    return true;
}

bool BehaviorBot::findZombieTask(GameWorld* world)
{
    ++m_getTargetCount;
    Zombie* zombie = nullptr;
    Zombie* target = nullptr;
    float minDist  = -1;

    while (Zombie::getNext(zombie))
    {
        Vec2 v(zombie->getCoord(), getCoord());
        float dist = v.getNorm2();

        if (dist < minDist|| minDist < 0)
        {
            minDist = dist;
            target = zombie;
        }
    }

    if (target)
    {
        m_target = target;
    }
    return true;
}

bool BehaviorBot::idleTask()
{
    _changeState(IDLE);
    return true;
}

#include "Bot.hpp"
#include "System/GameWorld.hpp"
#include "Zombie.hpp"

Bot::Bot() :
    HunterBase(0, 0),
    m_target(nullptr)
{

}

Bot::Bot(float x, float y) :
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

void Bot::update(GameWorld& world)
{
    treeNodeIsEnemyTargeted(world);

    _currentWeapon->update();
    _time += DT;

    if (_state == SHOOTING)
    {
        if (!_currentWeapon->fire(&world, this))
        {
            actionIdle();
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

void Bot::treeNodeIsEnemyTargeted(GameWorld& world)
{
    if (m_target)
    {
        treeNodeIsEnemyInShootingDistance(world);
    }
    else
    {
        treeNodeIsThereEnemyOnField(world);
    }

    m_coord = getBodyCoord();
}

void Bot::treeNodeIsEnemyInShootingDistance(GameWorld& world)
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

    if (absDot<0.25f || dist < 100)
    {
        treeNodeIsEnemyTooClose(world, dist, vx, vy);
    }
    else
    {
        actionIdle();
    }

    _angle += dot2>0?-coeff:coeff;

    if (m_target->isDying())
        m_target = nullptr;
}

void Bot::treeNodeIsEnemyTooClose(GameWorld& world, float distance, float vx, float vy)
{
    if (distance < 300)
    {
        actionShoot();
        if (distance < 100)
            actionScoot(vx, vy);
    }
    else
    {
        actionMoveTowardEnemy(vx, vy);
    }
}

void Bot::treeNodeIsThereEnemyOnField(GameWorld& world)
{
    actionIdle();
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
}

void Bot::hit(WorldEntity* entity, GameWorld* gameWorld)
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

void Bot::actionIdle()
{
    _changeState(IDLE);
}

void Bot::actionShoot()
{
    _changeState(SHOOTING);
}

void Bot::actionScoot(float vx, float vy)
{
    _feetTime += DT;
    move(vx*_speed, vy*_speed);
}

void Bot::actionMoveTowardEnemy(float vx, float vy)
{
    float speedFactor = 0.25f;
    move(-vx*_speed*speedFactor, -vy*_speed*speedFactor);
    _feetTime += DT*speedFactor;
}

void Bot::initialize()
{
    //HunterBase::init();
}

// DecisionTreeNode::DecisionTreeNode()
// {}

Action::Action()
{}

void Action::actionIdle()
{}

void Action::actionShoot()
{}

void Action::actionScoot(float vx, float vy)
{}

void Action::actionMoveTowardEnemy(float vx, float vy)
{}


DecisionTree::DecisionTree()
{}

void DecisionTree::treeNodeIsEnemyTargeted(GameWorld& world)
{}

void DecisionTree::treeNodeIsEnemyInShootingDistance(GameWorld& world)
{}

void DecisionTree::treeNodeIsEnemyTooClose(GameWorld& world, float distance, float vx, float vy)
{}

void DecisionTree::treeNodeIsThereEnemyOnField(GameWorld& world)
{}

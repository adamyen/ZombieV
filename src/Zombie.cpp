#include "Zombie.hpp"
#include "System/GameWorld.hpp"
#include "System/GameRender.hpp"
#include "Props/Props.hpp"
#include "Bot.hpp"
#include "System/Utils.hpp"
#include "Hunter.hpp"


#include <iostream>

uint64_t      Zombie::_moveTextureID;
uint64_t      Zombie::_attackTextureID;
Animation   Zombie::_moveAnimation(3, 6, 288, 311, 17, 20);
Animation   Zombie::_attackAnimation(3, 3, 954/3, 882/3, 9, 20);

Zombie::Zombie() :
    StandardEntity()
{

}

Zombie::Zombie(float x, float y) :
    StandardEntity(x, y, 0.0f),
    _vertexArray(sf::VertexArray(sf::Quads, 4))
{
    _speed = 150;
    
    _life  = 100;
    
    _done  = false;

    _currentAnimation = _moveAnimation;

    _time = getRandUnder(100.0f);
    _type = EntityTypes::ZOMBIE;

    _currentState = IDLE;
    _marked = false;
    _target = ENTITY_NULL;
}

Zombie::Zombie(float x, float y, bool isBoss) :
    StandardEntity(x, y, 0.0f),
    _vertexArray(sf::VertexArray(sf::Quads, 4))
{
    
    if (isBoss) {
        _life = 1000;
        _speed = 300;
    } else {
        _life  = 100;
        _speed = 150;
    }
    _isBoss = isBoss;
    _done  = false;

    _currentAnimation = _moveAnimation;

    _time = getRandUnder(100.0f);
    _type = EntityTypes::ZOMBIE;

    _currentState = IDLE;
    _marked = false;
    _target = ENTITY_NULL;
}

Zombie::~Zombie()
{
}

void Zombie::kill(GameWorld* world)
{
    world->removeBody(m_bodyID);
    this->remove();
}

void Zombie::setTarget(EntityID target)
{
    _currentState = MOVING;
    _target = target;
}

void Zombie::update(GameWorld& world)
{
    _time += DT;

    if (_target)
    {
        WorldEntity* target = world.getEntityByID(_target);

        /*std::cout << target->getCoord().x << std::endl;
        std::cout << target->getCoord().y << std::endl;
        std::cout << m_coord.x << std::endl;
        std::cout << m_coord.y << std::endl;*/

        Vec2 vTarget(target->getCoord(), m_coord);
        Vec2 direction(cos(_angle), sin(_angle));
        Vec2 directionNormal(-direction.y, direction.x);

        /*std::cout << "end" << std::endl;*/

        float dist = vTarget.getNorm();
        float vx = vTarget.x/dist;
        float vy = vTarget.y/dist;

        float dot2 = vx*directionNormal.x + vy*directionNormal.y;
        float coeff = 0.04f;

        float absDot = std::abs(dot2);
        coeff *= absDot;

        _angle += dot2>0?-coeff:coeff;

        // Flocking force added here
        if (_isBoss == false && (flock || noTargetFlock)) {
            float maxForce = 30;
            float maxSpeed = 75;
            float maxAccel = 5;
            float kC = 0.3;
            float kA = 0.3;
            float kS = 0.4;
            int collideRadius = 30;

            std::tuple<Vec2, Vec2> nearbyPosVel = _getNearbyPosVel(m_coord, direction * 75, 1000, world._zombieEntities);
            Vec2 cohesionForce = std::get<0>(nearbyPosVel);
            Vec2 alignmentForce = std::get<1>(nearbyPosVel);

            // Cohesion force
            cohesionForce = cohesionForce - m_coord;
            cohesionForce = cohesionForce.normalize() * 75;
            cohesionForce = cohesionForce - (direction * 75);
            if (cohesionForce.length() > maxForce) {
                cohesionForce = cohesionForce.normalize() * maxForce;
            }

            // Alignment force
            alignmentForce = alignmentForce.normalize() * 75;
            alignmentForce = alignmentForce - (direction * 75);
            if (alignmentForce.length() > maxForce) {
                alignmentForce = alignmentForce.normalize() * maxForce;
            }

            // Separation force
            Vec2 charFuturePos = m_coord + (direction * 75);
            bool collide = false;
            float minDistance = std::numeric_limits<float>::infinity();
            Vec2 separationForce;
            for (WorldEntity* zombieEntity : world._zombieEntities) {
                Vec2 targetDirection =  Vec2(cos(zombieEntity->_angle), sin(zombieEntity->_angle));
                Vec2 targetFuturePos = zombieEntity->m_coord + (targetDirection * 75);
                Vec2 posDiff = charFuturePos - targetFuturePos;
                if (posDiff.length() >= collideRadius) {
                    continue;
                }
                else if (zombieEntity->isDone()) {
                    continue;
                }
                else {
                    collide = true;
                    separationForce = separationForce + (posDiff * maxAccel/(collideRadius-posDiff.length()));
                }
            }
            if (separationForce.length() > maxForce) {
                separationForce = separationForce.normalize() * maxForce;
            }


            // Blending all three force
            Vec2 blends = cohesionForce * kC + alignmentForce * kA + separationForce * kS;

            if (noTargetFlock) {
                direction = direction + blends.normalize();
            }
            else {
                direction = direction * 2 + blends.normalize();
            }
        } 


        if (_currentState == MOVING)
        {
            float speed = 150;
            if (_isBoss) {
                speed = 300;
            }
            move(speed*direction.normalize().x, speed*direction.normalize().y);
        }
        else if (_currentAnimation.isDone())
        {
            if (dist < 3*CELL_SIZE)
            {
                // Need to create damage variable
                if (_isBoss) {
                    target->addLife(-50);
                } else {
                    target->addLife(-5);
                }
                world.addEntity(ExplosionProvider::getBase(target->getCoord()));
            }
        }

        if (target->isDying())
            _target = ENTITY_NULL;

    }
    else
    {
        _getTarget();
    }

    if (_life<0)
    {
        world.addEntity(ExplosionProvider::getBig(m_coord, true));
        world.addEntity(ExplosionProvider::getBigFast(m_coord));
        world.addEntity(ExplosionProvider::getBase(m_coord));
        _done = true;
    }

    if (_currentAnimation.isDone())
    {
        _currentAnimation = _moveAnimation;
        _currentState = MOVING;
    }

    m_coord = getBodyCoord();
}

void Zombie::render()
{
    if (GameRender::isVisible(this))
    {
        const Vec2& coord = m_coord;
        float x = coord.x;
        float y = coord.y;
        if (_isBoss) {
            GraphicUtils::initQuad(_vertexArray, sf::Vector2f(288, 311), sf::Vector2f(144, 155), SCALE*0.6);
        }
        else {
            GraphicUtils::initQuad(_vertexArray, sf::Vector2f(288, 311), sf::Vector2f(144, 155), SCALE*0.25);
        }
        GraphicUtils::transform(_vertexArray, sf::Vector2f(x, y), _angle);

        _currentAnimation.applyOnQuad(_vertexArray, _time);

        GameRender::addQuad(_currentAnimation.getTexture(), _vertexArray, RenderLayer::RENDER);
        GameRender::addShadowCaster(getCoord(), CELL_SIZE);
        GraphicUtils::createEntityShadow(this);
    }
}

void Zombie::initialize()
{
    _moveAnimation.setCenter(sf::Vector2f(90, 168));
    _moveTextureID = GameRender::registerTexture("data/textures/zombie/zombie_move.png");
    _attackTextureID = GameRender::registerTexture("data/textures/zombie/zombie_attack.png");

    _moveAnimation.setTextureID(_moveTextureID);
    _attackAnimation.setTextureID(_attackTextureID);
}

void Zombie::hit(WorldEntity* entity, GameWorld* gameWorld)
{
    switch(entity->getType())
    {
        case(EntityTypes::BULLET) :
        {
            Bullet* bullet = static_cast<Bullet*>(entity);
            const Vec2& pos(bullet->getCoord());
            float bulletAngle = bullet->getAngle();

            m_thisBody()->accelerate2D(bullet->getImpactForce());
            addLife(-bullet->getDamage());
            _time = getRandUnder(1000.0f);

            if (GameRender::isVisible(this))
            {
                gameWorld->addEntity(ExplosionProvider::getBase(pos));
                if (bullet->getDistance() < 50.0f)
                {
                    gameWorld->addEntity(ExplosionProvider::getClose(pos, bulletAngle));
                    gameWorld->addEntity(Guts::add(pos, bullet->getV()*40.f));
                }

                if (bullet->getPenetration() > -1.0f)
                {
                    gameWorld->addEntity(ExplosionProvider::getThrough(pos, bulletAngle, true));
                    gameWorld->addEntity(ExplosionProvider::getBigThrough(pos, bulletAngle));
                    gameWorld->addEntity(ExplosionProvider::getHit(pos, bulletAngle, true));
                }
                else
                {
                    gameWorld->addEntity(ExplosionProvider::getHit(pos, bulletAngle+PI, true));
                }
            }

            break;
        }
        case(EntityTypes::HUNTER) :
        {
            //_target = static_cast<Hunter*>(entity)->getGlobalIndex();
            if (_currentState != ATTACKING)
            {
                _currentState     = ATTACKING;
                _currentAnimation = _attackAnimation;
                _currentAnimation.resetTime(_time);
            }
            break;
        }
        default:
        {
            break;
        }
    }
}

void Zombie::initPhysics(GameWorld* world)
{
    m_initBody(world);
}

std::tuple< Vec2, Vec2 > Zombie::_getNearbyPosVel(Vec2 currPos, Vec2 currVel, int range, std::list<WorldEntity*> zombieEntities)
{
    Vec2 sumNearbyPos;
    Vec2 sumNearbyVel;
    int nearbyCount = 0;
    
    for (WorldEntity* zombieEntity : zombieEntities) {
        Vec2 zPos = zombieEntity->m_coord;
        Vec2 posDiff = zPos - currPos;
        if (posDiff.length() > range && zombieEntity->_id != bossId) {
            continue;
        }
        else if (zombieEntity->isDone()) {
            continue;
        }
        else {
            if (zombieEntity->_id == bossId && bossId != 8055) {
                nearbyCount += 10;
                sumNearbyPos = sumNearbyPos + zPos * 10;
                Vec2 direction = Vec2(cos(zombieEntity->_angle), sin(zombieEntity->_angle));
                sumNearbyVel = sumNearbyVel + (direction * 750);
            } else {
                nearbyCount += 1;
                sumNearbyPos = sumNearbyPos + zPos;
                Vec2 direction = Vec2(cos(zombieEntity->_angle), sin(zombieEntity->_angle));
                sumNearbyVel = sumNearbyVel + (direction * 75);
            }
        }
    }

    if (nearbyCount == 0) {
        return { sumNearbyPos, sumNearbyVel };
    }
    else {
        return { sumNearbyPos / nearbyCount, sumNearbyVel / nearbyCount };
    }
}

void Zombie::_getTarget()
{
    Hunter*  hunter = nullptr;
    EntityID target = ENTITY_NULL;
    float minDist  = -1;

    while (Hunter::getNext(hunter))
    {
        Vec2 v(hunter->getCoord(), m_coord);
        float dist = v.getNorm2();

        if ((dist < minDist|| minDist < 0) && !hunter->isDying())
        {
            minDist = dist;
            target = hunter->getID();
        }
    }

    /*Bot* b = nullptr;
    while (Bot::getNext(b))
    {
        Vec2 v(b->getCoord(), getCoord());
        float dist = v.getNorm2();

        if ((dist < minDist|| minDist < 0) && !b->isDying())
        {
            minDist = dist;
            target = b->getGlobalIndex();
        }
    }*/


    if (target)
    {
        setTarget(target);
    }
}





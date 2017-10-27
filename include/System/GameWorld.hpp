#ifndef GAMEWORLD_HPP_INCLUDED
#define GAMEWORLD_HPP_INCLUDED

#include <list>

#include "Level.hpp"
#include "UnitedEngine/UnitedEngine.h"
#include "EventManager.hpp"
#include "Config.hpp"
#include "GameRender.hpp"
#include "SoundPlayer.hpp"
#include "LightEngine/LightEngine.hpp"

typedef WorldEntity*(*AccessFunc)(size_t);

class GameWorld
{
public:
    GameWorld();
    void initEventHandler(sf::RenderWindow& window);

    U_2DConstraint* addConstraint(BodyID body1, BodyID body2, float length=0.0);
    void addEntity(WorldEntity* entity);
    void removeBody(BodyID id);
    void removeConstraint(U_2DConstraint* constraint);
    void update();
    void render();

    bool                isInLevelBounds(const Vec2& coord) {return _level.isInBounds(coord);}
    size_t              registerEntityClass(AccessFunc func);
    BodyID              addBody();
    GridCell*           getBodiesAt(const Vec2& coord);
    WorldEntity*        getEntityByID(EntityID id);
    const Level&        getLevel()  const {return _level;}
    const EventManager& getEvents() const {return _eventManager;}

    static U_2DBody* getBodyByID(BodyID id);

private:
    float _dt;
    Level _level;

    std::list<WorldEntity*> _entities;
    U_2DCollisionManager    _phyManager;
    EventManager            _eventManager;

    void _cleanEntities();

    std::vector<AccessFunc> m_accessFuncs;
};

#endif // GAMEWORLD_HPP_INCLUDED

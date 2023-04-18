#ifndef BEHAVIORBOT_HPP_INCLUDED
#define BEHAVIORBOT_HPP_INCLUDED

#include "HunterBase.hpp"
#include "Task.hpp"

class BehaviorTree
{
public:
    BehaviorTree();

    void rootSelector(GameWorld* world, float absDot, float dist, float vx, float vy);

    bool targetAndKillSequence(float absDot, float dist, float vx, float vy);
    bool findAndKillSequence(GameWorld* world, float vx, float vy);

    bool checkIsTooCloseDecorator(float absDot, float dist, float vx, float vy);
    bool checkIsTooFarDecorator(float absDot, float dist, float vx, float vy);

    bool findZombieTask(GameWorld* world);
    bool checkHasTargetTask();
    bool scootTask(float vx, float vy);
    bool moveTowardTask(float vx, float vy);
    bool shootTask();
    bool idleTask();
};

class BehaviorBot : public HunterBase, public WorldEntityPool<BehaviorBot>, public BehaviorTree
{
public:
    BehaviorBot();
    BehaviorBot(float x, float y);

    void hit(WorldEntity* entity, GameWorld* gameWorld);
    void setTarget(WorldEntity* entity) {m_target = entity;}
    void update(GameWorld& world);
    void run();

    static void initialize();

private:
    WorldEntity* m_target;
    size_t m_getTargetCount;


    void rootSelector(GameWorld* world, float absDot, float dist, float vx, float vy);

    bool targetAndKillSequence(float absDot, float dist, float vx, float vy);
    bool findAndKillSequence(GameWorld* world, float vx, float vy);

    bool checkIsTooCloseDecorator(float absDot, float dist, float vx, float vy);
    bool checkIsTooFarDecorator(float absDot, float dist, float vx, float vy);

    bool findZombieTask(GameWorld* world);
    bool checkHasTargetTask();
    bool scootTask(float vx, float vy);
    bool moveTowardTask(float vx, float vy);
    bool shootTask();
    bool idleTask();


    void computeControls(GameWorld& world);
    void getTarget(GameWorld* world);
};

#endif // BOT_HPP_INCLUDED
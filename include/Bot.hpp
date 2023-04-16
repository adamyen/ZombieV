#ifndef BOT_HPP_INCLUDED
#define BOT_HPP_INCLUDED

#include "HunterBase.hpp"


// class DecisionTreeNode
// {
// public:
//     DecisionTreeNode();
// };

class Action
{
public:
    Action();

    void actionIdle();
    void actionShoot();
    void actionScoot(float vx, float vy);
    void actionMoveTowardEnemy(float vx, float vy);
};

class DecisionTree
{
public:
    DecisionTree();

    void treeNodeIsEnemyTargeted(GameWorld& world);
    void treeNodeIsEnemyInShootingDistance(GameWorld& world);
    void treeNodeIsEnemyTooClose(GameWorld& world, float distance, float vx, float vy);
};

class Bot : public HunterBase, public WorldEntityPool<Bot>, public DecisionTree, public Action
{
public:
    Bot();
    Bot(float x, float y);

    void hit(WorldEntity* entity, GameWorld* gameWorld);
    void setTarget(WorldEntity* entity) {m_target = entity;}
    void update(GameWorld& world);

    static void initialize();

private:
    WorldEntity* m_target;
    size_t m_getTargetCount;

    void getTarget(GameWorld* world);
    void treeNodeIsEnemyTargeted(GameWorld& world);
    void treeNodeIsEnemyInShootingDistance(GameWorld& world);
    void treeNodeIsEnemyTooClose(GameWorld& world, float distance, float vx, float vy);
    void actionIdle();
    void actionShoot();
    void actionScoot(float vx, float vy);
    void actionMoveTowardEnemy(float vx, float vy);
};

#endif // BOT_HPP_INCLUDED

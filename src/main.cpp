#include <iostream>
#include <SFML/Graphics.hpp>

#include "System/GameWorld.hpp"
#include "System/GameRender.hpp"

#include "Bot.hpp"
#include "BehaviorBot.hpp"
#include "Props/Props.hpp"
#include "Turret.hpp"
#include "Hunter.hpp"
#include "Zombie.hpp"

#define WIN_WIDTH 1600
#define WIN_HEIGHT 900

#include "System/Utils.hpp"

int main()
{
    sf::ContextSettings settings;
    settings.antialiasingLevel = 0;
    sf::RenderWindow window(sf::VideoMode(WIN_WIDTH, WIN_HEIGHT), "Zombie V", sf::Style::Default, settings);
    //window.setVerticalSyncEnabled(true);
    window.setFramerateLimit(60);

    GameRender::initialize(WIN_WIDTH, WIN_HEIGHT);
    GameWorld world;
    world.initEventHandler(window);

    Hunter::registerObject(&world);

	world.initializeWeapons();

    world.getPhyManager().setGravity(Vec2(0, 0));

    Hunter& h = *Hunter::newEntity(static_cast<float>(MAP_SIZE/2), static_cast<float>(MAP_SIZE/2));
    // world.addEntity(&h);

    int waveCount = 0;

    // Decision Tree
    Bot::registerObject(&world);
    Bot& bot1 = *Bot::newEntity(static_cast<float>(MAP_SIZE / 2 + rand() % 10), static_cast<float>(MAP_SIZE / 2 + rand() % 10));
    world.addEntity(&bot1);
    Bot& bot2 = *Bot::newEntity(static_cast<float>(MAP_SIZE / 2 + rand() % 10), static_cast<float>(MAP_SIZE / 2 + rand() % 10));
    world.addEntity(&bot2);

    // Behavior Tree
    // BehaviorBot& bot1 = *BehaviorBot::newEntity(static_cast<float>(MAP_SIZE / 2 + rand() % 10), static_cast<float>(MAP_SIZE / 2 + rand() % 10));
    // world.addEntity(&bot1);
    // BehaviorBot& bot2 = *BehaviorBot::newEntity(static_cast<float>(MAP_SIZE / 2 + rand() % 10), static_cast<float>(MAP_SIZE / 2 + rand() % 10));
    // world.addEntity(&bot2);
    



    // Zombie& bossZombie = *Zombie::newEntity(getRandUnder(static_cast<float>(MAP_SIZE)), getRandUnder(static_cast<float>(MAP_SIZE)));
    // std::cout << "---------bossZombie->getID() = " << bossZombie.getID() << "\n";
    // std::cout << "---------hunter->getID() = " << h.getID() << "\n";
    // EntityID target = h.getID();
    // bossZombie.setTarget(target);
    // world.addEntity(&bossZombie);

    Zombie* newZombie;

    // Boss zombie setup
    newZombie = Zombie::newEntity(static_cast<float>(MAP_SIZE/2 + 300), static_cast<float>(MAP_SIZE/2 + 10), true);
    EntityID target = bot1.getID();
    newZombie->setTarget(target);
    newZombie->flock = true;
    newZombie->secondTargetID = bot2.getID();
    world.addZombieEntity(newZombie);
    int bossWorldId = world._zombieEntities.back()->_id;

    // Normal zombie setup
    for (int i(100); i--;)
    {
        newZombie = Zombie::newEntity(getRandUnder(static_cast<float>(MAP_SIZE)), getRandUnder(static_cast<float>(MAP_SIZE)));
		newZombie->setTarget(target);
        newZombie->flock = true;
        // newZombie->noTargetFlock = true;
        newZombie->secondTargetID = bot2.getID();
        newZombie->_isBoss = false;
        newZombie->bossId = bossWorldId;
        world.addZombieEntity(newZombie);
    }

    for (int i(0); i<10; ++i)
    {
        Light light;
        light.position = Vec2(getRandUnder(2000.0f), getRandUnder(2000.0f));
        light.color    = sf::Color(rand()%255, rand()%255,rand()%255);
        light.radius   = getRandFloat(300.0f, 450.0f);
        GameRender::getLightEngine().addDurableLight(light);
    }

    int frameCount = 0;
    float ttime = 0;
    while (window.isOpen())
    {
        ++frameCount;
        if (Zombie::getObjectsCount() == 0)
        {
            ++waveCount;
            for (int i(waveCount*waveCount + 10); i--;)
            {
                Zombie* newZombie(Zombie::newEntity(getRandUnder(static_cast<float>(MAP_SIZE)), getRandUnder(static_cast<float>(MAP_SIZE))));
                //newZombie->setTarget(&(*Hunter::getObjects().front()));
                world.addZombieEntity(newZombie);
            }
        }

        sf::Event event;
        while (window.pollEvent(event))
        {
			switch (event.type)
			{
				case sf::Event::KeyPressed:
					if (event.key.code == sf::Keyboard::Escape) window.close();
				default:
                    break;
			}
        }

        sf::Clock clock;
        world.update();

		int upTime = clock.getElapsedTime().asMilliseconds();
        ttime += upTime;

        Vec2 p = bot1.getCoord();
        if (bot1.isDone()) {
            p = bot2.getCoord();
        }

		GameRender::setFocus({ p.x, p.y });

        GameRender::clear();

        world.render();
        GameRender::display(&window);

        window.display();
    }

    return 0;
}

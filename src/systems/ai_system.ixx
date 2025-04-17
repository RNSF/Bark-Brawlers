module;

#include "common.hpp"
#include <unordered_map>
#include <iostream>

export module AiSystem;
import ECS;
import BTNodes;
import BTree;
import Factories;
import Entity;
import Components;

namespace std {
    template<>
    struct hash<Entity> {
        size_t operator()(const Entity& entity) const {
            return std::hash<int>()(static_cast<int>(entity));
        }
    };
}

export class AISystem {
private:
    std::unordered_map<Entity, BTNode*> btrees;

public:
    ~AISystem() {
        for (auto& [entity, btree] : btrees) {
            delete btree;
        }
    }

    void destroyBTree(const Entity& entity) {
        delete btrees[entity];
        btrees.erase(entity);
        std::cout << "Deleted AI BTree!" << std::endl;
    }

    void update(float delta) {

        for (auto it = btrees.cbegin(), nextIt = it; it != btrees.cend(); it = nextIt){
            ++nextIt;
            if (!ecs.aiAgents.has(it->first)) destroyBTree(it->first);
        }


        // Delete old b tree
        for (auto& [entity, btree] : btrees) {
            if (!ecs.aiAgents.has(entity)) {
                destroyBTree(entity);
            }
        }

        Entity p2 = getPlayer(2);
        Entity p1 = getPlayer(1);

        
        if (p1 != Entity::null()) {
            updateAIState(p1);
            updateInputs(delta, p1);
        }

        if (p2 != Entity::null()) {
            updateAIState(p2);
            updateInputs(delta, p2);
        }

        
    }

    void updateAIState(Entity entity) {
        if (ecs.players.get(entity).isDead) {
            ecs.playerInputs.get(entity).clear();
            if (btrees.count(entity)) {
                destroyBTree(entity);
            }
            return;
        }

        if (!btrees.count(entity) && ecs.aiAgents.has(entity)) {
            initBTree(entity, ecs.aiAgents.get(entity).class_id);
        }
        else if (btrees.count(entity) && !ecs.aiAgents.has(entity)) {
            ecs.playerInputs.get(entity).clear();
            destroyBTree(entity);
        }
    }

    void initBTree(Entity& ai, CLASS_ID class_id) {
        if (class_id == CLASS_ID::KNIGHT) {
            createKnightAI(ai);
        }
    }

    void updateInputs(float delta, Entity entity) {
        if (btrees.count(entity)) {
            auto& playerInput = ecs.playerInputs.get(entity);
            playerInput.release(INPUT_ID::ATTACK);
            playerInput.release(INPUT_ID::SPECIAL);
            btrees[entity]->execute(entity, delta);
        }
    }

    void createKnightAI(Entity& ai) {
        /*BTNode* basicAttack = new CooldownDecorator(3.0f, new InRangeDecorator(vec2(60, 20), new UseBasicAttack()));
        BTNode* chase = new CooldownDecorator(0.4f, new MoveTowardsPlayer());

        btrees[ai] = new Selector({
            new Sequence({ basicAttack }),
            new Sequence({ chase })
            });*/

        auto& aiAgent = ecs.aiAgents.get(ai);
        if (!aiAgent.isTutorial) {
            btrees[ai] = new Selector({
                new InDangerDecorator(
                       vec2(30, 20),
                       new Selector({
                            new UseDash(),
                            new Hop(),
                       })
                ),
                new ChooseRandom({
                    // Run at player
                    new DoUntil(new Selector({new InRangeDecorator(vec2(60, 20), new UseBasicAttack()), new Timeline({ new TargetOpponent(), new UpdatePath(), new RepeatDecorator(0.2f, new MoveAlongPath())})}), new Selector({new WasHurt(), new WasOpponentHurt()})),

                    // Evade player
                    new DoOnce({
                        new TargetRandom(),
                        new UpdatePath(),
                        new DoUntil(
                            new DoAll({
                                new InRangeDecorator(vec2(60, 20), new UseDash()),
                                new Timeline({
                                    new UpdatePath(),
                                    new RepeatDecorator(0.2f, new MoveAlongPath())
                                }),

                            }),
                            new Selector({new TargetReached(), new WasHurt()})
                        )
                    })
                })
            });
        } else {
            btrees[ai] = new Selector({

            new DoOnce({
                    new TargetRandom(),
                    new UpdatePath(),
                    new DoUntil(
                        new Timeline({
                                new UpdatePath(),
                                new RepeatDecorator(0.2f, new MoveAlongPath())
                         }),
                        new Selector({new TargetReached(), new WasHurt()})
                    )
                })
            });
        }
        
    }
};
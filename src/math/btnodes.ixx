module; 

#include <common.hpp>
#include <iostream>;
#include <chrono>;
#include <random>;
using namespace std::chrono;

export module BTNodes;

import BTree;
import ECS;
import Factories;
import Singletons;
import Pathing;
import RNG;
import Entity;
import Rect;
import WindowHelpers;



Entity getTargetPlayer(Entity aiEntity) {
    int aiPlayerId = ecs.players.get(aiEntity).id;
    return getPlayer(aiPlayerId == 1 ? 2 : 1);
}


export class Selector : public BTNode {
private:
    std::vector<BTNode*> children;
public:
    Selector(std::vector<BTNode*> nodes) : children(nodes) {}

    ~Selector() {
        for (auto& child : children) {
            delete child;
        }
    }

    bool execute(Entity& entity, float delta) override {
        for (auto& child : children) {
            if (child->execute(entity, delta)) return true;
        }
        return false;
    }
};

export class Sequence : public BTNode {
private:
    std::vector<BTNode*> children;
public:
    Sequence(std::vector<BTNode*> nodes) : children(nodes) {}

    ~Sequence() {
        for (auto& child : children) {
            delete child;
        }
    }

    bool execute(Entity& entity, float delta) override {
        for (auto& child : children) {
            if (!child->execute(entity, delta)) return false;
        }
        return true;
    }
};

export class DoAll : public BTNode {
private:
    std::vector<BTNode*> children;
public:
    DoAll(std::vector<BTNode*> nodes) : children(nodes) {}

    ~DoAll() {
        for (auto& child : children) {
            delete child;
        }
    }

    bool execute(Entity& entity, float delta) override {
        for (auto& child : children) {
            child->execute(entity, delta);
        }
        return true;
    }
};

export class DoOnce : public BTNode {
private:
    unsigned int index = 0;
    std::vector<BTNode*> children;
public:
    DoOnce(std::vector<BTNode*> nodes) : children(nodes) {}

    ~DoOnce() {
        for (auto& child : children) {
            delete child;
        }
    }

    bool execute(Entity& entity, float delta) override {
        while (index < children.size() && children[index]->execute(entity, delta)) {
            index++;
        }

        if (index >= children.size()) {
            index = 0;
            return true;
        }
        
        return false;
    }
};

export class ChooseRandom : public BTNode {
private:
    unsigned int index;
    std::vector<BTNode*> children;
public:
    ChooseRandom(std::vector<BTNode*> nodes) : children(nodes) {
        chooseNew();
    }

    ~ChooseRandom() {
        for (auto& child : children) {
            delete child;
        }
    }

    bool execute(Entity& entity, float delta) override {
        if (children[index]->execute(entity, delta)) chooseNew();
        return true;
    }

    void chooseNew() {
        index = randomIntRange(0, children.size() - 1);
    }
};

export class Timeline : public BTNode {
private:
    unsigned int index = 0;
    std::vector<BTNode*> children;
public:
    Timeline(std::vector<BTNode*> nodes) : children(nodes) {}

    ~Timeline() {
        for (auto& child : children) {
            delete child;
        }
    }

    bool execute(Entity& entity, float delta) override {
        while (children[index]->execute(entity, delta)) {
            index++;
            index = index % children.size();
        }
        return true;
    }
};

export class CooldownDecorator : public BTNode {
private:
    BTNode* child;
    float cooldownTime;
    float elapsedTime;

public:
    CooldownDecorator(float cooldown, BTNode* node) : cooldownTime(cooldown), child(node) {
        elapsedTime = 0.0f;
    }

    ~CooldownDecorator() {
        delete child;
    }

    bool execute(Entity& entity, float delta) override {
        elapsedTime += delta;

        if (elapsedTime >= cooldownTime) {
            if (child->execute(entity, delta)) {
                elapsedTime = 0.0f;
                return true;
            }
        }
        return false;
    }
};

export class DoUntil : public BTNode {
private:
    BTNode* action;
    BTNode* condition;

public:
    DoUntil(BTNode* actionNode, BTNode* conditionNode) : action(actionNode), condition(conditionNode) {
    }

    ~DoUntil() {
        delete action;
        delete condition;
    }

    bool execute(Entity& entity, float delta) override {
        if (condition->execute(entity, delta)) return true;
        action->execute(entity, delta);
        return false;
    }
};

export class WasHurt : public BTNode {
private:

public:
    WasHurt() {}

    ~WasHurt() {}

    bool execute(Entity& entity, float delta) override {
        return ecs.knockables.get(entity).isKnocked();
    }
};

export class WasOpponentHurt : public BTNode {
private:

public:
    WasOpponentHurt() {}

    ~WasOpponentHurt() {}

    bool execute(Entity& entity, float delta) override {
        return ecs.knockables.get(getTargetPlayer(entity)).isKnocked();
    }
};



export class TargetReached : public BTNode {
private:

public:
    TargetReached() {}

    ~TargetReached() {}

    bool execute(Entity& entity, float delta) override {
        auto& aiAgent = ecs.aiAgents.get(entity);
        return aiAgent.edgeIndex <= 0;
    }
};

export class RepeatDecorator : public BTNode {
private:
    BTNode* child;
    float repeatTime;
    float elapsedTime;

public:
    RepeatDecorator(float time, BTNode* node) : repeatTime(time), child(node) {
        elapsedTime = 0.0f;
    }

    ~RepeatDecorator() {
        delete child;
    }

    bool execute(Entity& entity, float delta) override {
        
        // std::cout << elapsedTime << std::endl;
        if (elapsedTime < repeatTime) {
            if (child->execute(entity, delta)) {
                elapsedTime += delta;
            }
        } else {
            elapsedTime = 0.0f;
            return true;
        }
        return false;
    }
};




export class InRangeDecorator : public BTNode {
public:
    BTNode* child;
    vec2 range;       

    InRangeDecorator(vec2 rangeDistance, BTNode* node)
        : range(rangeDistance), child(node) {
    }

    ~InRangeDecorator() {
        delete child;
    }

    bool execute(Entity& entity, float delta) override {
        if (inRange(entity, range)) {
            return child->execute(entity, delta);
        }
        return false;
    }

private:
    bool inRange(Entity& ai, vec2 rangeDistance) {
        Entity target = getTargetPlayer(ai);
        bool isFacingRight = ecs.players.get(ai).isFacingRight;

        vec2 ai_pos = ecs.transforms.get(ai).getTranslation();
        vec2 target_pos = ecs.transforms.get(target).getTranslation();

        float dx = ai_pos.x - target_pos.x;
        float dy = ai_pos.y - target_pos.y;

        if (std::abs(dy) > rangeDistance.y) return false;
        if (isFacingRight) {
            return dx < 5 && dx * -1 < rangeDistance.x;
        }
        else {
            return dx > -5 && dx < rangeDistance.x;
        }
    }
};


export class InDangerDecorator : public BTNode {
public:
    BTNode* child;
    vec2 range;

    InDangerDecorator(vec2 rangeDistance, BTNode* node)
        : range(rangeDistance), child(node) {
    }

    ~InDangerDecorator() {
        delete child;
    }

    bool execute(Entity& entity, float delta) override {
        if (inRange(entity, range)) {
            return child->execute(entity, delta);
        }
        return false;
    }

private:
    bool inRange(Entity& ai, vec2 rangeDistance) {
        
        bool isFacingRight = ecs.players.get(ai).isFacingRight;

        Vector2 aiPos = ecs.transforms.get(ai).getTranslation();

        Entity target = getTargetPlayer(ai);
        auto& targetPlayer = ecs.players.get(target);
        auto& targetWeapon = ecs.weapons.get(targetPlayer.weapon);
        auto& targetPlayerTransform = ecs.transforms.get(target);
        
        Rect detectionArea = Rect(-range.x, -range.y, range.x, range.y);
        detectionArea = detectionArea.translate(aiPos);

        Vector2 targetPosition = targetPlayerTransform.getTranslation();

        if (detectionArea.overlapsPoint(targetPosition)) {
            if (targetPlayer.class_id == CLASS_ID::ARCHER && targetWeapon.cooldownTime > 0.0f) {
                return true;
            }

            if (targetPlayer.class_id == CLASS_ID::MAGE && targetWeapon.attackTime > 0.0f) {
                return true;
            }
        }
        

        for (auto& entity : ecs.attacks.entities) {

            if (ecs.ignoredCollisionEntities.has(entity)) {
                auto& ignoredCollision = ecs.ignoredCollisionEntities.get(entity);
                if (std::find(std::begin(ignoredCollision.entities), std::end(ignoredCollision.entities), ai.id()) != std::end(ignoredCollision.entities)) continue;
            }

            auto& collisionBody = ecs.collisionBodies.get(entity);
           

            if (!(collisionBody.hitMask & (int)COLLISION_LAYER_ID::PLAYER_HURT)) continue;

            auto& transform = ecs.transforms.get(entity);

            Vector2 attackPosition = transform.getTranslation();

            if (detectionArea.overlapsPoint(aiPos)) {
                return true;
            }
            
        }

        return false;
    }
};


export class UseSpecialAttack : public BTNode {
public:
    bool execute(Entity& entity, float delta) override {
        //std::cout << "CPU: using special attack" << std::endl;
        return true;
    }
};

export class UseChargeAttack : public BTNode {
public:
    bool execute(Entity& entity, float delta) override {
        //std::cout << "CPU: using charge attack" << std::endl;
        return true;
    }
};

export class UseBasicAttack : public BTNode {
public:
    bool execute(Entity& entity, float delta) override {
        //std::cout << "CPU: using basic attack" << std::endl;
        auto& playerInput = ecs.playerInputs.get(entity);
        playerInput.press(INPUT_ID::ATTACK);

        auto& player = ecs.players.get(entity);

        // Special attack if possible
        if (player.isSpecialCharged && player.canDash()) {
            playerInput.press(INPUT_ID::SPECIAL);
        }

        if (ecs.weapons.get(ecs.players.get(entity).weapon).attackChargeAmount > 0.2) {
            playerInput.release(INPUT_ID::ATTACK);
        }

        return true;
    }
};

export class UseDash : public BTNode {
public:
    bool execute(Entity& entity, float delta) override {
        //std::cout << "CPU: using basic attack" << std::endl;
        auto& player = ecs.players.get(entity);
        auto& playerInput = ecs.playerInputs.get(entity);

        if (player.canDash()) {
            playerInput.release(INPUT_ID::LEFT);
            playerInput.release(INPUT_ID::RIGHT);
            if (rand() & 1) playerInput.press(INPUT_ID::LEFT);
            else            playerInput.press(INPUT_ID::RIGHT);
            playerInput.press(INPUT_ID::SPECIAL);
            return true;
        } else {
            playerInput.release(INPUT_ID::SPECIAL);
        }
        return false;
    }
};


export class Hop : public BTNode {
public:
    bool execute(Entity& entity, float delta) override {
        auto& player = ecs.players.get(entity);
        auto& playerInput = ecs.playerInputs.get(entity);

        playerInput.press(INPUT_ID::JUMP);

        return false;
    }
};

export class MoveTowardsPlayer : public BTNode {
public:
    bool execute(Entity& entity, float delta) override {
        Entity target = getTargetPlayer(entity);
        auto& playerInput = ecs.playerInputs.get(entity);
        playerInput.release(INPUT_ID::JUMP);
        playerInput.release(INPUT_ID::LEFT);
        playerInput.release(INPUT_ID::RIGHT);

        vec2 ai_pos = ecs.transforms.get(entity).getTranslation();
        vec2 target_pos = ecs.transforms.get(target).getTranslation();
        float dx = ai_pos.x - target_pos.x;
        float dy = ai_pos.y - target_pos.y;

        // Randomize movement
        static std::random_device rd;
        static std::mt19937 gen(rd());
        std::uniform_int_distribution<int> dist(0, 9);
        int rng = dist(gen);

        if (rng == 0) {
            playerInput.press(INPUT_ID::JUMP);
        }
        else if (rng < 8) {
            if (dx > 0) {
                playerInput.press(INPUT_ID::LEFT);
            }
            else {
                playerInput.press(INPUT_ID::RIGHT);
            }
            if (dy > 50) {
                playerInput.press(INPUT_ID::JUMP);
            }
        }
        else {
            if (dx < 0) {
                playerInput.press(INPUT_ID::LEFT);
            }
            else {
                playerInput.press(INPUT_ID::RIGHT);
            }
        }
        return true;
    }
};

export class TargetOpponent : public BTNode {
    bool execute(Entity& entity, float delta) override {

        Entity target = getTargetPlayer(entity);
        auto& targetTransform = ecs.transforms.get(target);
        auto& aiAgent = ecs.aiAgents.get(entity);
        aiAgent.targetPosition = targetTransform.getTranslation();
        
        // std::cout << "Target mouse position: " << aiAgent.targetPosition.x << ", " << aiAgent.targetPosition.y << std::endl;

        return true;
    }
};

export class TargetRandom : public BTNode {
    bool execute(Entity& entity, float delta) override {

        auto& pathingGraphEntity = ecs.pathingGraphs.entities.front();
        auto& pathingGraph = ecs.pathingGraphs.get(pathingGraphEntity);
        auto& pathingGraphTransform = ecs.transforms.get(pathingGraphEntity);

        int count = rand() % pathingGraph.walkableCoordinates.size();
        auto it = pathingGraph.walkableCoordinates.begin();
        for (int i = 0; i < count; i++) it++;
        int index = *it;

        auto& aiAgent = ecs.aiAgents.get(entity);
        aiAgent.targetPosition = pathingGraph.getNodePosition(pathingGraph.getCoordinate(index), pathingGraphTransform.getTranslation());

        // std::cout << "Target mouse position: " << aiAgent.targetPosition.x << ", " << aiAgent.targetPosition.y << std::endl;

        return true;
    }
};

export class TargetMouse : public BTNode {
    bool execute(Entity& entity, float delta) override {
        auto& transform = ecs.transforms.get(entity);
        auto& aiAgent = ecs.aiAgents.get(entity);
        aiAgent.targetPosition = defaultCamera.getTransform(WindowHelpers::getWindowSize(window)) * Vector2(mousePosX, mousePosY);

        // std::cout << "Target mouse position: " << aiAgent.targetPosition.x << ", " << aiAgent.targetPosition.y << std::endl;

        return true;
    }
};

export class UpdatePath : public BTNode {
    bool execute(Entity& entity, float delta) override {
        auto& transform = ecs.transforms.get(entity);
        auto& aiAgent = ecs.aiAgents.get(entity);

        auto& pathingGraphEntity = ecs.pathingGraphs.entities.front();
        auto& pathingGraph = ecs.pathingGraphs.get(pathingGraphEntity);
        auto& pathingGraphTransform = ecs.transforms.get(pathingGraphEntity);
        auto& collisionTracker = ecs.collisionTrackers.get(entity);


        // Is target above me?
        bool isTargetAbove = false;

        if (aiAgent.path.edges.size() > 0 && aiAgent.edgeIndex >= 0) {
            Vector2 dx = pathingGraph.getNodePosition(aiAgent.getCurrentEdge().coordinate, pathingGraphTransform.getTranslation()) - transform.getTranslation();
            isTargetAbove = dx.y < 0 && abs(dx.x) < 3;
        }

        // Update path
        if ((collisionTracker.flags & (int)COLLISION_TRACKER_FLAG::IS_ON_GROUND) || isTargetAbove) {

            Vector2 startPosition = transform.getTranslation() - pathingGraphTransform.getTranslation();
            // std::cout << startPosition.x << ", " << startPosition.y << std::endl;
            glm::ivec2 startCoord = pathingGraph.getClosestWalkableNode(pathingGraph.getNodeCoordinateAt(startPosition));

            Vector2 endPosition = aiAgent.targetPosition - pathingGraphTransform.getTranslation();
            glm::ivec2 endCoord = pathingGraph.getClosestWalkableNode(pathingGraph.getNodeCoordinateAt(endPosition));



            if (pathingGraph.walkableCoordinates.contains(pathingGraph.getIndex(startCoord)) && pathingGraph.walkableCoordinates.contains(pathingGraph.getIndex(endCoord)))
                findPath(pathingGraph, startCoord, endCoord, aiAgent.path);

            aiAgent.edgeIndex = aiAgent.path.edges.size() - 1;
        }

        return true;
    }
};


// Advanced Feature: [15] Advanced decision-making
// Implementation for an AI character to navigate a given path
export class MoveAlongPath : public BTNode {
public:
    bool execute(Entity& entity, float delta) override {
        auto& transform = ecs.transforms.get(entity);
        auto& aiAgent = ecs.aiAgents.get(entity);
        auto& pathingGraphEntity = ecs.pathingGraphs.entities.front();
        auto& pathingGraph = ecs.pathingGraphs.get(pathingGraphEntity);
        auto& pathingGraphTransform = ecs.transforms.get(pathingGraphEntity);
        auto& playerInput = ecs.playerInputs.get(entity);
        auto& collisionTracker = ecs.collisionTrackers.get(entity);
        playerInput.release(INPUT_ID::JUMP);
        playerInput.release(INPUT_ID::LEFT);
        playerInput.release(INPUT_ID::RIGHT);

        Vector2 currentPosition = transform.getTranslation();

        if (aiAgent.path.edges.size() > 0) {
            Vector2 targetPosition = pathingGraph.getNodePosition(aiAgent.getCurrentEdge().coordinate, pathingGraphTransform.getTranslation());
            Vector2 displacement = (targetPosition - currentPosition);
            
            
            // Get next targetPosition
            while (abs(displacement.x) < 3.0f && displacement.y <= 6 && displacement.y > -32 && aiAgent.edgeIndex > 0) {
                aiAgent.edgeIndex--;
                targetPosition = pathingGraph.getNodePosition(aiAgent.getCurrentEdge().coordinate, pathingGraphTransform.getTranslation());
                displacement = (targetPosition - currentPosition);
            };

            // Apply controls to reach targetPosition
            Vector2 dx = targetPosition - currentPosition;

            if (dx.x < -2) {
                playerInput.press(INPUT_ID::LEFT);
            } else if (dx.x > 2) {
                playerInput.press(INPUT_ID::RIGHT);
            }
            bool isOnGround = collisionTracker.flags & (int)COLLISION_TRACKER_FLAG::IS_ON_GROUND;

            bool isBelow = dx.y < -3;
            bool isBelowJumping = (dx.y < 6 && !isOnGround);
            bool isNotCloseEnough = !isOnGround && abs(dx.x) > dx.y;

            if (isBelow || isBelowJumping || isNotCloseEnough) {
                playerInput.press(INPUT_ID::JUMP);
            }

        }
       

        return true;
    }
};
module;

export module BTree;

import Entity;

export class BTNode {
public:
    virtual bool execute(Entity& entity, float delta) = 0;
    virtual ~BTNode() = default;
};
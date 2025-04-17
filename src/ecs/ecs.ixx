module;

#include <algorithm>
#include <vector>
#include <unordered_map>
#include <unordered_set>
#include <set>
#include <functional>
#include <typeindex>
#include <assert.h>
#include <iterator>
#include "common.hpp"


export module ECS;

import Components;
import Entity;
import Transform;



	// Common interface to refer to all containers in the ECS registry
export struct ContainerInterface {
	virtual void clear() = 0;
	virtual size_t size() = 0;
	virtual void remove(Entity e) = 0;
	virtual bool has(Entity entity) = 0;
	virtual const std::vector<Entity>& getEntities() const = 0;
};

// A container that stores components of type 'Component' and associated entities
export template <typename Component> // A component can be any class
class ComponentContainer : public ContainerInterface {
private:
	// The hash map from Entity -> array index.
	std::unordered_map<unsigned int, unsigned int> map_entity_componentID; // the entity is cast to uint to be hashable.
	bool registered = false;
public:
	// Container of all components of type 'Component'
	std::vector<Component> components;

	// The corresponding entities
	std::vector<Entity> entities;

	// Constructor that registers the type
	ComponentContainer() {
	}

	// Inserting a component c associated to entity e
	inline Component& insert(Entity e, Component c, bool check_for_duplicates = true) {
		// Usually, every entity should only have one instance of each component type
		assert(!(check_for_duplicates && has(e)) && "Entity already contained in ECS registry");

		map_entity_componentID[e] = (unsigned int)components.size();
		components.push_back(std::move(c)); // the move enforces move instead of copy constructor
		entities.push_back(e);
		return components.back();
	};

	// The emplace function takes the the provided arguments Args, creates a new object of type Component, and inserts it into the ECS system
	template<typename... Args>
	Component& emplace(Entity e, Args &&... args) {
		return insert(e, Component(std::forward<Args>(args)...));
	};
	template<typename... Args>
	Component& emplace_with_duplicates(Entity e, Args &&... args) {
		return insert(e, Component(std::forward<Args>(args)...), false);
	};

	// A wrapper to return the component of an entity
	Component& get(Entity e) {
		assert(has(e) && "Entity not contained in ECS registry");
		return components[map_entity_componentID[e]];
	}

	// Check if entity has a component of type 'Component'
	bool has(Entity entity) {
		return map_entity_componentID.count(entity) > 0;
	}

	// Remove an component and pack the container to re-use the empty space
	void remove(Entity e) {
		if (has(e))
		{
			// Get the current position
			int cID = map_entity_componentID[e];

			// Move the last element to position cID using the move operator
			// Note, components[cID] = components.back() would trigger the copy instead of move operator
			components[cID] = std::move(components.back());
			entities[cID] = entities.back(); // the entity is only a single index, copy it.
			map_entity_componentID[entities.back()] = cID;

			// Erase the old component and free its memory
			map_entity_componentID.erase(e);
			components.pop_back();
			entities.pop_back();
			// Note, one could mark the id for re-use
		}
	};

	// Remove all components of type 'Component'
	void clear() {
		map_entity_componentID.clear();
		components.clear();
		entities.clear();
	}

	// Report the number of components of type 'Component'
	size_t size() {
		return components.size();
	}

	// Sort the components and associated entity assignment structures by the comparisonFunction, see std::sort
	template <class Compare>
	void sort(Compare comparisonFunction) {
		// First sort the entity list as desired
		std::sort(entities.begin(), entities.end(), comparisonFunction);
		// Now re-arrange the components (Note, creates a new vector, which may be slow! Not sure if in-place could be faster: https://stackoverflow.com/questions/63703637/how-to-efficiently-permute-an-array-in-place-using-stdswap)
		std::vector<Component> components_new; components_new.reserve(components.size());
		std::transform(entities.begin(), entities.end(), std::back_inserter(components_new), [&](Entity e) { return std::move(get(e)); }); // note, the get still uses the old hash map (on purpose!)
		components = std::move(components_new); // note, we use move operations to not create unneccesary copies of objects, but memory is still allocated for the new vector
		// Fill the new hashmap
		for (unsigned int i = 0; i < entities.size(); i++)
			map_entity_componentID[entities[i]] = i;
	}

	const std::vector<Entity>& getEntities() const override {
		return entities;
	}
};


export class ECSRegistry {
	// callbacks to remove a particular or all entities in the system
	std::vector<ContainerInterface*> registry_list;

public:
	// Manually created list of all components this game has
	ComponentContainer<Transform>						transforms;
	ComponentContainer<Kinematic>						kinematics;
	ComponentContainer<Player>							players;
	ComponentContainer<PlayerInput>						playerInputs;
	ComponentContainer<Arena>							arenas;
	ComponentContainer<Tilemap<30,17>>					tilemaps;
	ComponentContainer<Tileset>							tilesets;
	ComponentContainer<CollisionBox>					collisionBoxes;
	ComponentContainer<CollisionPolygon>				collisionPolygons;
	ComponentContainer<CollisionBody>					collisionBodies;
	ComponentContainer<PlatformerController>			platformerControllers;
	ComponentContainer<CollisionTracker>				collisionTrackers;
	ComponentContainer<Weapon>							weapons;
	ComponentContainer<TransformParent>					transformParents;
	ComponentContainer<DeathTimer>						deathTimers;
	ComponentContainer<Attack>							attacks;
	ComponentContainer<EntityList<16>>					ignoredCollisionEntities;
	ComponentContainer<Health>							healths;
	ComponentContainer<AIAgent>							aiAgents;
	ComponentContainer<Dead>							dead;
	ComponentContainer<InstancedRender>					instancedRenders;
	ComponentContainer<ParticleEmitter>					particleEmitters;
	ComponentContainer<Text>							texts;
	ComponentContainer<Knockable>						knockables;
	ComponentContainer<Gravity>							gravities;
	ComponentContainer<Dash>							dashes;
	ComponentContainer<Arrow>							arrows;
	ComponentContainer<CharacterSelect> 			    characterSelects;
	ComponentContainer<PlayerOptions> 					playerOptions;
	ComponentContainer<Fireball>						fireballs;
	ComponentContainer<Explosion>						explosions;
	ComponentContainer<BackgroundParticle>				backgroundParticles;
	ComponentContainer<PathingGraph<TILEMAP_W, TILEMAP_H>> pathingGraphs;

	

	// constructor that adds all containers for looping over them
	ECSRegistry() {
		registry_list.push_back(&transforms);
		registry_list.push_back(&kinematics);
		registry_list.push_back(&players);
		registry_list.push_back(&playerInputs);
		registry_list.push_back(&arenas);
		registry_list.push_back(&tilemaps);
		registry_list.push_back(&tilesets);
		registry_list.push_back(&collisionBoxes);
		registry_list.push_back(&collisionPolygons);
		registry_list.push_back(&collisionBodies);
		registry_list.push_back(&platformerControllers);
		registry_list.push_back(&collisionTrackers);
		registry_list.push_back(&weapons);
		registry_list.push_back(&transformParents);
		registry_list.push_back(&deathTimers);
		registry_list.push_back(&attacks);
		registry_list.push_back(&ignoredCollisionEntities);
		registry_list.push_back(&healths);
		registry_list.push_back(&aiAgents);
		registry_list.push_back(&dead);
		registry_list.push_back(&instancedRenders);
		registry_list.push_back(&particleEmitters);
		registry_list.push_back(&texts);
		registry_list.push_back(&knockables);
		registry_list.push_back(&gravities);
		registry_list.push_back(&dashes);
		registry_list.push_back(&arrows);
		registry_list.push_back(&characterSelects);
		registry_list.push_back(&playerOptions);
		registry_list.push_back(&fireballs);
		registry_list.push_back(&explosions);
		registry_list.push_back(&backgroundParticles);
		registry_list.push_back(&pathingGraphs);
	}

	void clear_all_components() {
		for (ContainerInterface* reg : registry_list)
			reg->clear();
	}

	void list_all_components() {
		printf("Debug info on all registry entries:\n");
		for (ContainerInterface* reg : registry_list)
			if (reg->size() > 0)
				printf("%4d components of type %s\n", (int)reg->size(), typeid(*reg).name());
	}

	void list_all_components_of(Entity e) {
		printf("Debug info on components of entity %u:\n", (unsigned int)e);
		for (ContainerInterface* reg : registry_list)
			if (reg->has(e))
				printf("type %s\n", typeid(*reg).name());
	}

	void remove_all_components_of(Entity e) {
		for (ContainerInterface* reg : registry_list)
			reg->remove(e);
	}
};

export ECSRegistry ecs;
	





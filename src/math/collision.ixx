
module;

#include <optional>
#include <vector>
#include <cassert>
#include <iterator>
#include "common.hpp"
#include <iostream>

export module Collision;

import Rect;
import Entity;
import ECS;
import Components;
import Vector2;
import Polygon;

export {
	struct CollisionEvent {
		Rect rect;
		Entity entity;
	};


	Rect getAABB(Entity& entity) {
		assert(ecs.collisionBoxes.has(entity));
		auto& collisionBox = ecs.collisionBoxes.get(entity);
		Rect aabb = collisionBox.bounds;

		if (ecs.transforms.has(entity)) {
			auto& transform = ecs.transforms.get(entity);
			aabb = aabb.translate(transform.getTranslation()).scale(transform.getScale());
		}

		return aabb;
	}

	ConvexPolygon getTransformedPolygon(Entity& entity) {
		assert(ecs.collisionPolygons.has(entity));
		auto& collisionPolygon = ecs.collisionPolygons.get(entity);
		ConvexPolygon polygon = collisionPolygon.polygon();
		

		if (ecs.transforms.has(entity)) {
			auto& transform = ecs.transforms.get(entity);
			polygon.transform(transform);
		}

		return polygon;
	}

	

	

	// Detects for collision with CollisionBoxes and Tilemap in that order (no polygon).
	// ignores rotaion
	void getBoxCollisions(Entity& entity, std::vector<CollisionEvent>& output, unsigned int count = INT_MAX) {
		output.clear();
		if (count == 0) return;

		assert(ecs.collisionBoxes.has(entity));
		auto& collisionBody = ecs.collisionBodies.get(entity);

		EntityList<16> empty;
		auto& ignoredCollision = ecs.ignoredCollisionEntities.has(entity) ? ecs.ignoredCollisionEntities.get(entity) : empty;

		Rect aabb = getAABB(entity);


		// COLLISION WITH OTHER COLLISION BOXES
		// this can be optimized by pre filtering the items into different collision layers
		for (auto& otherEntity : ecs.collisionBoxes.entities) {
			if (otherEntity == entity) continue;
			if (std::find(std::begin(ignoredCollision.entities), std::end(ignoredCollision.entities), otherEntity.id()) != std::end(ignoredCollision.entities)) continue;
			
			auto& otherCollisionBody = ecs.collisionBodies.get(otherEntity);
			if (!(collisionBody.hitMask & otherCollisionBody.hurtMask)) continue;

			Rect otherAABB = getAABB(otherEntity);
			if (!aabb.overlaps(otherAABB)) continue;

			output.push_back(CollisionEvent(otherAABB, otherEntity));
			if (output.size() >= count) return;
		}

		// TILEMAP COLLISION
		// this can be optimized by pre filtering the items into different collision layers
		for (auto& tilemapEntity : ecs.tilemaps.entities) {
			if (tilemapEntity == entity) continue;
			if (std::find(std::begin(ignoredCollision.entities), std::end(ignoredCollision.entities), tilemapEntity.id()) != std::end(ignoredCollision.entities)) continue;

			auto& tilemap = ecs.tilemaps.get(tilemapEntity);
			auto& transform = ecs.transforms.get(tilemapEntity);
			if (!(collisionBody.hitMask & tilemap.hurtMask)) continue;

			Rect shiftedAABB = aabb.translate(-transform.getTranslation());
			glm::ivec2 topLeftTileCoord = tilemap.toTileSpace(shiftedAABB.topLeft()).floor().max(Vector2::zero());
			glm::ivec2 bottomRightTileCoord = tilemap.toTileSpace(shiftedAABB.bottomRight()).ceil().min({ (float)tilemap.width() , (float)tilemap.height() });



			for (int y = topLeftTileCoord.y; y < bottomRightTileCoord.y; y++) {
			for (int x = topLeftTileCoord.x; x < bottomRightTileCoord.x; x++) {
				if (tilemap.getTile({ x, y }).id) {
					output.push_back(CollisionEvent(tilemap.getTileRect({ x, y }).translate(transform.getTranslation()), tilemapEntity));
					if (output.size() >= count) return;
				}
			}}
		}
	}

	// Detects for collision with the CollisionBoxes and CollisionPolygons (in that order). No tilemap collision.
	// ignores rotation for collision boxes.
	void getPolygonCollisions(Entity& entity, std::vector<CollisionEvent>& output, unsigned int count = INT_MAX) {
		
		output.clear();
		if (count == 0) return;

		assert(ecs.collisionPolygons.has(entity));
		auto polygon = getTransformedPolygon(entity);
		auto& collisionBody = ecs.collisionBodies.get(entity);

		EntityList<16> empty;
		auto& ignoredCollision = ecs.ignoredCollisionEntities.has(entity) ? ecs.ignoredCollisionEntities.get(entity) : empty;

		// COLLISION WITH OTHER POLYGONS
		for (auto& otherEntity : ecs.collisionPolygons.entities) {
			if (otherEntity == entity) continue;
			if (std::find(std::begin(ignoredCollision.entities), std::end(ignoredCollision.entities), otherEntity.id()) != std::end(ignoredCollision.entities)) continue;

			auto& otherCollisionBody = ecs.collisionBodies.get(otherEntity);
			if (!(collisionBody.hitMask & otherCollisionBody.hurtMask)) continue;

			auto otherPolygon = getTransformedPolygon(otherEntity);
			if (!polygon.overlaps(otherPolygon)) continue;

			output.push_back(CollisionEvent(otherPolygon.getAABB(), otherEntity));
			if (output.size() >= count) return;
		}

		
		// COLLISION WITH OTHER COLLISION BOXES
		for (auto& otherEntity : ecs.collisionBoxes.entities) {
			if (otherEntity == entity) continue;
			if (std::find(std::begin(ignoredCollision.entities), std::end(ignoredCollision.entities), otherEntity.id()) != std::end(ignoredCollision.entities)) continue;

			auto& otherCollisionBody = ecs.collisionBodies.get(otherEntity);
			if (!(collisionBody.hitMask & otherCollisionBody.hurtMask)) continue;

			auto otherAABB = getAABB(otherEntity);
			auto otherPolygon = ConvexPolygon::fromRect(otherAABB);
			if (!polygon.overlaps(otherPolygon)) continue;

			output.push_back(CollisionEvent(otherAABB, otherEntity));
			if (output.size() >= count) return;
		}
	}

	std::optional<CollisionEvent> getFirstBoxCollision(Entity& entity) {
		std::vector<CollisionEvent> output;
		getBoxCollisions(entity, output, 1);
		if (output.size() > 0) return output[0];
		return std::nullopt;
	}

	std::optional<CollisionEvent> getFirstPolygonCollision(Entity& entity) {
		std::vector<CollisionEvent> output;
		getPolygonCollisions(entity, output, 1);
		if (output.size() > 0) return output[0];
		return std::nullopt;
	}



	
}
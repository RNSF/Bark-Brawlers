module;

#include <common.hpp>
#include <queue>
#include <iostream>

export module Pathing;
import Components;
import Entity;
import ECS;

export {


	struct FallCheck {
		unsigned int mag;		// distance away in x direction
		unsigned int minFallY;	// minimal fall distance
		unsigned int maxJumpY;	// maximal jump distance
		unsigned int minExtraJumpSpaceY; // extra space needed to clear the jump (player doesn't hit their head)
	};

	bool isSolidTile(glm::ivec2 pos) {

		for (auto& tilemap : ecs.tilemaps.components) {
			if (!(tilemap.hurtMask & (int)COLLISION_LAYER_ID::WALL)) continue;
			if (!tilemap.hasTile(pos)) continue;
			if (tilemap.getTile(pos).id == 0) continue;
			return true;
		}
		return false;

	}

	bool scanRect(glm::ivec2 topLeft, glm::ivec2 bottomRight) {
		for (unsigned int y = topLeft.y; y <= bottomRight.y; y++)
		for (unsigned int x = topLeft.x; x <= bottomRight.x; x++) {
			if (isSolidTile({ x, y})) return true;
		};

		return false;
	}

	bool scanRectUnordered(glm::ivec2 pos1, glm::ivec2 pos2) {
		glm::ivec2 topLeft = {std::min(pos1.x, pos2.x), std::min(pos1.y, pos2.y) };
		glm::ivec2 bottomRight = { std::max(pos1.x, pos2.x), std::max(pos1.y, pos2.y) };
		return scanRect(topLeft, bottomRight);
	}

	

	// Advanced Feature: [15] Advanced decision-making
	// Creation of the pathing graph based on the tilemap layout
	Entity createPathingGraph() {
		Entity pathingGraphEntity;


		auto& pathingGraph = ecs.pathingGraphs.emplace(pathingGraphEntity);
		pathingGraph.tileSize = Vector2::one() * 16;

		// Add walkable tiles
		for (unsigned int y = 1; y < TILEMAP_H - 1; y++)
		for (unsigned int x = 0; x < TILEMAP_W; x++) {
			if (!isSolidTile({ x, y }) && !isSolidTile({x, y - 1}) && isSolidTile({x, y + 1}))
				pathingGraph.addWalkable({ x, y });
		}
		
		// Connect edges
		for (auto& index : pathingGraph.walkableCoordinates) {
			glm::ivec2 coordinate = pathingGraph.getCoordinate(index);
			PathingNode& pathingNode = pathingGraph.getNode(coordinate);


			
			for (int dir : {-1, 1}) {
				bool canWalkToNeighbor = false;
				for (FallCheck fallCheck : {FallCheck(1, 0, 2, 0), FallCheck(2, 1, 4, 0), FallCheck(3, 1, 4, 1), FallCheck(4, 2, 4, 1), FallCheck(5, 7, 4, 2)}) {
					unsigned int fallXClose = coordinate.x + dir;
					unsigned int fallXFar = coordinate.x + fallCheck.mag * dir;
					

					// check empty rectangle next to ledge
					if (scanRectUnordered(coordinate + glm::ivec2(0, -1), {fallXFar, coordinate.y})) continue;

					unsigned int fallY = coordinate.y;

					bool isValid = false;
					

					while(1)  {
						
						if (!pathingGraph.hasTile({ fallXFar , fallY})) break;
						if (scanRectUnordered({fallXClose, fallY}, {fallXFar, fallY})) break;
						if (pathingGraph.isWalkable({ fallXFar, fallY })) {
							isValid = true;
							break;
						}
						fallY++;
					}


					if (isValid) {

						

						glm::ivec2 otherCoordinate = { fallXFar, fallY };
						PathingNode& otherNode = pathingGraph.getNode(otherCoordinate);

						int minExtraSpace = fallCheck.minExtraJumpSpaceY - abs((coordinate.y - otherCoordinate.y)) / 2;
						minExtraSpace = std::max(minExtraSpace, 0);
						bool hasJumpSpace = !scanRectUnordered(coordinate + glm::ivec2(0, -1), { fallXFar, coordinate.y - 1 - minExtraSpace });

						if (std::abs(coordinate.y - otherCoordinate.y) >= fallCheck.minFallY) {
							pathingNode.reachableCoordinates.push_back(PathNodeEdge(PATH_EDGE_FLAVOR::FALL_WALK, otherCoordinate));
							if (otherCoordinate.y == coordinate.y && abs(otherCoordinate.x - coordinate.x) <= 1) canWalkToNeighbor = true;
						} else if (hasJumpSpace && coordinate.y - otherCoordinate.y <= fallCheck.maxJumpY) {
							if (!(otherCoordinate.y == coordinate.y && canWalkToNeighbor))
								pathingNode.reachableCoordinates.push_back(PathNodeEdge(PATH_EDGE_FLAVOR::JUMP, otherCoordinate));
						}

						if (hasJumpSpace && otherCoordinate.y - coordinate.y <= fallCheck.maxJumpY) {
							if (! (otherCoordinate.y == coordinate.y && canWalkToNeighbor))
								otherNode.reachableCoordinates.push_back(PathNodeEdge(PATH_EDGE_FLAVOR::JUMP, coordinate));
						}
					}
				}
			}
		}
		
		
		auto& transform = ecs.transforms.emplace(pathingGraphEntity);
		transform.translate(-Vector2(pathingGraph.width(), pathingGraph.height()) * pathingGraph.tileSize * 0.5f);


		return pathingGraphEntity;
	}


	float heuristicDistance(glm::ivec2& start, glm::ivec2& end) {
		float distance = Vector2(start).distanceTo(Vector2(end));
		return distance;
	};

	// Advanced Feature: [15] Advanced decision-making
	// Implementation of A* pathfinding on a PathingGraph<30, 17>
	void findPath(PathingGraph<30, 17>& pathingGraph, glm::ivec2 start, glm::ivec2 end, Path& path) {



		struct SearchNode {
			unsigned int previousIndex;
			PathNodeEdge edge;
			float h = 0;
			float g = INFINITY;
			float f() const { return h + g; };
		};

		std::array<SearchNode, TILEMAP_W* TILEMAP_H> searchSpace;
		searchSpace[pathingGraph.getIndex(start)] = SearchNode(
			pathingGraph.getIndex(start),
			PathNodeEdge(),
			heuristicDistance(start, end),
			0
		);

		auto compare = [&searchSpace](const unsigned int& i1, const unsigned int& i2) {
			if (searchSpace[i1].f() > 50)
				std::cout << searchSpace[i1].f() << std::endl;
			return searchSpace[i1].f() == searchSpace[i2].f() ? searchSpace[i1].h > searchSpace[i2].h :  searchSpace[i1].f() > searchSpace[i2].f(); // tie break by the smaller h score
		};

		

		path.edges.clear();
		
		
		std::priority_queue<unsigned int, std::vector<unsigned int>, decltype(compare)> toSearch(compare);
		std::unordered_set<unsigned int> toSearchSet;
		std::unordered_set<unsigned int> searchedIndexes;

		toSearch.emplace(pathingGraph.getIndex(start));
		toSearchSet.insert(pathingGraph.getIndex(start));

		while (!toSearch.empty()) {
			unsigned int index = toSearch.top();
			toSearch.pop();
			toSearchSet.erase(index);

			SearchNode& searchNode = searchSpace[index];
			glm::ivec2 coordinate = pathingGraph.getCoordinate(index);
			PathingNode& pathingNode = pathingGraph.getNode(coordinate);
			
			
			// Found end? Rebuild path...
			if (index == pathingGraph.getIndex(end)) {
				while (index != pathingGraph.getIndex(start)) {
					SearchNode& searchNode = searchSpace[index];
					path.edges.push_back(searchNode.edge);
					index = searchNode.previousIndex;
				}

				break;
			}

			// Add new nodes
			for (auto& edge : pathingNode.reachableCoordinates) {

				unsigned int edgeIndex = pathingGraph.getIndex(edge.coordinate);
				float possibleGScore = searchNode.g + heuristicDistance(coordinate, edge.coordinate);
				SearchNode& edgeSearchNode = searchSpace[edgeIndex];

				// Update G score
				if (possibleGScore >= edgeSearchNode.g) continue;
				edgeSearchNode.g = possibleGScore;
				edgeSearchNode.previousIndex = index;
				edgeSearchNode.edge = edge;
				
				// Add to search set if not already present
				if (toSearchSet.contains(edgeIndex)) continue;
				toSearchSet.insert(edgeIndex);
				toSearch.emplace(edgeIndex);
				edgeSearchNode.h = heuristicDistance(edge.coordinate, end);
				
			}
		}
	}

};
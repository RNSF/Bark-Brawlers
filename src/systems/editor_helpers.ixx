
module;

#include <common.hpp>

export module EditorHelpers;
import ECS;
import Entity;
import Tiling;
import RenderHelpers;

export {

	// TODO: get this function into its own system
	void updateTilemap(Entity entity, bool doDataPush = false) {

		auto& tileset = ecs.tilesets.get(entity);
		auto& tilemap = ecs.tilemaps.get(entity);
		auto& instancedRender = ecs.instancedRenders.get(entity);

		for (unsigned int y = 0; y < tilemap.height(); y++) {
			for (unsigned int x = 0; x < tilemap.width(); x++) {
				int tileID = tilemap.getTile({ x, y }).id ? Tiling::getAutoTilingID(tilemap.tiles, x, y, tileset.tileCount) : Tiling::getAirTileID(tileset.tileCount);
				instancedRender.offsets.data[x + y * tilemap.width()] = Vector2(x, y) * tilemap.tileSize;
				instancedRender.sections.data[x + y * tilemap.width()] = getSubSpriteRect(tileset.tileCount, tileID);
			}
		}

		if (doDataPush) {
			instancedRender.offsets.pushData();
			instancedRender.sections.pushData();
		}
	}
}
module;

#include "common.hpp"
#include <iostream>
#include <unordered_map>

export module Tiling;

import Components;

export struct Tiling {
	
	static const std::unordered_map<int, glm::ivec2> tilesetSize;

	enum NB {
		UL = (1 << 0),
		U = (1 << 1),
		UR = (1 << 2),
		L = (1 << 3),
		R = (1 << 4),
		DL = (1 << 5),
		D = (1 << 6),
		DR = (1 << 7),

		U_EDGE = UL | U | UR,
		D_EDGE = DL | D | DR,
		L_EDGE = UL | L | DL,
		R_EDGE = UR | R | DR,

		UR_CORNER = U | UR | R,
		DR_CORNER = D | DR | R,
		UL_CORNER = U | UL | L,
		DL_CORNER = D | DL | L,

		CROSS = U | D | L | R,
		ALL = U_EDGE | L | R | D_EDGE,
		NONE = 0
	};

	enum class TILES6X8 {
		EMPTY = 0,
		FULL_TILE = 1,

		INNER_UL_CORNER = 2, // Air top left
		INNER_UR_CORNER = 3,
		INNER_DL_CORNER = 9,
		INNER_DR_CORNER = 5,

		INNER_U_EDGE = 4,
		INNER_R_EDGE = 7,
		INNER_L_EDGE = 10,
		INNER_D_EDGE = 13,

		INNER_BACKSLASH = 6, // Air top left + bot right
		INNER_SLASH = 11,

		INNER_CORNERS_EXCEPT_DL = 8, // Cross + tile bot left
		INNER_CORNERS_EXCEPT_DR = 12,
		INNER_CORNERS_EXCEPT_UR = 14,
		INNER_CORNERS_EXCEPT_UL = 15,

		CROSS = 16,

		L_EDGE = 17,
		U_EDGE = 21,
		R_EDGE = 25,
		D_EDGE = 29,

		L_EDGE_PLUS_UR = 18, // Air top right
		L_EDGE_PLUS_DR = 19,

		U_EDGE_PLUS_DR = 22,
		U_EDGE_PLUS_DL = 23,

		R_EDGE_PLUS_DL = 26,
		R_EDGE_PLUS_UL = 27,

		D_EDGE_PLUS_UL = 30,
		D_EDGE_PLUS_UR = 31,

		T_D = 24, // T points down, tiles L/R/D
		T_L = 28,
		T_R = 20,
		T_U = 32,

		V_BAR = 33, // Tiles above and below
		H_BAR = 34,

		UL_CORNER = 35, // Tiles right and below
		UR_CORNER = 37,
		DR_CORNER = 39,
		DL_CORNER = 41,

		UL_BORDER = 36, // Tiles right and below only
		UR_BORDER = 38,
		DR_BORDER = 40,
		DL_BORDER = 42,

		U_SINGLE = 43, // Single tile below
		L_SINGLE = 44,
		D_SINGLE = 45,
		R_SINGLE = 46,

		DOT_TILE = 47
	};

	enum class TILES4X12 {
		EMPTY = 47,
		FULL_TILE = 26,

		INNER_UL_CORNER = 45, // Air top left
		INNER_UR_CORNER = 44,
		INNER_DL_CORNER = 33,
		INNER_DR_CORNER = 32,

		INNER_U_EDGE = 22,
		INNER_R_EDGE = 10,
		INNER_L_EDGE = 23,
		INNER_D_EDGE = 11,

		INNER_BACKSLASH = 34, // Air top left + bot right
		INNER_SLASH = 35,

		INNER_CORNERS_EXCEPT_DL = 21, // Cross + tile bot left
		INNER_CORNERS_EXCEPT_DR = 9,
		INNER_CORNERS_EXCEPT_UR = 8,
		INNER_CORNERS_EXCEPT_UL = 20,

		CROSS = 46,

		L_EDGE = 25,
		U_EDGE = 14,
		R_EDGE = 27,
		D_EDGE = 38,

		L_EDGE_PLUS_UR = 28, // Air top right
		L_EDGE_PLUS_DR = 30,

		U_EDGE_PLUS_DR = 29,
		U_EDGE_PLUS_DL = 31,

		R_EDGE_PLUS_DL = 41,
		R_EDGE_PLUS_UL = 43,

		D_EDGE_PLUS_UL = 40,
		D_EDGE_PLUS_UR = 42,

		T_D = 7, // T points down, tiles L/R/D
		T_L = 19,
		T_R = 6,
		T_U = 18,

		V_BAR = 24, // Tiles above and below
		H_BAR = 2,

		UL_CORNER = 13, // Tiles right and below
		UR_CORNER = 15,
		DR_CORNER = 39,
		DL_CORNER = 37,

		UL_BORDER = 4, // Tiles right and below only
		UR_BORDER = 5,
		DR_BORDER = 17,
		DL_BORDER = 16,

		U_SINGLE = 12, // Single tile below
		L_SINGLE = 1,
		D_SINGLE = 36,
		R_SINGLE = 3,

		DOT_TILE = 0
	};

	template <typename TileEnum>
	static int calculateBitmask(const std::array<Tile, TILEMAP_W* TILEMAP_H>& tilemap, int x, int y)
	{
		int bitmask = 0;

		// Neighbor offsets
		const int dx[8] = { -1,  0,  1, -1, 1, -1,  0, 1 };
		const int dy[8] = { -1, -1, -1,  0, 0,  1,  1, 1 };

		// 1  2  4
		// 8  T  16
		// 32 64 128
		const int bits[8] = { 1, 2, 4, 8, 16, 32, 64, 128 };

		// Check all 8 neighbors
		for (int i = 0; i < 8; ++i) {
			int nx = x + dx[i];
			int ny = y + dy[i];

			// Check for tiles within bounds
			if (nx >= 0 && ny >= 0 && nx < TILEMAP_W && ny < TILEMAP_H) {
				if (tilemap[ny * TILEMAP_W + nx].id != 0) {
					bitmask += bits[i];
				}
			} else {
				bitmask += bits[i];
			}
		}
		return bitmask;
	}


	template <typename TileEnum>
	static int mapBitmaskToTile(int bitmask) {
		TileEnum res = TileEnum::DOT_TILE;

		// Don't need to handle single corners

		// Single edges
		if ((bitmask & NB::U) == NB::U) res = TileEnum::D_SINGLE;
		if ((bitmask & NB::D) == NB::D) res = TileEnum::U_SINGLE;
		if ((bitmask & NB::L) == NB::L) res = TileEnum::R_SINGLE;
		if ((bitmask & NB::R) == NB::R) res = TileEnum::L_SINGLE;

		// 2 Edges
		if ((bitmask & (NB::U | NB::L)) == (NB::U | NB::L)) res = TileEnum::DR_BORDER;
		if ((bitmask & (NB::U | NB::R)) == (NB::U | NB::R)) res = TileEnum::DL_BORDER;
		if ((bitmask & (NB::D | NB::L)) == (NB::D | NB::L)) res = TileEnum::UR_BORDER;
		if ((bitmask & (NB::D | NB::R)) == (NB::D | NB::R)) res = TileEnum::UL_BORDER;

		// Bars
		if ((bitmask & (NB::U | NB::D)) == (NB::U | NB::D)) res = TileEnum::V_BAR;
		if ((bitmask & (NB::L | NB::R)) == (NB::L | NB::R)) res = TileEnum::H_BAR;

		// 3 Edges (T)
		if ((bitmask & (NB::CROSS & ~NB::U)) == (NB::CROSS & ~NB::U)) res = TileEnum::T_D;
		if ((bitmask & (NB::CROSS & ~NB::D)) == (NB::CROSS & ~NB::D)) res = TileEnum::T_U;
		if ((bitmask & (NB::CROSS & ~NB::L)) == (NB::CROSS & ~NB::L)) res = TileEnum::T_R;
		if ((bitmask & (NB::CROSS & ~NB::R)) == (NB::CROSS & ~NB::R)) res = TileEnum::T_L;

		// Cross
		if ((bitmask & NB::CROSS) == NB::CROSS) res = TileEnum::CROSS;

		// Corners
		if ((bitmask & (NB::UL_CORNER)) == NB::UL_CORNER) res = TileEnum::DR_CORNER;
		if ((bitmask & (NB::UR_CORNER)) == NB::UR_CORNER) res = TileEnum::DL_CORNER;
		if ((bitmask & (NB::DL_CORNER)) == NB::DL_CORNER) res = TileEnum::UR_CORNER;
		if ((bitmask & (NB::DR_CORNER)) == NB::DR_CORNER) res = TileEnum::UL_CORNER;

		// Corner + edge
		if ((bitmask & (NB::UL_CORNER | NB::R)) == (NB::UL_CORNER | NB::R)) res = TileEnum::D_EDGE_PLUS_UR;
		if ((bitmask & (NB::UL_CORNER | NB::D)) == (NB::UL_CORNER | NB::D)) res = TileEnum::R_EDGE_PLUS_DL;

		if ((bitmask & (NB::UR_CORNER | NB::L)) == (NB::UR_CORNER | NB::L)) res = TileEnum::D_EDGE_PLUS_UL;
		if ((bitmask & (NB::UR_CORNER | NB::D)) == (NB::UR_CORNER | NB::D)) res = TileEnum::L_EDGE_PLUS_DR;

		if ((bitmask & (NB::DL_CORNER | NB::R)) == (NB::DL_CORNER | NB::R)) res = TileEnum::U_EDGE_PLUS_DR;
		if ((bitmask & (NB::DL_CORNER | NB::U)) == (NB::DL_CORNER | NB::U)) res = TileEnum::R_EDGE_PLUS_UL;

		if ((bitmask & (NB::DR_CORNER | NB::L)) == (NB::DR_CORNER | NB::L)) res = TileEnum::U_EDGE_PLUS_DL;
		if ((bitmask & (NB::DR_CORNER | NB::U)) == (NB::DR_CORNER | NB::U)) res = TileEnum::L_EDGE_PLUS_UR;

		// Full edges
		if ((bitmask & (NB::U_EDGE | NB::L | NB::R)) == (NB::U_EDGE | NB::L | NB::R)) res = TileEnum::D_EDGE;
		if ((bitmask & (NB::D_EDGE | NB::L | NB::R)) == (NB::D_EDGE | NB::L | NB::R)) res = TileEnum::U_EDGE;
		if ((bitmask & (NB::L_EDGE | NB::U | NB::D)) == (NB::L_EDGE | NB::U | NB::D)) res = TileEnum::R_EDGE;
		if ((bitmask & (NB::R_EDGE | NB::U | NB::D)) == (NB::R_EDGE | NB::U | NB::D)) res = TileEnum::L_EDGE;

		// Cross + 1 corner (fish)
		if ((bitmask & (NB::CROSS | NB::UL)) == (NB::CROSS | NB::UL)) res = TileEnum::INNER_CORNERS_EXCEPT_UL;
		if ((bitmask & (NB::CROSS | NB::UR)) == (NB::CROSS | NB::UR)) res = TileEnum::INNER_CORNERS_EXCEPT_UR;
		if ((bitmask & (NB::CROSS | NB::DL)) == (NB::CROSS | NB::DL)) res = TileEnum::INNER_CORNERS_EXCEPT_DL;
		if ((bitmask & (NB::CROSS | NB::DR)) == (NB::CROSS | NB::DR)) res = TileEnum::INNER_CORNERS_EXCEPT_DR;

		// Cross + 2 corners
		if ((bitmask & (NB::CROSS | NB::UL | NB::UR)) == (NB::CROSS | NB::UL | NB::UR)) res = TileEnum::INNER_D_EDGE;
		if ((bitmask & (NB::CROSS | NB::DL | NB::DR)) == (NB::CROSS | NB::DL | NB::DR)) res = TileEnum::INNER_U_EDGE;
		if ((bitmask & (NB::CROSS | NB::UL | NB::DL)) == (NB::CROSS | NB::UL | NB::DL)) res = TileEnum::INNER_R_EDGE;
		if ((bitmask & (NB::CROSS | NB::UR | NB::DR)) == (NB::CROSS | NB::UR | NB::DR)) res = TileEnum::INNER_L_EDGE;
		if ((bitmask & (NB::CROSS | NB::UL | NB::DR)) == (NB::CROSS | NB::UL | NB::DR)) res = TileEnum::INNER_SLASH;
		if ((bitmask & (NB::CROSS | NB::UR | NB::DL)) == (NB::CROSS | NB::UR | NB::DL)) res = TileEnum::INNER_BACKSLASH;

		// Cross + 3 corners
		if ((bitmask & (NB::ALL & ~NB::UL)) == (NB::ALL & ~NB::UL)) res = TileEnum::INNER_UL_CORNER;
		if ((bitmask & (NB::ALL & ~NB::UR)) == (NB::ALL & ~NB::UR)) res = TileEnum::INNER_UR_CORNER;
		if ((bitmask & (NB::ALL & ~NB::DL)) == (NB::ALL & ~NB::DL)) res = TileEnum::INNER_DL_CORNER;
		if ((bitmask & (NB::ALL & ~NB::DR)) == (NB::ALL & ~NB::DR)) res = TileEnum::INNER_DR_CORNER;

		// All
		if (bitmask == NB::ALL) res = TileEnum::FULL_TILE;

		return (int)res;
	}

	static int getAutoTilingID(const std::array<Tile, TILEMAP_W* TILEMAP_H>& tilemap, int x, int y, glm::ivec2 tileCount) {
		if (tileCount == glm::ivec2(8, 6)) {
			int bitmask = calculateBitmask<TILES6X8>(tilemap, x, y);
			return mapBitmaskToTile<TILES6X8>(bitmask);
		}
		else {
			int bitmask = calculateBitmask<TILES4X12>(tilemap, x, y);
			return mapBitmaskToTile<TILES4X12>(bitmask);
		}
	}

	static int getAirTileID(glm::ivec2 tileCount) {
		return tileCount == glm::ivec2(8, 6) ? (int)TILES6X8::EMPTY : (int)TILES4X12::EMPTY;
	}
	
};

const std::unordered_map<int, glm::ivec2> Tiling::tilesetSize = {
	{(int)TEXTURE_ASSET_ID::GRASS_TILESET, {8, 6}},
	{(int)TEXTURE_ASSET_ID::GRASS_TILESET2, {8, 6}},
	{(int)TEXTURE_ASSET_ID::BLACK_BRICKS_TILESET, {12, 4}},
	{(int)TEXTURE_ASSET_ID::BLACK_BRICKS_TILESET2, {12, 4}},
	{(int)TEXTURE_ASSET_ID::SANDSTONE_BRICKS_TILESET, {12, 4}},
	{(int)TEXTURE_ASSET_ID::WOODEN_PLANKS_TILESET, {12, 4}},
	{(int)TEXTURE_ASSET_ID::DRY_DIRT_TILESET, {12, 4}},
	{(int)TEXTURE_ASSET_ID::DARK_DIRT_TILESET, {12, 4}},
	{(int)TEXTURE_ASSET_ID::CAVE_TILESET, {12, 4}},
	{(int)TEXTURE_ASSET_ID::RED_BRICKS_TILESET, {12, 4}},

};
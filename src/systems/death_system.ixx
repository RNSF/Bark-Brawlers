module;

#include <iostream>

export module DeathSystem;
import ECS;
import Singletons;
import RenderSystem;
import Factories;
import Components;
import Entity;
import Transform;
import Color;
import Vector2;

export class DeathSystem {

public:
	const float AFTER_DEATH_DURATION = 3.0f; // 3 seconds
	float afterDeathTimer = AFTER_DEATH_DURATION;

	bool isRoundOver = false;


	void update(float delta, RenderSystem& renderSystem) {
		
		if (gameState.currentScreen == GAME_SCREEN::TITLE_SCREEN) {
			// Don't process deaths in title screen
			return;
		}

		// iterate in reverse because we might remove entities
		for (unsigned int i = ecs.deathTimers.entities.size(); i > 0; i--) {
			Entity& entity = ecs.deathTimers.entities[i - 1];
			auto& deathTimer = ecs.deathTimers.get(entity);
			deathTimer.timeLeft -= delta;

			// kill if timeLeft is up.
			if (deathTimer.timeLeft <= 0.0f) {

				if (ecs.fireballs.has(entity)) {
					auto& fireball = ecs.fireballs.get(entity);
					renderSystem.destroyInstancedRender(ecs.instancedRenders.get(fireball.particle));
					ecs.remove_all_components_of(fireball.particle);
					ecs.remove_all_components_of(fireball.hitbox);
				}

				if (ecs.instancedRenders.has(entity)) {
					renderSystem.destroyInstancedRender(ecs.instancedRenders.get(entity));
				}

				ecs.remove_all_components_of(entity);
			}
		}

		if (isRoundOver) {
			afterDeathTimer -= delta;
		}

		for (Entity& entity : ecs.players.entities) {
			auto& health = ecs.healths.get(entity);
			if (health.health <= 0.0f && !ecs.dead.has(entity)) {
				isRoundOver = true;
				ecs.players.get(entity).isDead = true;

				if (!ecs.texts.has(gameState.gameOverText)) {
					Transform transform2;
					transform2.translate(Vector2::up() * 100);
					gameState.gameOverText = createText(L"Round over!", ATLAS_FONT_ID::BORE_BLASTERS, transform2, Color::white(), Vector2::unitCenter(), Color::black(), 1);
					renderSystem.initInstancedRender(ecs.instancedRenders.get(gameState.gameOverText));
				}

				if (afterDeathTimer < 0.0f) {
					std::cout << "Player " << ecs.players.get(entity).id << " is dead" << std::endl;

					isRoundOver = false;
					afterDeathTimer = AFTER_DEATH_DURATION;

					ecs.dead.emplace(entity, Dead{});
					// reset health
					health.health = 10.0f;
					ecs.players.get(entity).isDead = false;

					for (Entity& playerEntity : ecs.players.entities) {
						auto& player = ecs.players.get(playerEntity);
						auto& health = ecs.healths.get(playerEntity);
						if (!ecs.dead.has(playerEntity)) {
							if (player.id == 1) {
								gameState.gameScore.player1Wins++;
								gameState.gameScore.emphasizeP1Won = true;
							}
							if (player.id == 2) {
								gameState.gameScore.player2Wins++;
								gameState.gameScore.emphasizeP2Won = true;
							}
						}
					}
				}
				
				
			}
		}

	}

};
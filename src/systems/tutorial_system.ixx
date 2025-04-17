module;

#include <iostream>
#include <cassert>

export module TutorialSystem;
import ECS;
import Singletons;
import Factories;
import RenderSystem;
import Components;

export class TutorialSystem {

public:
	void update(float delta, RenderSystem& renderSystem) {
		bool isStateCompleted = false;
		TUTORIAL_STATE_ID oldState = tutorial.state;

		tutorial.stateTime += delta;


		// end tutorial if kill opponent
		if (tutorial.stateAchievements & (int)TUTORIAL_ACHIEVEMENT_ID::KILLED_AI) {
			tutorial.state = TUTORIAL_STATE_ID::INACTIVE;
		}

		do {
			switch (tutorial.state) {
			case TUTORIAL_STATE_ID::INACTIVE:		isStateCompleted = tutorial.isPendingStart; break;
			case TUTORIAL_STATE_ID::WALK:			isStateCompleted = tutorial.stateTime >= 1.0f && tutorial.stateAchievements & (int)TUTORIAL_ACHIEVEMENT_ID::WALKED; break;
			case TUTORIAL_STATE_ID::JUMP:			isStateCompleted = tutorial.stateTime >= 1.0f && tutorial.stateAchievements & (int)TUTORIAL_ACHIEVEMENT_ID::JUMPED; break;
			case TUTORIAL_STATE_ID::LONG_JUMP:		isStateCompleted = tutorial.stateTime >= 2.0f && tutorial.stateAchievements & (int)TUTORIAL_ACHIEVEMENT_ID::LONG_JUMPED; break;
			case TUTORIAL_STATE_ID::DASH:			isStateCompleted = tutorial.stateTime >= 1.0f && tutorial.stateAchievements & (int)TUTORIAL_ACHIEVEMENT_ID::DASHED; break;
			case TUTORIAL_STATE_ID::ATTACK:			isStateCompleted = tutorial.stateTime >= 1.0f && tutorial.stateAchievements & (int)TUTORIAL_ACHIEVEMENT_ID::ATTACKED; break;
			case TUTORIAL_STATE_ID::CHARGE_ATTACK:	isStateCompleted = tutorial.stateTime >= 1.0f && tutorial.stateAchievements & (int)TUTORIAL_ACHIEVEMENT_ID::FULLY_CHARGED_ATTACK; break;
			case TUTORIAL_STATE_ID::ATTACK_UP_DOWN:	isStateCompleted = tutorial.stateTime >= 1.0f && (tutorial.stateAchievements & (int)TUTORIAL_ACHIEVEMENT_ID::ATTACKED_UPWARD)
				|| (tutorial.stateAchievements & (int)TUTORIAL_ACHIEVEMENT_ID::ATTACKED_DOWNWARD); break;
			case TUTORIAL_STATE_ID::LAND_HIT:		isStateCompleted = tutorial.stateTime >= 1.0f && tutorial.stateAchievements & (int)TUTORIAL_ACHIEVEMENT_ID::CHARGED_SPECIAL; break;
			case TUTORIAL_STATE_ID::SPECIAL_DASH:	isStateCompleted = tutorial.stateTime >= 1.0f && tutorial.stateAchievements & (int)TUTORIAL_ACHIEVEMENT_ID::SPECIAL_DASHED; break;
			case TUTORIAL_STATE_ID::KILL:			isStateCompleted = tutorial.stateTime >= 1.0f && tutorial.stateAchievements & (int)TUTORIAL_ACHIEVEMENT_ID::KILLED_AI; break;
			default: assert(false); break;
			}

			if (isStateCompleted) {
				
				tutorial.state = static_cast<TUTORIAL_STATE_ID>((1 + (int)tutorial.state) % ((int)TUTORIAL_STATE_ID::COUNT));
				tutorial.stateAchievements = 0;

				
				
			}

			

		} while (isStateCompleted);

		tutorial.isPendingStart = false;

		// load new state
		if (oldState != tutorial.state) {

			if (tutorial.state == TUTORIAL_STATE_ID::KILL) {
				isTutorialCompleted = true;
			}

			tutorial.stateTime = 0.0f;

			if (tutorial.state == TUTORIAL_STATE_ID::INACTIVE) {
				if (ecs.texts.has(tutorial.textEntity)) {
					renderSystem.destroyInstancedRender(ecs.instancedRenders.get(tutorial.textEntity));
					renderSystem.destroyInstancedRender(ecs.instancedRenders.get(tutorial.skipTextEntity));
					renderSystem.destroyInstancedRender(ecs.instancedRenders.get(tutorial.subTextEntity));
					ecs.remove_all_components_of(tutorial.textEntity);
					ecs.remove_all_components_of(tutorial.skipTextEntity);
					ecs.remove_all_components_of(tutorial.subTextEntity);
				}
			} else {
				if (!ecs.texts.has(tutorial.textEntity)) {
					Transform transform;
					transform.translate(Vector2::up() * 100);
					tutorial.textEntity = createText(L"", ATLAS_FONT_ID::BORE_BLASTERS, transform, Color::white(), Vector2::unitCenter(), Color::black(), 1, 80);
					renderSystem.initInstancedRender(ecs.instancedRenders.get(tutorial.textEntity));

					Transform transform2;
					transform2.translate(Vector2::up() * 85);
					tutorial.subTextEntity = createText(L"", ATLAS_FONT_ID::SMILEY_5X5, transform2, Color::white(), Vector2::unitCenter(), Color::black(), 1, 200);
					renderSystem.initInstancedRender(ecs.instancedRenders.get(tutorial.subTextEntity));

					Transform transform3;
					transform3.translate(Vector2::down() * 110);
					tutorial.skipTextEntity = createText(L"Press [Esc] to skip", ATLAS_FONT_ID::SMILEY_5X5, transform3, Color::white(), Vector2::unitCenter(), Color::black(), 1);
					renderSystem.initInstancedRender(ecs.instancedRenders.get(tutorial.skipTextEntity));
				}

				Text& text = ecs.texts.get(tutorial.textEntity);
				Text& subText = ecs.texts.get(tutorial.subTextEntity);

				subText.updateStr(L"");

				switch (tutorial.state) {
					case TUTORIAL_STATE_ID::INACTIVE:		break;
					case TUTORIAL_STATE_ID::WALK:			text.updateStr(L"Press [A] and [D] to move"); break;
					case TUTORIAL_STATE_ID::JUMP:			text.updateStr(L"Press [F] to jump"); break;
					case TUTORIAL_STATE_ID::LONG_JUMP:		text.updateStr(L"Hold [F] to jump higher"); break;
					case TUTORIAL_STATE_ID::DASH:			{
						text.updateStr(L"Press [H] to dash"); 
						switch (gameState.player1Options.selectedClass) {
						case CLASS_ID::KNIGHT: subText.updateStr(L"The knight can dash left and right"); break;
						case CLASS_ID::ARCHER: subText.updateStr(L"The archer does a quick back hop"); break;
						case CLASS_ID::MAGE: subText.updateStr(L"The mage can dash in all 4 directions"); break;
						}
					} break;
												
					case TUTORIAL_STATE_ID::ATTACK:			text.updateStr(L"Press [G] to attack"); break;
					case TUTORIAL_STATE_ID::CHARGE_ATTACK:	{
						text.updateStr(L"Hold [G] to do a charged attack"); 
						subText.updateStr(L"Landing a fully charged attack deals double damage!");
					} break;
					case TUTORIAL_STATE_ID::ATTACK_UP_DOWN:	{ 
						text.updateStr(L"Aim your attacks with [W] and [S]"); 
						subText.updateStr(L"You can attack diagonally or straight up and down");
					} break;
					case TUTORIAL_STATE_ID::LAND_HIT:		{ 
						text.updateStr(L"Land a hit to charge your special"); 
						subText.updateStr(L"If you have a white outline your special is charged!");
					} break;
					case TUTORIAL_STATE_ID::SPECIAL_DASH:	text.updateStr(L"Hold [G] then [H] to special attack"); {
						switch (gameState.player1Options.selectedClass) {
						case CLASS_ID::KNIGHT: subText.updateStr(L"The knight deals damage to anything they special dash into"); break;
						case CLASS_ID::ARCHER: subText.updateStr(L"The archer shoots out a burst of arrows"); break;
						case CLASS_ID::MAGE: subText.updateStr(L"The mage casts a giant fireball"); break;
						}
					}break;
					case TUTORIAL_STATE_ID::KILL:			{ 
						text.updateStr(L"Defeat your opponent to win"); 
						subText.updateStr(L"In a normal game, first to 3 wins is the champion!");
					} break;
					default: assert(false); break;
				}

				
			}
		}

	}

};
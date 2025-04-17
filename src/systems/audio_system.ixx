
module;

#include <SDL.h>
#include <SDL_mixer.h>
#include <common.hpp>
#include <iostream>

export module AudioSystem;
import Components;
import RNG;

export class AudioSystem {

	std::array<Mix_Chunk*, (int) SFX_ASSET_ID::COUNT> loadedSfx;
	std::array<Mix_Music*, (int) MUSIC_ASSET_ID::COUNT> loadedMusic;

public:
	bool init() {
		
		if (SDL_Init(SDL_INIT_AUDIO) < 0) {
			fprintf(stderr, "Failed to initialize SDL Audio");
			return false;
		}



		if (Mix_OpenAudio(44100, MIX_DEFAULT_FORMAT, 2, 2048) == -1) {
			fprintf(stderr, "Failed to open audio device");
			return false;
		}

		Mix_AllocateChannels(32);

		// Load SFX
		for (unsigned int i = 0; i < (unsigned int)SFX_ASSET_ID::COUNT; i++) {
			auto* chunk = Mix_LoadWAV(sfxPath(SFX_FILES[i]).c_str());
			if (chunk == nullptr) return false;
			loadedSfx[i] = chunk;
		}

		// Load Music
		for (unsigned int i = 0; i < (unsigned int)MUSIC_ASSET_ID::COUNT; i++) {
			auto* chunk = Mix_LoadMUS(musicPath(MUSIC_FILES[i]).c_str());
			if (chunk == nullptr) return false;
			loadedMusic[i] = chunk;
		}

		return true;
	};

	void destroy() {
		// Free SFX
		for (unsigned int i = 0; i < (unsigned int)SFX_ASSET_ID::COUNT; i++) {
			Mix_FreeChunk(loadedSfx[i]);
		}

		// Free Music
		for (unsigned int i = 0; i < (unsigned int)MUSIC_ASSET_ID::COUNT; i++) {
			Mix_FreeMusic(loadedMusic[i]);
		}

		Mix_CloseAudio();
	};


	int playSfx(SFX_ASSET_ID sfxAsset, bool loops = false) {
		int channel = Mix_PlayChannel(-1, loadedSfx[(int)sfxAsset], loops ? -1 : 0);
		if (channel == -1) std::cout << "Error: could not play sound!" << std::endl;
		Mix_Volume(channel, MIX_MAX_VOLUME);
		return channel;
	}

	void haltSfx(int channel) {
		if (channel == -1) return;
		Mix_HaltChannel(channel);
	}

	void haltAllSfx() {
		Mix_HaltChannel(-1);
	}

	void setSfxVolume(int channel, float volume) {
		if (channel == -1) return;
		Mix_Volume(channel, volume * MIX_MAX_VOLUME);
	}

	int playSfxRandomRange(SFX_ASSET_ID minSfxAsset, SFX_ASSET_ID maxSfxAsset) {
		int channel = Mix_PlayChannel(-1, loadedSfx[randomIntRange((int)minSfxAsset, (int)maxSfxAsset)], false);
		return channel;
	}

	void playMusic(MUSIC_ASSET_ID musicAsset) {
		Mix_PlayMusic(loadedMusic[(int) musicAsset], INT_MAX);
	}

	void haltMusic() {
		Mix_HaltMusic();
	}
};
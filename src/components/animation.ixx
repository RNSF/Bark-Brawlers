module;
#include <vector>
export module Animation;

export {
    float WALK_FRAME_DURATION = 0.05f;
    float JUMP_FRAME_DURATION = 0.25f;
    float IDLE_FRAME_DURATION = 0.25f;
	float DEATH_FRAME_DURATION = 0.25f;

    struct AnimationFrame {
        unsigned int textureIndex;
        float duration;  // in seconds
    };

    struct Animation {
        std::vector<AnimationFrame> frames;
        float timeInCurrentFrame = 0.f;
        unsigned int currentFrameIndex = 0;
        bool isPlaying = true;
        bool loop = true;

        void update(float deltaTime) {
            if (!isPlaying) return;

            timeInCurrentFrame += deltaTime;

            if (timeInCurrentFrame >= frames[currentFrameIndex].duration) {
                timeInCurrentFrame -= frames[currentFrameIndex].duration;
                currentFrameIndex++;

                if (currentFrameIndex >= frames.size()) {
                    if (loop) {
                        currentFrameIndex = 0;
                    }
                    else {
                        currentFrameIndex--;
                        isPlaying = false;
                    }
                }
            }
        }

        void play() {
            isPlaying = true;
        }

        void pause() {
            isPlaying = false;
        }

        void reset() {
            currentFrameIndex = 0;
            timeInCurrentFrame = 0.f;
        }

        unsigned int getCurrentFrame() const {
            return frames[currentFrameIndex].textureIndex;
        }
    };

    struct WalkAnimation : Animation {
        WalkAnimation() {
            frames = {
                {8, WALK_FRAME_DURATION},
                {9, WALK_FRAME_DURATION},
                {10, WALK_FRAME_DURATION},
                {11, WALK_FRAME_DURATION},
                {12, WALK_FRAME_DURATION},
                {13, WALK_FRAME_DURATION},
                {14, WALK_FRAME_DURATION},
            };
            loop = true;
        }
    };

    struct FallAnimation : Animation {
        FallAnimation() {
            frames = {
                {21, JUMP_FRAME_DURATION},
                {22, JUMP_FRAME_DURATION},
            };
            loop = true;
        }
    };

    struct IdleAnimation : Animation {
        IdleAnimation() {
            frames = {
                {0, IDLE_FRAME_DURATION},
                {1, IDLE_FRAME_DURATION},
                {2, IDLE_FRAME_DURATION},
                {3, IDLE_FRAME_DURATION},
            };
            loop = true;
        }
    };

    struct DeathAnimation : Animation {
        DeathAnimation() {
            frames = {
            { 30, DEATH_FRAME_DURATION },
            { 31, DEATH_FRAME_DURATION },
            { 32, DEATH_FRAME_DURATION },
            { 33, DEATH_FRAME_DURATION },
            };
            loop = false;
        }
    };
}
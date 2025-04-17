module;

#include <cmath>
#include <iostream>

export module ParticleSystem;
import ECS;
import Components;
import Vector2;
import RenderHelpers;
import Math;

export class ParticleSystem {

public:
	void update(float delta) {
		for (auto& entity : ecs.particleEmitters.entities) {
			ParticleEmitter& particleEmitter = ecs.particleEmitters.get(entity);
			InstancedRender& instancedRender = ecs.instancedRenders.get(entity);

			unsigned int startIndex = particleEmitter.getNextParticleToEmit();
			particleEmitter.lifetime += delta;
			unsigned int endIndex = particleEmitter.getNextParticleToEmit();

			// Spawn new particles
			for (unsigned int i = startIndex; i < endIndex; i++) {
				Particle& particle = particleEmitter.particles[i % particleEmitter.particles.size()];
				particle = particleEmitter.newParticle();
				/*particle.position += transform.getTranslation();
				particle.rotation += transform.getRotation();*/
			};

			
			for (unsigned int i = 0; i < particleEmitter.particles.size(); i++) {
				int instancedRenderIndex = wrapMod(i - endIndex, 0, particleEmitter.particles.size());
				Particle& particle = particleEmitter.particles[i];
				particle.timeLeft -= delta;
				if (!particle.isAlive()) {
					instancedRender.colors.data[instancedRenderIndex] = Color::clear();
					continue;
				}

				// Update physics
				float lifePercent = particle.getTimePercent();
				particle.velocity += particleEmitter.getAcceleration(lifePercent) * delta;
				particle.velocity *= pow(particleEmitter.getDampening(lifePercent), delta);
				particle.position += particle.velocity * delta;

				particle.angularVelocity += particleEmitter.getAngularAcceleration(lifePercent) * delta;
				particle.angularVelocity *= pow(particleEmitter.getAngularDampening(lifePercent), delta);
				particle.rotation += particle.angularVelocity * delta;

				// Update instancedRender
				instancedRender.bases.data[instancedRenderIndex] = Basis::withScaleRotation(Vector2::one(), particle.rotation);
				instancedRender.offsets.data[instancedRenderIndex] = particle.position;
				instancedRender.sections.data[instancedRenderIndex] = getSubSpriteRect(particleEmitter.spriteCount, particleEmitter.getFrame(lifePercent) * particleEmitter.spriteCount.x * particleEmitter.spriteCount.y);
				instancedRender.colors.data[instancedRenderIndex] = particleEmitter.getColor(lifePercent);
			};

			instancedRender.bases.pushData();
			instancedRender.offsets.pushData();
			instancedRender.sections.pushData();
			instancedRender.colors.pushData();
			
		}
	}

};
#pragma once

#include "Engine/Renderer/Rgba.hpp"
#include "Engine/Math/Range.hpp"
#include "Engine/Math/Vector2.hpp"
#include "Engine/Renderer/GLRenderer.hpp"
#include "Engine/Renderer/SpriteRenderer.hpp"
#include "Engine/Renderer/RendererInterface.hpp"

#include <map>
#include <string>
#include <vector>


//-----------------------------------------------------------------------------------------------
void ParticleSystemInit();
void ParticleSystemPlay(const std::string& name, const Vector2& systemLoc, ERenderLayer layer);
class ParticleSystem* ParticleSystemCreate(const std::string& name, const Vector2& systemLoc, ERenderLayer layer);
void ParticleSystemDestroy(class ParticleSystem* system);
void ParticleSystemDestroyImmediate(class ParticleSystem* system);


//-----------------------------------------------------------------------------------------------
class ParticleEmitterDefinition
{
public:
	ParticleEmitterDefinition(const std::string& sprite);
	bool IsLooping() const { return m_particlesPerSecond > 0.f; }
	void Update(float deltaSeconds, std::vector<struct Particle>& particles, float& timeSinceLastSpawn, bool canSpawn) const;
	Particle MakeParticle() const;

public:
	const class SpriteResource* m_sprite;
	int m_initialParticles;
	float m_particlesPerSecond;
	Rgba m_tint;
	Range<float> m_lifetime;
	Range<Vector2> m_startVelocity;
	Range<Vector2> m_startAcceleration;
	Range<Vector2> m_scale;
	float m_radius;
	EBlendFunction m_blendFunction;
};


//-----------------------------------------------------------------------------------------------
struct Particle
{
	Vector2 position;
	Vector2 velocity;
	Vector2 acceleration;
	float maxAge;
	float currAge;
	Vector2 rectangleHalfDimensions;
	Rgba tint;
};


//-----------------------------------------------------------------------------------------------
class ParticleEmitter
{
public:
	static ParticleEmitter* Create(const ParticleEmitterDefinition* ped);
	void Update(float deltaSeconds);
	void Render() const;
	void MarkForDeletion() { m_isQueuedForDestruction = true; }
	bool IsDone() const
	{
		if (m_definition->IsLooping())
		{
			return m_isReadyToDelete;
		}
		else
		{
			return m_particles.empty();
		}
	}

private:
	float m_timeSinceLastSpawn;
	const ParticleEmitterDefinition* m_definition;
	std::vector<Particle> m_particles;
	bool m_isQueuedForDestruction = false;
	bool m_isReadyToDelete = false;
};


//-----------------------------------------------------------------------------------------------
class ParticleSystemDefinition
{
	friend class ParticleSystem;

public:
	ParticleEmitterDefinition* AddEmitter(ParticleEmitterDefinition* toAdd);
	~ParticleSystemDefinition();
	static ParticleSystemDefinition* Register(const std::string& name);
	static void DestroySystems();
	bool IsLooping() const;

private:
	std::vector<ParticleEmitterDefinition*> m_emitters;
	static std::map<std::string, ParticleSystemDefinition*> s_particleSystemDefinitionRegistry;
};


//-----------------------------------------------------------------------------------------------
class ParticleSystem : public RendererInterface
{
	friend class ParticleSystemDefinition;

public:
	bool IsLooping() const;
	static ParticleSystem* Create(const std::string& name, const Vector2& loc);
	virtual void Update(float deltaSeconds) override;
	virtual void Render() const override;
	virtual bool ShouldDie() const override { return IsDone(); }
	void MarkForDeletion() { m_isQueuedForDestruction = true; }
	bool IsDone() const;
	~ParticleSystem();

private:
	std::vector<ParticleEmitter*> m_emitters;
	Vector2 m_location;
	ParticleSystemDefinition* m_definition;
	bool m_isQueuedForDestruction = false;
	bool m_isReadyToDelete = false;
};
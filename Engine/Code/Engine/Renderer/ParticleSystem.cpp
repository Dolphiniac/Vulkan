#include "Engine/Renderer/ParticleSystem.hpp"
#include "Engine/Renderer/Sprite.hpp"
#include "Engine/Core/ErrorWarningAssert.hpp"
#include "Engine/Core/EngineSystemManager.hpp"
#include "Engine/Model/MeshBuilder.hpp"


//-----------------------------------------------------------------------------------------------
std::map<std::string, ParticleSystemDefinition*> ParticleSystemDefinition::s_particleSystemDefinitionRegistry;


//-----------------------------------------------------------------------------------------------
ParticleEmitterDefinition::ParticleEmitterDefinition(const std::string& sprite)
{
	m_sprite = SpriteResource::GetResource(sprite);
}


//-----------------------------------------------------------------------------------------------
ParticleEmitterDefinition* ParticleSystemDefinition::AddEmitter(ParticleEmitterDefinition* toAdd)
{
	m_emitters.push_back(toAdd);
	return toAdd;
}


//-----------------------------------------------------------------------------------------------
Particle ParticleEmitterDefinition::MakeParticle() const
{
	Particle thisParticle;
	thisParticle.maxAge = m_lifetime.GetRandom();
	thisParticle.currAge = 0.f;
	thisParticle.acceleration = m_startAcceleration.GetRandom();
	thisParticle.velocity = m_startVelocity.GetRandom();
	thisParticle.rectangleHalfDimensions = m_radius * m_scale.GetRandom();
	thisParticle.tint = m_tint;
	
	return thisParticle;
}


//-----------------------------------------------------------------------------------------------
void ParticleEmitterDefinition::Update(float deltaSeconds, std::vector<struct Particle>& particles, float& timeSinceLastSpawn, bool canSpawn) const
{
	for (Particle& p : particles)
	{
		p.velocity += p.acceleration * deltaSeconds;
		p.position += p.velocity * deltaSeconds;
		p.currAge += deltaSeconds;
	}

	for (size_t i = 0; i < particles.size(); i++)
	{
		if (particles[i].currAge >= particles[i].maxAge)
		{
			std::swap(particles[i], particles[particles.size() - 1]);
			particles.pop_back();
		}
	}

	if (!IsLooping())
	{
		if (timeSinceLastSpawn == 0.f)
		{
			for (int i = 0; i < m_initialParticles; i++)
			{
				particles.push_back(MakeParticle());
			}
		}
	}
	timeSinceLastSpawn += deltaSeconds;
	if (IsLooping() && canSpawn)
	{
		float secondsPerParticle = 1.f / m_particlesPerSecond;
		while (timeSinceLastSpawn >= secondsPerParticle)
		{
			particles.push_back(MakeParticle());
			timeSinceLastSpawn -= secondsPerParticle;
		}
	}
}


//-----------------------------------------------------------------------------------------------
ParticleSystemDefinition::~ParticleSystemDefinition()
{
	for (ParticleEmitterDefinition* emitter : m_emitters)
	{
		delete emitter;
	}
}


//-----------------------------------------------------------------------------------------------
ParticleEmitter* ParticleEmitter::Create(const ParticleEmitterDefinition* ped)
{
	ParticleEmitter* result = new ParticleEmitter();
	result->m_definition = ped;

	return result;
}


//-----------------------------------------------------------------------------------------------
void ParticleEmitter::Update(float deltaSeconds)
{
	if (m_isQueuedForDestruction && m_particles.empty())
	{
		m_isReadyToDelete = true;
	}
	else
	{
		m_definition->Update(deltaSeconds, m_particles, m_timeSinceLastSpawn, !m_isQueuedForDestruction);
	}
}


//-----------------------------------------------------------------------------------------------
void ParticleEmitter::Render() const
{
	if (m_particles.empty())
	{
		return;
	}
	The.Renderer->UseZBuffering(false);
	The.Renderer->BindTexture(m_definition->m_sprite->GetTexture());
	The.Renderer->SetBlendFunction(m_definition->m_blendFunction);
	MeshBuilder mb("particles");
	std::vector<Vertex_PCT> verts;
	for (const Particle& p : m_particles)
	{
		Vector2 bl(p.position - p.rectangleHalfDimensions);
		Vector2 tr(p.position + p.rectangleHalfDimensions);
		Vector2 br(tr.x, bl.y);
		Vector2 tl(bl.x, tr.y);
		Vertex_PCT tlv;
		tlv.m_color = p.tint;
		tlv.m_texCoords = Vector2(0.f, 0.f);
		tlv.m_position = tl;
		Vertex_PCT blv = tlv;
		blv.m_texCoords = Vector2(0.f, 1.f);
		blv.m_position = bl;
		Vertex_PCT brv = tlv;
		brv.m_texCoords = Vector2(1.f, 1.f);
		brv.m_position = br;
		Vertex_PCT trv = tlv;
		trv.m_texCoords = Vector2(1.f, 0.f);
		trv.m_position = tr;
		verts.push_back(tlv);
		verts.push_back(blv);
		verts.push_back(brv);
		verts.push_back(brv);
		verts.push_back(trv);
		verts.push_back(tlv);
	}
	The.Renderer->DrawVertexArray(&verts[0], verts.size(), R_TRIANGLES);
}


//-----------------------------------------------------------------------------------------------
ParticleSystemDefinition* ParticleSystemDefinition::Register(const std::string& name)
{
	ParticleSystemDefinition* def = new ParticleSystemDefinition();
	s_particleSystemDefinitionRegistry.insert(std::make_pair(name, def));
	return def;
}


//-----------------------------------------------------------------------------------------------
void ParticleSystemDefinition::DestroySystems()
{
	for (std::pair<const std::string, ParticleSystemDefinition*>& p : s_particleSystemDefinitionRegistry)
	{
		delete p.second;
	}

	s_particleSystemDefinitionRegistry.clear();
}


//-----------------------------------------------------------------------------------------------
bool ParticleSystemDefinition::IsLooping() const
{
	for (ParticleEmitterDefinition* def : m_emitters)
	{
		if (def->IsLooping())
		{
			return true;
		}
	}

	return false;
}


//-----------------------------------------------------------------------------------------------
ParticleSystem* ParticleSystem::Create(const std::string& name, const Vector2& loc)
{
	ParticleSystem* result = new ParticleSystem();
	result->m_definition = ParticleSystemDefinition::s_particleSystemDefinitionRegistry.at(name);
	result->m_location = loc;

	for (ParticleEmitterDefinition* ped : result->m_definition->m_emitters)
	{
		result->m_emitters.push_back(ParticleEmitter::Create(ped));
	}

	return result;
}


//-----------------------------------------------------------------------------------------------
bool ParticleSystem::IsLooping() const
{
	return m_definition->IsLooping();
}


//-----------------------------------------------------------------------------------------------
void ParticleSystem::Update(float deltaSeconds)
{
	m_isReadyToDelete = true;
	for (ParticleEmitter* emitter : m_emitters)
	{
		if (m_isQueuedForDestruction)
		{
			emitter->MarkForDeletion();
		}
		emitter->Update(deltaSeconds);
		if (!emitter->IsDone())
		{
			m_isReadyToDelete = false;
		}
	}
}


//-----------------------------------------------------------------------------------------------
void ParticleSystem::Render() const
{
	for (const ParticleEmitter* emitter : m_emitters)
	{
		emitter->Render();
	}
}


//-----------------------------------------------------------------------------------------------
bool ParticleSystem::IsDone() const
{
	if (IsLooping())
	{
		return m_isReadyToDelete;
	}

	for (const ParticleEmitter* emitter : m_emitters)
	{
		if (!emitter->IsDone())
		{
			return false;
		}
	}

	return true;
}


//-----------------------------------------------------------------------------------------------
ParticleSystem::~ParticleSystem()
{
	for (ParticleEmitter* pe : m_emitters)
	{
		delete pe;
	}
}


//-----------------------------------------------------------------------------------------------
void ParticleSystemInit()
{
	SpriteResource::Register("soft particle", "Data/Textures/soft_particle.png");

	ParticleSystemDefinition* spark = ParticleSystemDefinition::Register("spark");

	ParticleEmitterDefinition* sparks = spark->AddEmitter(new ParticleEmitterDefinition("soft particle"));
	sparks->m_blendFunction = ADDITIVE;
	sparks->m_initialParticles = 100;
	sparks->m_particlesPerSecond = 0.f;
	sparks->m_lifetime = Range<float>(1.f, 5.f);
	sparks->m_scale = Range<Vector2>(.5f, .7f);
	sparks->m_tint = Rgba(.8f, .8f, .1f, .2f);
	sparks->m_startVelocity = Range<Vector2>(-1.f, 1.f);
	ASSERT_OR_DIE(!sparks->IsLooping(), "Should not evaluate to looping");

	ParticleSystemDefinition* smoke = ParticleSystemDefinition::Register("smoke");

	ParticleEmitterDefinition* smokey = smoke->AddEmitter(new ParticleEmitterDefinition("soft particle"));
	smokey->m_blendFunction = ALPHABLEND;
	smokey->m_initialParticles = 10;
	smokey->m_particlesPerSecond = 5.f;
	smokey->m_lifetime = Range<float>(.75f, 1.f);
	smokey->m_scale = Vector2(1.f);
	smokey->m_tint = Rgba(.4f, .4f, .4f, .2f);
	smokey->m_startVelocity = Range<Vector2>(-.5f, .5f);
	ASSERT_OR_DIE(smokey->IsLooping(), "Should evaluate to looping");
}


//-----------------------------------------------------------------------------------------------
void ParticleSystemPlay(const std::string& name, const Vector2& systemLoc, ERenderLayer layer)
{
	ParticleSystem* sys = ParticleSystem::Create(name, systemLoc);
	ASSERT_OR_DIE(!sys->IsLooping(), "Cannot \"play\" looping particle system!");
	g_spriteRenderer->AddToLayer(sys, layer);
}


//-----------------------------------------------------------------------------------------------
ParticleSystem* ParticleSystemCreate(const std::string& name, const Vector2& systemLoc, ERenderLayer layer)
{
	ParticleSystem* sys = ParticleSystem::Create(name, systemLoc);
	ASSERT_OR_DIE(sys->IsLooping(), "Cannot \"Create\" non-looping particle system!");
	g_spriteRenderer->AddToLayer(sys, layer);

	return sys;
}


//-----------------------------------------------------------------------------------------------
void ParticleSystemDestroy(class ParticleSystem* system)
{
	system->MarkForDeletion();
}


//-----------------------------------------------------------------------------------------------
void ParticleSystemDestroyImmediate(class ParticleSystem* system)
{
	g_spriteRenderer->RemoveFromLayers(system);
	delete system;
}
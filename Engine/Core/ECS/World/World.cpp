#include "World.hpp"
#include <Core/DataStructure/Utils/Utils.hpp>
#include <Core/ECS/ECS/ECS.hpp>
#include <Core/ECS/System/BaseSystem.hpp>
#include <Core/ECS/Archetype/Chunk/ArchetypeChunk.hpp>

TRE_NS_START

World::World() : 
	m_EntityManager(this), m_SystemList(this), m_WorldId(ECS::DefaultWorld)
{
}

World::~World()
{
}

void World::UpdateSystems(float delta)
{
	usize systems_sz = m_SystemList.GetActiveSystemsCount();

	for (uint32 i = 0; i < systems_sz; i++) {
		BaseSystem& system = *m_SystemList[i];
		// const Bitset& sig = system.GetSignature();
		Archetype* arche = system.GetArchetype();
		
		if (arche && !arche->IsEmpty()) {
			system.UpdateComponents(delta, *arche);
		}
	}
}

TRE_NS_END
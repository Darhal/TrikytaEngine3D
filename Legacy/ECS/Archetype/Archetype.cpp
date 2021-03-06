#include "Archetype.hpp"
#include "Chunk/ArchetypeChunk.hpp"
#include <Legacy/Memory/Utils/Utils.hpp>
#include <Legacy/ECS/Entity/Entity.hpp>
#include <Legacy/ECS/Component/BaseComponent.hpp>
#include <Legacy/ECS/Archetype/Chunk/ArchetypeChunk.hpp>

TRE_NS_START

Archetype::Archetype(EntityManager* manager, const Bitset& bitset, const Vector<ComponentTypeID>& ids) :
	m_TypesToBuffer(), m_Signature(::std::move(bitset)),
	m_Manager(manager), m_OccupiedChunks(NULL), m_FreeChunks(NULL),
	m_TypesCount((uint32)ids.Size()), m_ComponentsArraySize(0), m_Id(0)
{
	// Calculate size for all the components
	for (const ComponentTypeID& id : ids) {
		m_TypesToBuffer.Emplace(id, m_ComponentsArraySize);
		m_ComponentsArraySize += BaseComponent::GetTypeSize(id) * ArchetypeChunk::CAPACITY;
	}
}

Archetype::Archetype(EntityManager* manager, const Vector<ComponentTypeID>& ids) :
	m_TypesToBuffer(), m_Signature(BaseComponent::GetComponentsCount()),
	m_Manager(manager), m_OccupiedChunks(NULL), m_FreeChunks(NULL),
	m_TypesCount((uint32)ids.Size()), m_ComponentsArraySize(0), m_Id(0)
{
	// Calculate size for all the components
	for (const ComponentTypeID& id : ids) {
		m_TypesToBuffer.Emplace(id, m_ComponentsArraySize);
		m_ComponentsArraySize += BaseComponent::GetTypeSize(id) * ArchetypeChunk::CAPACITY;
		m_Signature.Set(id, true);
	}
}

Archetype::Archetype(EntityManager* manager, const Bitset& bitset) :
	m_TypesToBuffer(), m_Signature(::std::move(bitset)),
	m_Manager(manager), m_OccupiedChunks(NULL), m_FreeChunks(NULL),
	m_TypesCount(0), m_ComponentsArraySize(0), m_Id(0)
{
	// Calculate size for all the components
	String str = Helper::ToString(m_Signature);
	for (uint32 id = 0; id < m_Signature.Length(); id++) {
		if (m_Signature.Get(id)) {
			m_TypesToBuffer.Emplace(id, m_ComponentsArraySize);
			m_ComponentsArraySize += BaseComponent::GetTypeSize(id) * ArchetypeChunk::CAPACITY;
			m_TypesCount++;
		}
	}
}

Archetype::~Archetype()
{
	ArchetypeChunk* next;

	while (m_FreeChunks) {
		next = m_FreeChunks;
		m_FreeChunks = m_FreeChunks->GetNextChunk();
		Free((uint8*)next);
	}

	while (m_OccupiedChunks) {
		next = m_OccupiedChunks;
		m_OccupiedChunks = m_OccupiedChunks->GetNextChunk();
		next->ArchetypeChunk::~ArchetypeChunk();
		Free((uint8*)next);
	}
}

EntityManager& Archetype::GetEntityManager() const 
{ 
	return *m_Manager; 
}

void Archetype::AddComponentType(ComponentTypeID id)
{
	m_Signature.Set(id, true);
}

ArchetypeChunk* Archetype::GenerateChunk()
{
	usize total_chunk_size = sizeof(ArchetypeChunk) + sizeof(EntityID) * ArchetypeChunk::CAPACITY + m_ComponentsArraySize;

	// Allocate
	uint8* total_buffer = Utils::Allocate<uint8>(total_chunk_size);
	uint8* comp_buffer_off = total_buffer + sizeof(ArchetypeChunk);
	ArchetypeChunk* temp_free_chunk = m_FreeChunks;
	m_FreeChunks = new (total_buffer) ArchetypeChunk(this, comp_buffer_off);
	m_FreeChunks->SetNextChunk(temp_free_chunk);
	return m_FreeChunks;
}

ArchetypeChunk* Archetype::GetAllocationChunk()
{
	if (m_OccupiedChunks && !m_OccupiedChunks->IsFull()) {
		return m_OccupiedChunks;
	} else {
		if (!m_FreeChunks) {
			this->GenerateChunk();
		}

		ArchetypeChunk* new_chunk = m_FreeChunks;
		m_FreeChunks = m_FreeChunks->GetNextChunk();
		new_chunk->SetNextChunk(m_OccupiedChunks);
		m_OccupiedChunks = new_chunk;
		return m_OccupiedChunks;
	}
}

ArchetypeChunk* Archetype::GetLastOccupiedChunk()
{
	return m_OccupiedChunks;
}

ArchetypeChunk* Archetype::AddEntity(Entity& entity)
{
	ArchetypeChunk* chunk = this->GetAllocationChunk();
	chunk->AddEntity(entity);
	return chunk;
}

ArchetypeChunk* Archetype::AddEntityComponents(Entity& entity, BaseComponent** components, const ComponentTypeID* componentIDs, usize numComponents)
{
	ArchetypeChunk* chunk = this->GetAllocationChunk();
	chunk->AddEntityComponents(entity, components, componentIDs, numComponents);
	return chunk;
}

void Archetype::PushFreeChunk(ArchetypeChunk* chunk)
{
	m_OccupiedChunks = chunk->GetNextChunk();
	chunk->SetNextChunk(m_FreeChunks);
	m_FreeChunks = chunk;
}

bool Archetype::HasComponentType(ComponentTypeID id) const
{
	return m_Signature.Get(id);
}

TRE_NS_END
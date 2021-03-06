#pragma once

#include <Core/Misc/Defines/Common.hpp>
#include <Core/DataStructure/Vector/Vector.hpp>
#include <Core/Misc/Maths/Matrix4x4.hpp>
#include <Core/Memory/Allocators/LinearAlloc/LinearAllocator.hpp>
#include <Renderer/Backend/Commands/Command.hpp>
#include <Renderer/Backend/Keys/BucketKey.hpp>
#include <Renderer/Backend/ResourcesManager/ResourcesManager.hpp>

TRE_NS_START

class CommandBucket;

class CommandPacket
{
public:	
	CommandPacket(CommandBucket* bucket, const BucketKey& key);

	template<typename U>
	U* SubmitCommand(const uint64& internal_key);

	template<typename U>
	U* CreateCommand(const uint64& internal_key);

	template<typename U>
	U* GetCommandFromID(uint32 id);

	void Flush();

	void SwapBuffer();

	const BucketKey& GetKey() const { return m_Key; };

	void SetKey(const BucketKey& key);

	void SortCommands();
private:
	LinearAllocator m_CmdsAllocator;
	CommandBucket* m_Bucket;
	BucketKey m_Key;
	Pair<uint64, Cmd>* m_Commands;
	uint32 m_CmdsCount;
	uint8 m_BufferMarker;

	CONSTEXPR static uint32 DEFAULT_MAX_ELEMENTS = 2048;
	CONSTEXPR static uint32 COMMAND_SIZE = 1024;
	CONSTEXPR static uint32 COMMAND_PTR  = sizeof(Pair<uint64, Cmd>);
	CONSTEXPR static uint32 TOTAL_SIZE   = DEFAULT_MAX_ELEMENTS * (COMMAND_SIZE + COMMAND_PTR);
	CONSTEXPR static uint32 MULTIPLIER   = 2;
};

template<typename U>
U* CommandPacket::SubmitCommand(const uint64& internal_key)
{
	ASSERTF(m_CmdsCount >= DEFAULT_MAX_ELEMENTS, "CommandPacket command buffer is full.");

	Cmd cmd = Command::CreateCommand<U>(m_CmdsAllocator);
	Command::StoreBackendDispatchFunction(cmd, U::DISPATCH_FUNCTION);
	uint32 index = m_BufferMarker * DEFAULT_MAX_ELEMENTS + m_CmdsCount++;
	new (&m_Commands[index]) Pair<uint64, Cmd>(internal_key, cmd);	
	return Command::GetCommand<U>(cmd);
}

template<typename U>
U* CommandPacket::CreateCommand(const uint64& internal_key)
{
	Cmd cmd = Command::CreateCommand<U>(m_CmdsAllocator);
	Command::StoreBackendDispatchFunction(cmd, U::DISPATCH_FUNCTION);
	return Command::GetCommand<U>(cmd);
}

template<typename U>
U* CommandPacket::GetCommandFromID(uint32 id)
{
	ASSERTF(id >= DEFAULT_MAX_ELEMENTS, "Command Index out of bound\n");

	const uint32 real_id = m_BufferMarker * DEFAULT_MAX_ELEMENTS + id;
	Cmd cmd = m_Commands[real_id].second;
	return (U*)Command::LoadCommand(cmd);
}

TRE_NS_END

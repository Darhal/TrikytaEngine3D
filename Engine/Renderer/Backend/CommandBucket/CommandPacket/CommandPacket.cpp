#include <algorithm>
#include "CommandPacket.hpp"
#include <Renderer/Backend/Keys/ModelKey.hpp>
#include <RenderAPI/VertexArray/VAO.hpp>
#include <Renderer/Materials/Material.hpp>
#include <Renderer/Backend/Commands/Commands.hpp>
#include <Renderer/Backend/CommandBucket/CommandBucket.hpp>
#include <bitset>

TRE_NS_START

CommandPacket::CommandPacket(CommandBucket* bucket, const BucketKey& key) :
	m_CmdsAllocator(TOTAL_SIZE * MULTIPLIER, true), m_Bucket(bucket),
	m_Key(key), m_CmdsCount(0), m_BufferMarker(0)
{
	m_Commands = (Pair<uint64, Cmd>*) m_CmdsAllocator.Allocate(DEFAULT_MAX_ELEMENTS * COMMAND_PTR * MULTIPLIER);
}

void CommandPacket::Flush()
{
	const uint32 start = m_BufferMarker * DEFAULT_MAX_ELEMENTS;
	const uint32 end = m_BufferMarker * DEFAULT_MAX_ELEMENTS + m_CmdsCount;

	for (uint32 i = start; i < end; i++) {
		const uint64& internal_key = m_Commands[i].first;
		Cmd cmd = m_Commands[i].second;

		// Issue the draw call
		const BackendDispatchFunction CommandFunction = Command::LoadBackendDispatchFunction(cmd);
		const void* command = Command::LoadCommand(cmd);
		CommandFunction(command);
	}
}

void CommandPacket::SetKey(const BucketKey& key)
{
	m_Key = key;
}

void CommandPacket::SwapBuffer()
{
	m_CmdsCount = 0;
	m_BufferMarker = !m_BufferMarker;
	m_CmdsAllocator.SetOffset(DEFAULT_MAX_ELEMENTS * COMMAND_PTR * MULTIPLIER + DEFAULT_MAX_ELEMENTS * COMMAND_SIZE * m_BufferMarker);
}

void CommandPacket::SortCommands()
{
	/*std::bitset<64> b(m_Key);
	printf("Sorting packt with key : %llu | Bitset : ", m_Key); std::cout << b << std::endl;*/
	const uint32 start = m_BufferMarker * DEFAULT_MAX_ELEMENTS;
	// const uint32 end = m_BufferMarker * DEFAULT_MAX_ELEMENTS + m_CmdsCount;

	std::qsort(m_Commands + start, m_CmdsCount, sizeof(Pair<BucketKey, Cmd>), [](const void* a, const void* b) {
		const Pair<BucketKey, Cmd>& arg1 = *static_cast<const Pair<BucketKey, Cmd>*>(a);
		const Pair<BucketKey, Cmd>& arg2 = *static_cast<const Pair<BucketKey, Cmd>*>(b);

		if (arg1.first < arg2.first) return -1;
		if (arg1.first > arg2.first) return 1;
		return 0;
	});
}

TRE_NS_END
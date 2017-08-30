#pragma once

#include "Engine/Network/NetworkSystem.hpp"
#include "Engine/Network/NetMessage.hpp"
#include "Engine/Network/VoiceChatSystem.hpp"
#include "Quantum/Core/String.h"

#include <string>
#include <deque>
#include <set>
#include <vector>
#include <map>


//-----------------------------------------------------------------------------------------------
#define MAX_NUM_RELEVANT_ACK_BUNDLES 128


//-----------------------------------------------------------------------------------------------
#define INVALID_PLAYER_INDEX 0xFF
#define HOST_INDEX 0x00;


//-----------------------------------------------------------------------------------------------
enum ENetConnectionType
{
	NETCONNECTIONTYPE_LOCAL,
	NETCONNECTIONTYPE_UNCONFIRMED,
	NETCONNECTIONTYPE_CONFIRMED,
	NETCONNECTIONTYPE_BAD
};


//-----------------------------------------------------------------------------------------------
struct AckBundle
{
	ushort ackID;
	std::vector<ushort> reliableIDs;
};


//-----------------------------------------------------------------------------------------------
class NetConnection
{
	friend class NetSession;

	static const float TIME_UNTIL_HEARTBEAT;
	static const float TIME_UNTIL_MARKED_BAD;
	static const float TIME_UNTIL_DISCONNECTED;
public:
	NetConnection(byte index, const std::string& guid, const sockaddr_in& addr);
	byte GetIndex() const { return m_index; }
	bool ConstructPacketAndSend(byte playerIndex);
	void AddMessage(NetMessage& message);
	bool IsMe() const;
	bool IsValid() const { return m_guid != ""; }
	void UpdateAcksAndStatus(const class NetPacket& packet);
	AckBundle& GetAckForID(ushort ackID);
	void RemoveReceivedReliablesForBundle(const AckBundle& bundle);
	bool UpdateExpectedReliablesAndCheckShouldProcess(ushort reliableID);
	void CheckShouldSendHeartbeat(float deltaSeconds);
	QuString GetDebugString() const;
	const char* GetGUID() const { return m_guid.c_str(); }
	void SetGUID(const char* guid) { m_guid = guid; }
	void SetIndex(byte playerIndex) { m_index = playerIndex; }
	bool IsHost() const { return m_index == HOST_INDEX; }
	void InitializeVoiceSystem();
//	float GetVoiceTime() const { return m_voiceChatSystem.GetTime(); }
//	void SetVoiceSourceTime(float sourceTime) { m_voiceChatSystem.SetSourceTime(sourceTime); }
//	void HandleVoiceMessage(NetMessage& msg) { m_lastReceivedTimestamp = m_voiceChatSystem.HandleMessage(msg); }
//	void OnSoundBiteComplete(Event* e);

	//For starting connections from reliable connectionless join requests.  This way, we won't process the request more than once
	void StartFromReliableID(ushort firstReliableID) { m_nextExpectedReliableID = firstReliableID + 1; }

private:
	void PushReliablesIntoPacket(NetPacket& packet, struct PacketHeader* header);
	void PushUnreliablesIntoPacket(NetPacket& packet);
	ushort GetBundleIndexForID(ushort ackIndex);

private:
	//This constructor should only be used by the NetSession for its own connection
	NetConnection() {}
	byte m_index;
	std::string m_guid;
	sockaddr_in m_toAddr;
	ENetConnectionType m_type;
	ushort m_previousReceivedAckBitfield;
	ushort m_highestReceivedAck;
	ushort m_currentAckIndex;
	ushort m_currentReliableID;
	ushort m_currentSequenceID;

	//Would prefer queues here, but I want to iterate and change the data structure based on received reliables
	std::deque<NetMessage> m_unconfirmedReliables;
	std::deque<NetMessage> m_unsentUnreliables;


	AckBundle m_ackBundles[MAX_NUM_RELEVANT_ACK_BUNDLES];
	byte m_currentZerothAckBundleIndex;
	ushort m_nextExpectedReliableID;
	std::set<ushort> m_receivedReliablesPastExpected;

	//Using a map here because we can sort by ID but have a message value, and it's in order, so we can just poll the beginning
	std::map<ushort, NetMessage> m_outOfOrderMessages;
	ushort m_nextExpectedSequenceID;

	float m_timeSinceLastReceivedPacket;
	float m_timeSinceLastSentPacket;
	float m_timeSinceLastHeartbeatSent;

//	VoiceChatSystem m_voiceChatSystem;
	ushort m_lastReceivedTimestamp;
};
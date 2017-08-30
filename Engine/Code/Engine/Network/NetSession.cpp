#include "Engine/Network/NetSession.hpp"
#include "Engine/Core/ConsoleCommand.hpp"
#include "Engine/Network/NetPacket.hpp"
#include "Engine/Core/ErrorWarningAssert.hpp"
#include "Engine/Core/EventSystem.hpp"


//-----------------------------------------------------------------------------------------------
static const int NS_GAME_PORT = 4334;
STATIC const float NetSession::NETWORK_TICK_INTERVAL = 1.f / 60.f;
STATIC const float NetSession::TIME_UNTIL_JOIN_TIMEOUT = 15.f;


//-----------------------------------------------------------------------------------------------
NetSession* g_netSession = nullptr;


//-----------------------------------------------------------------------------------------------
NetSession::NetSession()
	: m_packetChannel(nullptr)
	, m_state(NETSESSIONSTATE_INVALID)
	, m_lastError(NETERROR_NONE)
	, m_isHost(false)
{
	g_eventSystem->RegisterEvent<NetSession, &NetSession::Tick>("Tick", this);
}


//-----------------------------------------------------------------------------------------------
NetSession::~NetSession()
{
	g_eventSystem->UnregisterFromAllEvents(this);
	SAFE_DELETE(m_packetChannel);

	for (NetMessageDef* def : m_definitions)
	{
		SAFE_DELETE(def);
	}

	for (NetConnection* nc : m_activeConnections)
	{
		delete nc;
	}

	delete m_myConnection;
}


//-----------------------------------------------------------------------------------------------
bool NetSession::Start(const int portNum, const int maxDistanceFromPort /* = 8 */)
{
	m_timeSinceLastNetworkTick = 0.f;
	m_timeSinceLastPacketReceived = 0.f;
	m_timeSinceLastPacketSent = 0.f;
	for (int currPort = portNum; currPort < portNum + maxDistanceFromPort; currPort++)
	{
		char portString[6];
		Network::GetPortString(currPort, portString);
		m_myConnection = new NetConnection();
		m_myConnection->m_index = INVALID_CONNECTION_INDEX;
		SOCKET sock = Network::CreateUDPSocket(Network::GetLocalHostName(), portString, &m_myConnection->m_toAddr);
		if (sock != INVALID_SOCKET)
		{
			m_packetChannel = new PacketChannel(sock);
			m_myConnection->m_type = NETCONNECTIONTYPE_LOCAL;
			m_state = NETSESSIONSTATE_DISCONNECTED;
			for (NetConnection* conn : m_activeConnections)
			{
				SAFE_DELETE(conn);
			}
			m_activeConnections.clear();
			return true;
		}
		else
		{
			delete m_myConnection;
		}
	}

	m_lastError = NETERROR_SOCKET_CREATE_FAILED;
	return false;
}


//-----------------------------------------------------------------------------------------------
void NetSession::Stop()
{
	SAFE_DELETE(m_packetChannel);
	m_state = NETSESSIONSTATE_INVALID;
}


//-----------------------------------------------------------------------------------------------
std::string NetSession::GetAddressString()
{
	char port[6];
	getnameinfo((sockaddr*)&m_myConnection->m_toAddr, sizeof(sockaddr_in), nullptr, 0, port, 6, 0);
	return Stringf("%s:%s", Network::GetStringFromAddr((sockaddr*)&m_myConnection->m_toAddr), port);
}


//-----------------------------------------------------------------------------------------------
void NetSession::SendMessageDirect(const sockaddr_in* dest, NetMessage& msg)
{
	NetConnection* conn = FindConnectionWithAddr(*dest);
	if (conn)
	{
		conn->AddMessage(msg);
		conn->ConstructPacketAndSend(0);
	}
	else
	{
		NetPacket packet;
		packet.WriteContents(msg);
		SendPacketDirect(dest, packet);
	}
}


//-----------------------------------------------------------------------------------------------
void NetSession::SendPacketDirect(const sockaddr_in* dest, const NetPacket& packet)
{
	const char* buff = packet.GetCopyableBuffer();
	int len = packet.GetLength();
	m_timeSinceLastPacketSent = 0.f;
	m_packetChannel->SendTo(buff, len, 0, dest);
}


//-----------------------------------------------------------------------------------------------
void NetSession::Tick(Event* e)
{
	TickEvent* te = (TickEvent*)e;

	float deltaSeconds = te->deltaSeconds;

	m_timeSinceLastNetworkTick += deltaSeconds;
	m_timeSinceLastPacketReceived += deltaSeconds;
	m_timeSinceLastPacketSent += deltaSeconds;
	m_joinAttemptTime += deltaSeconds;
	if (m_state == NETSESSIONSTATE_JOINING && m_joinAttemptTime >= TIME_UNTIL_JOIN_TIMEOUT)
	{
		m_lastError = NETERROR_JOIN_HOST_TIMEOUT;
		m_state = NETSESSIONSTATE_DISCONNECTED;

		for (NetConnection* conn : m_activeConnections)
		{
			SAFE_DELETE(conn);
		}
		m_activeConnections.clear();
	}

	for (auto iter = m_activeConnections.begin(); iter != m_activeConnections.end();)
	{
		NetConnection* conn = *iter;
		conn->m_timeSinceLastReceivedPacket += deltaSeconds;
		conn->m_timeSinceLastSentPacket += deltaSeconds;
		conn->CheckShouldSendHeartbeat(deltaSeconds);

		if (conn->m_type == NETCONNECTIONTYPE_CONFIRMED)
		{
			if (conn->m_timeSinceLastReceivedPacket > NetConnection::TIME_UNTIL_MARKED_BAD)
			{
				conn->m_type = NETCONNECTIONTYPE_BAD;
			}
		}
		else if (conn->m_type == NETCONNECTIONTYPE_BAD)
		{
			if (conn->m_timeSinceLastReceivedPacket > NetConnection::TIME_UNTIL_DISCONNECTED)
			{
				if (conn->IsHost())
				{
					m_lastError = NETERROR_HOST_DISCONNECTED;
				}
				ConsolePrintf(RED, "Connection with %s timed out", Network::GetFullStringFromAddr((sockaddr*)&conn->m_toAddr));
				SAFE_DELETE(conn);
				iter = m_activeConnections.erase(iter);
				continue;
			}
		}
		iter++;
	}

	Update();
}


//-----------------------------------------------------------------------------------------------
void NetSession::Update()
{
	ProcessPackets();

	if (m_timeSinceLastNetworkTick < NETWORK_TICK_INTERVAL)
	{
		return;
	}

	m_timeSinceLastNetworkTick = 0.f;

	NetworkTickEvent nte;
	nte.connection = m_myConnection;

	g_eventSystem->TriggerEvent("OnNetworkTick", &nte);

	for (NetConnection* nc : m_activeConnections)
	{
		NetworkTickEvent nten;
		nten.connection = nc;
		g_eventSystem->TriggerEvent("OnNetworkTick", &nten);
	}
}


//-----------------------------------------------------------------------------------------------
void NetSession::RegisterMessage(ENetMessage type, const char* debugName, OnMessageReceiveFunc callback)
{
	ASSERT_OR_DIE(m_state == NETSESSIONSTATE_INVALID, "Can't register message to an initialized session");
	NetMessageDef* def = new NetMessageDef();

	def->debugName = debugName;
	def->callback = callback;

	if (m_definitions.size() < type + 1U)
	{
		m_definitions.resize(type + 1, nullptr);
	}
	m_definitions[type] = def;
}


//-----------------------------------------------------------------------------------------------
static void OnPingReceived(NetSender& sender, NetMessage& msg)
{
	char buffer[256];
	bool result = msg.Read<char* const>(buffer);
	std::string toPrint = (result && strlen(buffer) > 0) ? buffer : "null";

	ConsolePrintf(WHITE, "Ping received from %s. [%s]", Network::GetFullStringFromAddr(&sender.address), toPrint.c_str());

	NetMessage pong(NETMESSAGE_PONG);
	pong.SetFlag(NETMESSAGEFLAG_CONNECTIONLESS);
	pong.Write<const char*>(toPrint.c_str());
	sender.session->SendMessageDirect((sockaddr_in*)&sender.address, pong);
}


//-----------------------------------------------------------------------------------------------
static void OnPongReceived(NetSender& sender, NetMessage& msg)
{
	char buffer[256];
	bool result = msg.Read<char* const>(buffer);
	std::string toPrint = (result && strlen(buffer) > 0) ? buffer : "null";

	ConsolePrintf(WHITE, "Pong received from %s. [%s]", Network::GetFullStringFromAddr(&sender.address), toPrint.c_str());
}


//-----------------------------------------------------------------------------------------------
static void OnJoinRequest(NetSender& sender, NetMessage& msg)
{
	ASSERT_OR_DIE(!sender.connection, "Cannot accept join from preexisting connection!");

	if (!g_netSession->IsHost())
	{
		NetMessage notHost(NETMESSAGE_JOIN_DENY);
		notHost.Write<ENetErrorType>(NETERROR_JOIN_DENIED_NOT_HOST);
		g_netSession->SendConnectionlessReliableResponseWithMessage(notHost, sender.ackID, sender.address);
		return;
	}

	char buffer[256];
	msg.Read<char* const>(buffer);

	if (g_netSession->IsGUIDTaken(buffer))
	{
		NetMessage guidTaken(NETMESSAGE_JOIN_DENY);
		guidTaken.Write<ENetErrorType>(NETERROR_JOIN_DENIED_GUID_IN_USE);
		g_netSession->SendConnectionlessReliableResponseWithMessage(guidTaken, sender.ackID, sender.address);
		return;
	}

	byte playerIndex = g_netSession->GetValidPlayerIndex();

	sockaddr_in* addr = (sockaddr_in*)&sender.address;

	NetConnection* conn = new NetConnection(playerIndex, buffer, *addr);
	g_netSession->AddConnection(conn);
	conn->StartFromReliableID(msg.GetReliableID());

	NetMessage joinAccept(NETMESSAGE_JOIN_ACCEPT);
	joinAccept.SetFlag(NETMESSAGEFLAG_RELIABLE);
	joinAccept.Write<const char*>(g_netSession->GetOwnConnection()->GetGUID());
	joinAccept.Write<byte>(playerIndex);
	conn->AddMessage(joinAccept);

	conn->InitializeVoiceSystem();

	NetMessage voiceInit(NETMESSAGE_VOICE_SYNC);
	voiceInit.SetFlag(NETMESSAGEFLAG_RELIABLE);
//	voiceInit.Write<float>(conn->GetVoiceTime());
	conn->AddMessage(voiceInit);

	ConnectionChangeEvent cce;
	cce.connection = conn;
	g_eventSystem->TriggerEvent("OnConnectionJoin", &cce);
}
TODO("Implement no new connections deny and full deny");


//-----------------------------------------------------------------------------------------------
static void OnJoinDeny(NetSender& sender, NetMessage& msg)
{
	sender.connection = nullptr;
	ENetErrorType error;
	msg.Read<ENetErrorType>(error);

	g_netSession->SetError(error);

	g_netSession->OnJoinFail();
}


//-----------------------------------------------------------------------------------------------
static void OnJoinAccept(NetSender& sender, NetMessage& msg)
{
	ASSERT_OR_DIE(sender.connection, "Should have created a stub host by this point");

	char buffer[256];
	msg.Read<char* const>(buffer);
	sender.connection->SetGUID(buffer);

	sender.connection->InitializeVoiceSystem();

	NetMessage voiceInit(NETMESSAGE_VOICE_SYNC);
	voiceInit.SetFlag(NETMESSAGEFLAG_RELIABLE);
//	voiceInit.Write<float>(sender.connection->GetVoiceTime());
	sender.connection->AddMessage(voiceInit);

	byte playerIndex;
	msg.Read<byte>(playerIndex);
	NetConnection* ownConnection = g_netSession->GetOwnConnection();
	ownConnection->SetIndex(playerIndex);

	g_netSession->OnJoin();
}


//-----------------------------------------------------------------------------------------------
static void OnConnectionLeave(NetSender& sender, NetMessage& msg)
{
	UNUSED(msg);

	if (!sender.connection)
	{
		return;
	}
	ConsolePrintf(RED, "Connection at %s left", Network::GetFullStringFromAddr(&sender.address));


	if (sender.connection->IsHost())
	{
		ConsolePrint("Was host.  Purging connections", RED);
		g_netSession->Leave();
	}
	else
	{
		ConnectionChangeEvent cce;
		cce.connection = sender.connection;
		g_eventSystem->TriggerEvent("OnConnectionLeave", &cce);
		g_netSession->RemoveConnection(sender.connection);
	}
	sender.connection = nullptr;
}


//-----------------------------------------------------------------------------------------------
static void OnVoiceSync(NetSender& sender, NetMessage& msg)
{
	if (!sender.connection)
	{
		return;
	}

	float sourceTime;
	msg.Read<float>(sourceTime);
//	sender.connection->SetVoiceSourceTime(sourceTime);
}


//-----------------------------------------------------------------------------------------------
static void OnVoiceReceived(NetSender& sender, NetMessage& msg)
{
	if (!sender.connection)
	{
		return;
	}
	
//	sender.connection->HandleVoiceMessage(msg);
}


//-----------------------------------------------------------------------------------------------
void NetSession::RegisterCoreMessages()
{
	RegisterMessage(NETMESSAGE_PING, "ping", OnPingReceived);
	RegisterMessage(NETMESSAGE_PONG, "pong", OnPongReceived);
	RegisterMessage(NETMESSAGE_HEARTBEAT, "heartbeat", [](NetSender& sender, NetMessage& msg) { UNUSED(sender, msg); });
	RegisterMessage(NETMESSAGE_JOIN_REQUEST, "joinrequest", OnJoinRequest);
	RegisterMessage(NETMESSAGE_JOIN_DENY, "joindeny", OnJoinDeny);
	RegisterMessage(NETMESSAGE_JOIN_ACCEPT, "joinaccept", OnJoinAccept);
	RegisterMessage(NETMESSAGE_LEAVE, "leave", OnConnectionLeave);
	RegisterMessage(NETMESSAGE_VOICE_SYNC, "voiceinit", OnVoiceSync);
	RegisterMessage(NETMESSAGE_VOICE_CHUNK, "voice", OnVoiceReceived);

}


//-----------------------------------------------------------------------------------------------
bool NetSession::IsMe(const sockaddr_in& otherAddr) const
{
	return Network::AreSameAddress(otherAddr, m_myConnection->m_toAddr);
}


//-----------------------------------------------------------------------------------------------
void NetSession::AddConnection(NetConnection* nc)
{
	m_activeConnections.push_back(nc);
	nc->m_type = NETCONNECTIONTYPE_UNCONFIRMED;
}


//-----------------------------------------------------------------------------------------------
void NetSession::RemoveConnection(NetConnection* nc)
{
	for (auto iter = m_activeConnections.begin(); iter != m_activeConnections.end(); iter++)
	{
		if (*iter == nc)
		{
			m_activeConnections.erase(iter);
			break;
		}
	}

	SAFE_DELETE(nc);
}


//-----------------------------------------------------------------------------------------------
bool NetSession::DoesConnectionExist(const NetConnection* nc)
{
	if (Network::AreSameAddress(nc->m_toAddr, m_myConnection->m_toAddr))
	{
		return true;
	}

	for (NetConnection* conn : m_activeConnections)
	{
		if (Network::AreSameAddress(nc->m_toAddr, conn->m_toAddr))
		{
			return true;
		}
	}

	return false;
}


//-----------------------------------------------------------------------------------------------
bool NetSession::IsIndexTaken(byte index) const
{
	if (index == m_myConnection->m_index)
	{
		return true;
	}

	for (NetConnection* conn : m_activeConnections)
	{
		if (index == conn->m_index)
		{
			return true;
		}
	}

	return false;
}


//-----------------------------------------------------------------------------------------------
bool NetSession::IsGUIDTaken(const std::string& guid)
{
	if (guid == m_myConnection->m_guid)
	{
		return true;
	}

	for (NetConnection* conn : m_activeConnections)
	{
		if (guid == conn->m_guid)
		{
			return true;
		}
	}

	return false;
}


//-----------------------------------------------------------------------------------------------
void NetSession::DestroyConnection(byte index)
{
	if (index == m_myConnection->m_index)
	{
		m_myConnection->m_index = INVALID_CONNECTION_INDEX;
		m_myConnection->m_guid = "";
		return;
	}

	for (auto iter = m_activeConnections.begin(); iter != m_activeConnections.end();)
	{
		NetConnection* nc = *iter;
		if (nc->m_index == index)
		{
			delete nc;
			iter = m_activeConnections.erase(iter);
		}
		else
		{
			iter++;
		}
	}
}


//-----------------------------------------------------------------------------------------------
void NetSession::SetOwnConnectionInfo(int index, const std::string& guid)
{
	if (!m_myConnection)
	{
		ConsolePrint("My connection does not exist", RED);
		return;
	}

	m_myConnection->m_index = (byte)index;
	m_myConnection->m_guid = guid;
}


//-----------------------------------------------------------------------------------------------
NetConnection* NetSession::GetConnectionAtIndex(byte index) const
{
	if (index == m_myConnection->m_index)
	{
		return m_myConnection;
	}

	for (NetConnection* nc : m_activeConnections)
	{
		if (index == nc->m_index)
		{
			return nc;
		}
	}

	return nullptr;
}


//-----------------------------------------------------------------------------------------------
NetConnection* NetSession::FindConnectionWithAddr(const sockaddr_in& address) const
{
	if (IsMe(address))
	{
		return m_myConnection;
	}

	for (NetConnection* conn : m_activeConnections)
	{
		if (Network::AreSameAddress(address, conn->m_toAddr))
		{
			return conn;
		}
	}

	return nullptr;
}


//-----------------------------------------------------------------------------------------------
QuString NetSession::GetDebugString() const
{
	QuString result = IsHost() ? "Host\n" : "";

	result += QuString::F("Time since packet received: %.2fs\n", m_timeSinceLastPacketReceived);
	result += QuString::F("Time since packet sent: %.2fs\n", m_timeSinceLastPacketSent);
	if (m_packetChannel)
	{
		result += "Session bound to: ";
		result += Network::GetFullStringFromAddr((sockaddr*)&m_myConnection->m_toAddr);
		result += "\n";
		result += m_packetChannel->GetDebugString();
	}

	for (NetConnection* conn : m_activeConnections)
	{
		result += conn->GetDebugString();
	}

	return result;
}


//-----------------------------------------------------------------------------------------------
void NetSession::SendConnectionlessReliableResponseWithMessage(NetMessage& toSend, ushort ackID, const sockaddr& returnAddress)
{
	NetPacket packet;
	PacketHeader* header = packet.GetPacketHeader();
	header->mostRecentReceivedAck = ackID;
	header->previousReceivedAckBitfield = (1 << (BITS_PER_ACK_FIELD - 1));
	
	packet.WriteContents(toSend);

	SendPacketDirect((sockaddr_in*)&returnAddress, packet);
}


//-----------------------------------------------------------------------------------------------
byte NetSession::GetValidPlayerIndex() const
{
	//Hardcoding, since these are bytes and host is always 0
	for (byte i = 0; i < 256; i++)
	{
		if (!IsIndexTaken(i))
		{
			return i;
		}
	}

	return INVALID_PLAYER_INDEX;
}


//-----------------------------------------------------------------------------------------------
void NetSession::OnJoinFail()
{
	m_state = NETSESSIONSTATE_DISCONNECTED;
	FlushConnections();
}


//-----------------------------------------------------------------------------------------------
void NetSession::OnJoin()
{
	m_state = NETSESSIONSTATE_CONNECTED;

	ConnectionChangeEvent cce;
	cce.connection = m_myConnection;
	g_eventSystem->TriggerEvent("OnConnectionJoin", &cce);

	for (NetConnection* conn : m_activeConnections)
	{
		cce.connection = conn;
		g_eventSystem->TriggerEvent("OnConnectionJoin", &cce);
	}
}


//-----------------------------------------------------------------------------------------------
void NetSession::FlushConnections()
{
	ConnectionChangeEvent cce;
	cce.connection = GetOwnConnection();
	g_eventSystem->TriggerEvent("OnConnectionLeave", &cce);
	for (NetConnection* conn : m_activeConnections)
	{
		cce.connection = conn;
		g_eventSystem->TriggerEvent("OnConnectionLeave", &cce);
		SAFE_DELETE(conn);
	}
	m_activeConnections.clear();
}


//-----------------------------------------------------------------------------------------------
void NetSession::Host(const char* username)
{
	ASSERT_OR_DIE(m_state == NETSESSIONSTATE_DISCONNECTED, "Can't host unless valid and disconnected");
	m_state = NETSESSIONSTATE_HOSTING;

	ASSERT_OR_DIE(m_myConnection, "Should always have a connection for yourself");

	m_myConnection->m_guid = username;
	m_myConnection->m_index = 0;

	m_state = NETSESSIONSTATE_CONNECTED;
	m_isHost = true;
}


//-----------------------------------------------------------------------------------------------
void NetSession::Join(const char* username, const sockaddr_in& addr)
{
	ASSERT_OR_DIE(m_state == NETSESSIONSTATE_DISCONNECTED, "Can't join unless valid and disconnected");
	m_state = NETSESSIONSTATE_JOINING;
	m_joinAttemptTime = 0.f;

	NetConnection* conn = FindConnectionWithAddr(addr);
	ASSERT_OR_DIE(!conn, "Cannot join when connection for host already exists");

	SetOwnConnectionInfo(INVALID_CONNECTION_INDEX, username);
	conn = new NetConnection(0, "", addr);
	AddConnection(conn);

	NetMessage joinRequest(NETMESSAGE_JOIN_REQUEST);
	joinRequest.SetFlag(NETMESSAGEFLAG_RELIABLE);
	joinRequest.SetFlag(NETMESSAGEFLAG_CONNECTIONLESS);
	joinRequest.Write<const char*>(username);
	conn->AddMessage(joinRequest);
}


//-----------------------------------------------------------------------------------------------
void NetSession::Leave()
{
	for (NetConnection* conn : m_activeConnections)
	{
		NetMessage leave(NETMESSAGE_LEAVE);
		conn->AddMessage(leave);
		conn->ConstructPacketAndSend(GetOwnConnection()->GetIndex());
	}
	FlushConnections();

	m_state = NETSESSIONSTATE_DISCONNECTED;
	m_isHost = false;
}


//-----------------------------------------------------------------------------------------------
void NetSession::ProcessPackets()
{
	NetPacket packet(false);
	NetSender from;
	from.session = this;

	while (ReadNextPacket(&packet, &from.address))
	{
		PacketHeader* header = packet.GetPacketHeader();
		from.ackID = header->thisAck;
		m_timeSinceLastPacketReceived = 0.f;
		from.connection = FindConnectionWithAddr(*(sockaddr_in*)(&from.address));
		if (from.connection)
		{
			from.connection->UpdateAcksAndStatus(packet);
		}
		NetMessage msg;
		while (ReadNextMessage(&msg, packet, from.connection))
		{
			const NetMessageDef* def = GetDefinition(msg.m_type);
			if (!def)
			{
				ConsolePrint("Bad packet.  Contains unsupported message type", RED);
				break;
			}
			//This one's redundant and clunky to receive per frame.  So, just using it to debug when I need it
			ConsolePrintf(WHITE, "Received '%s' message", def->debugName);
			def->callback(from, msg);
			msg.Reset();
		}
		packet.Reset();
	}
}


//-----------------------------------------------------------------------------------------------
bool NetSession::ReadNextPacket(NetPacket* packet, sockaddr* addr)
{
	sockaddr_storage stor;
	int addrlen = sizeof(sockaddr_storage);
	int recvResult = m_packetChannel->RecvFrom(packet->GetBuffer(), UDP_PACKET_MAX_LENGTH, 0, (sockaddr_in*)&stor, &addrlen);

	if (recvResult <= 0)
	{
		return false;
	}

	packet->Initialize(recvResult);
	memcpy(addr, &stor, sizeof(sockaddr));
	return true;
}


//-----------------------------------------------------------------------------------------------
bool NetSession::ReadNextMessage(NetMessage* msg, NetPacket& packet, NetConnection* connection)
{
	if (connection)
	{
		//Check for ordered messages we can process now
		auto iter = connection->m_outOfOrderMessages.begin();
		if (iter != connection->m_outOfOrderMessages.end())
		{
			ushort sequenceID = iter->first;
			if (sequenceID == connection->m_nextExpectedSequenceID)
			{
				*msg = iter->second;
				connection->m_outOfOrderMessages.erase(iter);
				connection->m_nextExpectedSequenceID++;
				return true;
			}
		}
	}
	if (packet.m_remainingMessages == 0)
	{
		return false;
	}

	//I think I finally found the place for a goto.  YAAAAAYYYYY!!!!
readmessage:
	MessageHeader* header = packet.ReadMessageHeaderAndAdvance();
	uint16_t messageSize = header->messageSize;
	msg->m_type = header->type;
	msg->m_flags = header->flags;
	msg->m_reliableID = header->reliableID;
	msg->m_sequenceID = header->sequenceID;
	//Decreasing message size during advance so it only reflects payload
	messageSize -= sizeof(MessageHeader);

	packet.ReadContents(msg->m_buffer, messageSize);

	if (msg->IsReliable())
	{
		if (connection)
		{
			if (!connection->UpdateExpectedReliablesAndCheckShouldProcess(msg->m_reliableID))
			{
				return false;
			}
		}
	}
	if (msg->IsOrdered())
	{
		if (!connection)
		{
			return false;
		}

		//Shouldn't have to check if the sequenceID is too low, cause it will have been processed and discarded
		if (connection->m_nextExpectedSequenceID != msg->m_sequenceID)
		{
			//We can't process this message now, so we add it to the pending list and try again
			connection->m_outOfOrderMessages.insert(std::make_pair(msg->m_sequenceID, *msg));

			//Using a goto because the only reason to ever repeat the above process is to pass several checks
			//It seemed like the cleanest way to do it
			goto readmessage;
		}
		else
		{
			//Advance the sequence ID so we can process the next one
			connection->m_nextExpectedSequenceID++;
		}
	}
	//A message that requires a connection will be read but not processed if the connection does not exist
	if (!msg->CheckFlag(NETMESSAGEFLAG_CONNECTIONLESS))
	{
		if (!connection)
		{
			return false;
		}
	}
	

	return true;
}


//-----------------------------------------------------------------------------------------------
CONSOLE_COMMAND(NSPing, args)
{
	if (!g_netSession)
	{
		return;
	}

	sockaddr_in dest;
	bool success = Network::GetAddrFromString(args.GetNextArg().c_str(), &dest);
	if (!success)
	{
		ConsolePrint("Provided address was invalid", RED);
		return;
	}

	std::string messageText = args.GetRemainingArgString();
	NetMessage msg(NETMESSAGE_PING);
	msg.SetFlag(NETMESSAGEFLAG_CONNECTIONLESS);
	msg.Write<std::string>(messageText);

	g_netSession->SendMessageDirect(&dest, msg);
}


//-----------------------------------------------------------------------------------------------
CONSOLE_COMMAND(NSSimLag, args)
{
	if (!g_netSession)
	{
		return;
	}

	std::string minLag = args.GetNextArg();

	if (minLag == "")
	{
		ConsolePrint("Must supply at least one argument", RED);
		return;
	}

	int minMilliseconds;
	try
	{
		minMilliseconds = std::stoi(minLag);
	}
	catch (const std::exception&)
	{
		ConsolePrint("Must supply lag", RED);
		return;
	}

	std::string maxLag = args.GetNextArg();

	if (maxLag == "")
	{
		g_netSession->SetLag(minMilliseconds, minMilliseconds);
		return;
	}

	int maxMilliseconds;
	try
	{
		maxMilliseconds = std::stoi(maxLag);
	}
	catch (const std::exception&)
	{
		ConsolePrintf(RED, "Bad arg: %s", maxLag.c_str());
		return;
	}

	g_netSession->SetLag(minMilliseconds, maxMilliseconds);
}


//-----------------------------------------------------------------------------------------------
CONSOLE_COMMAND(NSSimLoss, args)
{
	if (!g_netSession)
	{
		return;
	}

	std::string lossPercentage = args.GetNextArg();
	if (lossPercentage == "")
	{
		ConsolePrint("Must supply loss percentage", RED);
		return;
	}

	int lossPercent;
	try
	{
		lossPercent = std::stoi(lossPercentage);
	}
	catch (const std::exception&)
	{
		ConsolePrintf(RED, "Bad argument: %s", lossPercentage.c_str());
		return;
	}

	lossPercent = Clampi(lossPercent, 0, 100);

	g_netSession->SetLoss((float)lossPercent * .01f);
}
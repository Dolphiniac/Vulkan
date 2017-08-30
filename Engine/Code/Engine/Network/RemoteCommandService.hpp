#pragma once

#include <vector>
#include <string>


//-----------------------------------------------------------------------------------------------
extern class RemoteCommandService* g_remoteCommandService;


//-----------------------------------------------------------------------------------------------
class RemoteCommandService
{
public:
	void Update();
	void SetListener(class TCPListener* listener) { m_listener = listener; }
	void AddConnection(class TCPConnection* connection) { m_activeConnections.push_back(connection); }
	std::vector<std::string> GetCommandServiceInfo();
	void SendCommand(const std::string& command);
	~RemoteCommandService();

private:
	class TCPListener* m_listener;
	std::vector<class TCPConnection*> m_activeConnections;

};
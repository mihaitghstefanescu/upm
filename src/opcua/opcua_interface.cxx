#include "opcua_interface.hpp"

using namespace upm;

OPCUA::OPCUA()
{
  m_client = UA_Client_new(UA_ClientConfig_standard);
}

OPCUA::~OPCUA()
{
  UA_Client_delete(m_client);
}

OPCUA::status_code OPCUA::connect(std::string server_addr)
{
  status_code sc = UA_STATUSCODE_GOOD;

  sc = UA_Client_connect(m_client, server_addr.c_str());

  return sc;
}

void OPCUA::disconnect(bool deleteNodes)
{
  if (deleteNodes)
  {
    for (auto const& node : m_nodes)
    {
      UA_Client_deleteNode(m_client, node.second, UA_TRUE);
    }
  }

  UA_Client_disconnect(m_client);
}

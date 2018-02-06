#pragma once

#include <stdexcept>
#include <string>
#include <map>
#include <iostream>

#include "open62541.h"

namespace upm {
  /* Generic map of types - OPC_UA enums.
   * NOTE: Not completed yet.*/
  template<typename OPC_TYPE>
  struct OpcTypeMap;

  template <typename OPC_TYPE> struct OpcTypeMap { static const int value = -1; }; // Bad or Unknown type.
  template <> struct OpcTypeMap<UA_Boolean> { static const int value = UA_TYPES_BOOLEAN; };
  template <> struct OpcTypeMap<UA_SByte> { static const int value = UA_TYPES_SBYTE; };
  template <> struct OpcTypeMap<UA_Byte> { static const int value = UA_TYPES_BYTE; };
  template <> struct OpcTypeMap<UA_Int16> { static const int value = UA_TYPES_INT16; };
  template <> struct OpcTypeMap<UA_UInt16> { static const int value = UA_TYPES_UINT16; };
  template <> struct OpcTypeMap<UA_Int32> { static const int value = UA_TYPES_INT32; };
  template <> struct OpcTypeMap<UA_UInt32> { static const int value = UA_TYPES_UINT32; };
  template <> struct OpcTypeMap<UA_Int64> { static const int value = UA_TYPES_INT64; };
  template <> struct OpcTypeMap<UA_UInt64> { static const int value = UA_TYPES_UINT64; };
  template <> struct OpcTypeMap<UA_Float> { static const int value = UA_TYPES_FLOAT; };


  class OPCUA {
  public:
    typedef UA_Client*    client_type;
    typedef UA_NodeId     node_type;
    typedef UA_StatusCode status_code;

  public:
    OPCUA();
    ~OPCUA();

    status_code connect(std::string server_addr);
    void disconnect(bool deleteNodes = true);

    template <typename T>
    void addNode(std::string displayName, std::string description, std::string qualifiedName, T defaultSensorData = T());

    template <typename T>
    T readNode(std::string displayName) const;

    template <typename T>
    void writeNode(std::string displayName, T value);

  private:
    static constexpr char* locale = const_cast<char*>("en_US");
    client_type m_client;
    std::map<std::string, node_type> m_nodes;
    mutable status_code m_status;
  };

  template <typename T>
  void OPCUA::addNode(std::string displayName, std::string description, std::string qualifiedName, T defaultSensorData)
  {
    static_assert(OpcTypeMap<T>::value != -1, "Type not recognized by OPC-UA");

    node_type node;
    int opcTypeNum = OpcTypeMap<T>::value;

    /* Create the OPC UA Node associated with this device. */
    UA_VariableAttributes node_attr;
    UA_VariableAttributes_init(&node_attr);
    node_attr.displayName = UA_LOCALIZEDTEXT(locale, const_cast<char*>(displayName.c_str()));
    node_attr.description = UA_LOCALIZEDTEXT(locale, const_cast<char*>(description.c_str()));

    /* NOTE: This is important - sensor data type. */
    T sensorData = defaultSensorData;
    UA_Variant_setScalar(&node_attr.value, &sensorData, &UA_TYPES[opcTypeNum]);
    node_attr.dataType = UA_TYPES[opcTypeNum].typeId;

    /* Register the node to the server. */
    m_status = UA_Client_addVariableNode(m_client,
                                         UA_NODEID_NUMERIC(1, 0), // Random new node ID
                                         UA_NODEID_NUMERIC(0, UA_NS0ID_OBJECTSFOLDER),
                                         UA_NODEID_NUMERIC(0, UA_NS0ID_ORGANIZES),
                                         UA_QUALIFIEDNAME(0, const_cast<char*>(qualifiedName.c_str())),
                                         UA_NODEID_NULL, // no variable type
                                         node_attr,
                                         &node);

    if (m_status != UA_STATUSCODE_GOOD)
    {
      throw std::runtime_error(std::string(__FUNCTION__) +
                               ": cannot add variable node ");
    }

    // TODO: check if key is already used.
    m_nodes.insert(std::pair<std::string, node_type>(displayName, node));
  }

  template <typename T>
  T OPCUA::readNode(std::string displayName) const
  {
    T value;
    // TODO: type sanity check

    // Check for requested node.
    const auto search = m_nodes.find(displayName);
    if (search == m_nodes.end())
    {
      throw std::runtime_error(std::string(__FUNCTION__) +
                               ": cannot find requested node displayName");
    }

    UA_Variant *variant = UA_Variant_new();
    m_status = UA_Client_readValueAttribute(m_client, search->second, variant);
    if (m_status == UA_STATUSCODE_GOOD && UA_Variant_isScalar(variant) &&
        variant->type == &UA_TYPES[OpcTypeMap<T>::value])
    {
      value = *(static_cast<T*>(variant->data));
    }
    else
    {
      throw std::runtime_error(std::string(__FUNCTION__) +
                               ": node read error");
    }
    UA_Variant_delete(variant);

    return value;
  }

  template <typename T>
  void OPCUA::writeNode(std::string displayName, T value)
  {
    // TODO: type sanity check
    const auto search = m_nodes.find(displayName);
    if (search == m_nodes.end())
    {
      throw std::runtime_error(std::string(__FUNCTION__) +
                               ": cannot find requested node displayName");
    }

    UA_Variant new_value;
    UA_Variant_init(&new_value);

    UA_Variant_setScalar(&new_value, &value, &UA_TYPES[OpcTypeMap<T>::value]);
    UA_Client_writeValueAttribute(m_client, search->second, &new_value);
  }
}
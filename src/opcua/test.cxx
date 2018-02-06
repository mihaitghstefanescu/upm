#include <iostream>
#include "opcua_interface.hpp"

const std::string serverAddress("opc.tcp://localhost:4840");

int main()
{
  upm::OPCUA opcua;

  if (opcua.connect(serverAddress) != UA_STATUSCODE_GOOD)
  {
    std::cout << "error connecting to server" << std::endl;
  }

  opcua.addNode<float>("RHUSB: Humidity", "Humidity sensor", "RHUSB Humidity Node", 2.0f);
  opcua.addNode<int64_t>("RHUSB: Temperature", "Temperature sensor", "RHUSB Temperature Node", 10);

  std::cout << "Connected to " + serverAddress << std::endl;

  std::cout << "Humidity sensor first read: " << opcua.readNode<float>("RHUSB: Humidity") << std::endl;

  std::cout << "Enter new value for Humidity sensor: ";
  float newValue;
  std::cin >> newValue;
  opcua.writeNode<float>("RHUSB: Humidity", newValue);
  std::cout << "Humidity sensor second read: " << opcua.readNode<float>("RHUSB: Humidity") << std::endl;

  std::cout << "Disconnecting from server..." << std::endl;
  opcua.disconnect();

  return 0;
}
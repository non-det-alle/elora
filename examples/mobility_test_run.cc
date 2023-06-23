#include "ns3/core-module.h"
#include "ns3/network-module.h"
#include "ns3/mobility-module.h"
#include "ns3/internet-module.h"

NS_LOG_COMPONENT_DEFINE("MobilityTestRun");

using namespace ns3;

void PrintPosition(Ptr<Node> node) {
  Ptr<ConstantVelocityMobilityModel> cvmm = node->GetObject<ConstantVelocityMobilityModel>();
  NS_LOG_INFO("Node position: " << cvmm->GetPosition());
  Simulator::Schedule(Seconds(1.0), &PrintPosition, node); // Print position every 1 second
}

int main(int argc, char *argv[]) {
  LogComponentEnable("MobilityTestRun", LOG_LEVEL_INFO);

  // Create a Node container
  NodeContainer nodes;
  nodes.Create(1);

  // Create a Mobility model and install it on the node
  MobilityHelper mobility;
  mobility.SetMobilityModel("ns3::ConstantVelocityMobilityModel");
  mobility.Install(nodes);

  // Set constant velocity for the node
  Ptr<ConstantVelocityMobilityModel> cvmm = nodes.Get(0)->GetObject<ConstantVelocityMobilityModel>();
  cvmm->SetVelocity(Vector(10.0, 0.0, 0.0)); // Set velocity along x-axis to 10 m/s

  // Print node position periodically
  Simulator::Schedule(Seconds(1.0), &PrintPosition, nodes.Get(0));

// Stop the simulation after 10 seconds
  Simulator::Stop(Seconds(10.0));

  // Run the simulation
  Simulator::Run();
  Simulator::Destroy();

  return 0;
}

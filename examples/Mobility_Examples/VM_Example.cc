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

  // Set start and end positions
  double startLat = 40.7128;  // Start latitude
  double startLng = -74.0060; // Start longitude
  double endLat = 37.7749;    // End latitude
  double endLng = -122.4194;  // End longitude

  // Calculate velocity vector
  double time = 10.0; // Total time to travel from start to end (in seconds)
  Vector velocity((endLng - startLng) / time, (endLat - startLat) / time, 0.0);

  // Set constant velocity for the node
  Ptr<ConstantVelocityMobilityModel> cvmm = nodes.Get(0)->GetObject<ConstantVelocityMobilityModel>();
  cvmm->SetVelocity(velocity);

  // Print node position periodically
  Simulator::Schedule(Seconds(1.0), &PrintPosition, nodes.Get(0));

  // Stop the simulation after the specified duration
  Simulator::Stop(Seconds(time));

  // Run the simulation
  Simulator::Run();
  Simulator::Destroy();

  return 0;
}

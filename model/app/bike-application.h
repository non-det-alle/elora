

#ifndef BIKE_APPLICATION_H
#define BIKE_APPLICATION_H

#include "ns3/lora-application.h"
#include "ns3/mobility-module.h"
#include "ns3/simulator.h"

namespace ns3
{
namespace lorawan
{

class BikeApplication : public Application
{
  public:
    static TypeId GetTypeId(void);

    BikeApplication();
    virtual ~BikeApplication();

    void SetNode(Ptr<Node> node);  // New function to set the node
    void PrintNodePosition();      // New function to print node position

//   protected:
//     void DoInitialize() override;
//     void DoDispose() override;

  private:
    virtual void StartApplication();
    virtual void StopApplication();
    void ScheduleNextPositionPrint();  // New function to schedule next position print

    Ptr<Node> m_node;      // Node pointer
    EventId m_printEvent;  // EventId for the periodic printing
};

} // namespace lorawan

} // namespace ns3
#endif /* BIKE_APPLICATION  */
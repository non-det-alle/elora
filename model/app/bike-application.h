

#ifndef BIKE_APPLICATION_H
#define BIKE_APPLICATION_H

#include "ns3/lora-application.h"

namespace ns3
{
namespace lorawan
{

class BikeApplication : public LoraApplication
{
  public:
    BikeApplication();
    virtual ~BikeApplication();

    static TypeId GetTypeId(void);

//   protected:
//     void DoInitialize() override;
//     void DoDispose() override;

  private:
    virtual void StartApplication(void);
    //virtual void StopApplication(void);
    
    void SendPacket(void);  // New function to schedule next position print
};

} // namespace lorawan

} // namespace ns3
#endif /* BIKE_APPLICATION_H  */
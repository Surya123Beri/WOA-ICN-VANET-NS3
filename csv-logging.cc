#include "ns3/internet-module.h"
#include <fstream>
#include "ns3/applications-module.h"
#include "ns3/core-module.h"
#include "ns3/network-module.h"
#include "ns3/mobility-module.h"
//#include  "ns3/wave-module.h"
#include "ns3/wifi-module.h"
#include "ns3/internet-module.h"

using namespace ns3;

NS_LOG_COMPONENT_DEFINE("WOAVanet");

int main( int argc, char *argv[])
{ 

  uint32_t numVehicles =20;

CommandLine cmd;
 
cmd.AddValue("numVehicles","Number of vehicles",numVehicles);
cmd.Parse(argc,argv);

  double simTime= 80.0;
//creating CSV file for generating graphs
 std::ofstream outFile;
// outFile.open("woa-results.csv");
outFile.open("woa-results.csv",std::ios::app);
//outFile
//<< "Vehicles,"
//<<"PackentsSent,"
//<<"PacketsReceived,"
//<<"PDR,"
//<<"SimulationTime"
//<<"\n";




//creating vehicle nodes
NodeContainer vehicles;
vehicles.Create(numVehicles);

//configuring the mobility

MobilityHelper mobility;


 Ptr<ListPositionAllocator> positionAlloc = 
                            CreateObject<ListPositionAllocator>();

for(uint32_t i=0;i<numVehicles;i++)
{
positionAlloc->Add( 
          Vector(
                  (i%5)*30,
                  (i/5)*30,
                  0
                )
              );
}
mobility.SetPositionAllocator(positionAlloc);


//RANDOM WALK2d MOBILITY MODEL

//mobility.SetMobilityModel( 
  //                          "ns3::RandomWalk2dMobilityModel",
    //                        "Bounds",
      //                       RectangleValue(Rectangle(0,500,0,500)),
        //                    "Speed",
          ///                   StringValue(
             //                "ns3::ConstantRandomVariable[Constant=10.0]"),
               //              "Time",
                 //             TimeValue(Seconds(2.0))
                   //       );



mobility.SetMobilityModel("ns3::ConstantVelocityMobilityModel");

mobility.Install(vehicles);

for(uint32_t i=0;i<numVehicles;i++)
{
  Ptr<ConstantVelocityMobilityModel> cvmm =
      vehicles.Get(i)->GetObject<ConstantVelocityMobilityModel>();
     
   if(i%2 == 0)
    {
       cvmm->SetVelocity( Vector(4+(i*0.2),0,0));
    }
    else
   {
       cvmm->SetVelocity( Vector(-(4+(i*0.2)),0,0));
   }
}

//cvmm->SetVelocity(Vector(5+(i*0.5), 0, 0));



WifiHelper wifi;
wifi.SetStandard(WIFI_STANDARD_80211b);

YansWifiChannelHelper channel= YansWifiChannelHelper::Default();

YansWifiPhyHelper phy;


phy.Set("TxPowerStart",DoubleValue(18.0));
phy.Set("TxPowerEnd", DoubleValue(18.0));


phy.SetChannel(channel.Create());
WifiMacHelper mac;

mac.SetType("ns3::AdhocWifiMac");

NetDeviceContainer devices=wifi.Install(phy,mac,vehicles);

//INternet Stack

InternetStackHelper internet;
internet.Install(vehicles);

Ipv4AddressHelper ipv4;
//Ipv4GlobalRoutingHelper:: PopulateRoutingTables();
//Ipv4AddressHelper ipv4;

ipv4.SetBase("10.1.1.0","255.255.255.0");


Ipv4InterfaceContainer interfaces = ipv4.Assign(devices);
Ipv4GlobalRoutingHelper:: PopulateRoutingTables();


//printing node positions

for(uint32_t i=0; i<numVehicles; i++)
{
 
     Ptr<MobilityModel> mob = vehicles.Get(i)->GetObject<MobilityModel>();

      Vector pos= mob->GetPosition();
      std::cout
      << "Vehicle"
      << i
      << " Position :(" 
      <<   pos.x
      << ","
      << pos.y
      << ") IP: "
       << interfaces.GetAddress(i)
    << std::endl;
}

//UDP  COMMUNICATION

uint16_t port=4000;

//receiver setup for v1

UdpServerHelper  server(port);

ApplicationContainer serverApp =  server.Install(vehicles.Get(1));

serverApp.Start(Seconds(1.0));
serverApp.Stop(Seconds(simTime));
//sending from v0

UdpClientHelper client(interfaces.GetAddress(1),port);

client.SetAttribute("MaxPackets",UintegerValue(100));
client.SetAttribute("Interval", TimeValue(Seconds(0.1)) );
client.SetAttribute("PacketSize", UintegerValue(1024));


ApplicationContainer clientApp=client.Install(vehicles.Get(0));
clientApp.Start(Seconds(2.0));

clientApp.Stop(Seconds(simTime));

Ptr<UdpServer> udpServer = serverApp.Get(0)->GetObject<UdpServer>();


    Simulator::Stop(Seconds(simTime));

    Simulator::Run();

uint32_t received= udpServer->GetReceived();
double pdr= (received/100.0)*100.0;

std::cout
<< "\n========RESULTS========\n";
std::cout
<< "Packets Sent : 100\n";

std::cout
<< "Packets Received:"
<<received
<< std::endl;

std::cout
<<"Packet Delivery Ratio:"
<< pdr
<< "%"
<< std::endl;

outFile
<< numVehicles<< ","
<< 100<< ","
<< received<< ","
<<pdr<< ","
<< simTime
<<"\n";

outFile.close();
    Simulator::Destroy();

 return 0;
}

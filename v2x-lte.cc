/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil; -*- */
/*
 * Copyright (c) 2016
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation;
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 *
 * Author: Joahannes Costa <joahannes@gmail.com>
 * 
 */

#include "ns3/core-module.h"
#include "ns3/ocb-wifi-mac.h"
#include "ns3/network-module.h"
#include "ns3/wave-mac-helper.h"
#include "ns3/internet-module.h"
#include "ns3/mobility-module.h"
#include "ns3/yans-wifi-helper.h"
#include "ns3/wifi-80211p-helper.h"
#include "ns3/applications-module.h"
#include "ns3/config-store-module.h"
#include "ns3/point-to-point-module.h"
#include "ns3/netanim-module.h"
#include "ns3/evalvid-client-server-helper.h"

#include "ns3/ipv4-global-routing-helper.h"
#include "ns3/ipv4-address.h"

#include "ns3/flow-monitor-helper.h"

#include "ns3/point-to-point-helper.h"
#include "ns3/lte-helper.h"
#include "ns3/epc-helper.h"
#include "ns3/lte-module.h"

#define SIMULATION_TIME_FORMAT(s) Seconds(s)

using namespace ns3;
using namespace std;
NS_LOG_COMPONENT_DEFINE("V2vExample");

int main(int argc, char *argv[]) {

    //LogComponentEnable ("EvalvidClient", LOG_LEVEL_INFO);
    //LogComponentEnable ("EvalvidServer", LOG_LEVEL_INFO);

/*--------------------- Logging System Configuration -------------------*/
    NS_LOG_INFO("/------------------------------------------------\\");
    NS_LOG_INFO(" - V2vClusteringExample [Example] -> Cluster vehicles communication");
    NS_LOG_INFO("\\------------------------------------------------/");
/*----------------------------------------------------------------------*/

//-------------Parâmetros da simulação
    std::string phyMode ("OfdmRate6MbpsBW10MHz");
    uint16_t node_ue = 10; //VEÍCULOS
    uint16_t node_enb = 1; //eNb-RSU
    uint16_t node_remote = 1; //HOST REMOTO
    double simTime = 20.0; //TEMPO DE SIMULAÇÃO
/*----------------------------------------------------------------------*/

//*********** CONFIGURAÇÃO LTE ***************//
//Configuração padrão de Downlink e Uplink
Config::SetDefault("ns3::LteEnbNetDevice::DlBandwidth", UintegerValue(25));
Config::SetDefault("ns3::LteEnbNetDevice::UlBandwidth", UintegerValue(25));

//Modo de transmissão (SISO [0], MIMO [1])
Config::SetDefault ("ns3::LteEnbRrc::DefaultTransmissionMode", UintegerValue (1));
/*----------------------------------------------------------------------*/

/*-------------------- Set explicitly default values -------------------*/
Config::SetDefault ("ns3::WifiRemoteStationManager::FragmentationThreshold", StringValue ("2200"));
// turn off RTS/CTS for frames below 2200 bytes
Config::SetDefault ("ns3::WifiRemoteStationManager::RtsCtsThreshold", StringValue ("2200"));
// Fix non-unicast data rate to be the same as that of unicast
Config::SetDefault ("ns3::WifiRemoteStationManager::NonUnicastMode", StringValue (phyMode));
/*----------------------------------------------------------------------*/

/*------------------------- MÓDULOS LTE ----------------------*/
    Ptr<LteHelper> lteHelper = CreateObject<LteHelper> ();
    Ptr<PointToPointEpcHelper>  epcHelper = CreateObject<PointToPointEpcHelper> ();
//Ptr<EpcHelper> epcHelper = CreateObject<EpcHelper> ();
    lteHelper -> SetEpcHelper (epcHelper);
    lteHelper -> SetSchedulerType("ns3::PfFfMacScheduler");
    lteHelper -> SetAttribute ("PathlossModel", StringValue ("ns3::FriisPropagationLossModel"));

    ConfigStore inputConfig;
    inputConfig.ConfigureDefaults ();

//-------------Parâmetros da Antena
    //lteHelper -> SetEnbAntennaModelType ("ns3::CosineAntennaModel");
    //lteHelper -> SetEnbAntennaModelAttribute ("Orientation", DoubleValue (0));
    //lteHelper -> SetEnbAntennaModelAttribute ("Beamwidth", DoubleValue (60));
    //lteHelper -> SetEnbAntennaModelAttribute ("MaxGain", DoubleValue (0.0));

    Ptr<Node> pgw = epcHelper->GetPgwNode ();

//-------------Criação do RemoteHost
    //Cria um simples RemoteHost
    NodeContainer remoteHostContainer;
    remoteHostContainer.Create (node_remote); 
    Ptr<Node> remoteHost = remoteHostContainer.Get (0);
    
    //Pilha de Internet
    InternetStackHelper internet;
    internet.Install (remoteHost);

//Cria link Internet
    PointToPointHelper p2ph;
    p2ph.SetDeviceAttribute ("DataRate", DataRateValue (DataRate ("100Gb/s")));
    p2ph.SetDeviceAttribute ("Mtu", UintegerValue (1500));
    p2ph.SetChannelAttribute ("Delay", TimeValue (Seconds (0.010)));
    NetDeviceContainer internetDevices = p2ph.Install (pgw, remoteHost);

//Determina endereço ip para o Link
    Ipv4AddressHelper ipv4h;
    ipv4h.SetBase ("10.1.0.0", "255.255.0.0");
    Ipv4InterfaceContainer internetIpIfaces;
    internetIpIfaces = ipv4h.Assign (internetDevices);

//interface 0 é localhost e interface 1 é dispositivo p2p
    Ipv4Address remoteHostAddr = internetIpIfaces.GetAddress (1);
    Ipv4StaticRoutingHelper ipv4RoutingHelper;
    Ptr<Ipv4StaticRouting> remoteHostStaticRouting = ipv4RoutingHelper.GetStaticRouting (remoteHost->GetObject<Ipv4> ());
    remoteHostStaticRouting->AddNetworkRouteTo (Ipv4Address ("7.0.0.0"), Ipv4Mask ("255.0.0.0"), 1);
/*----------------------------------------------------------------------*/

/*------------------- Criacao de UEs-Enb-RSUs --------------------------*/
//UE - Veículos
    NodeContainer ueNodes;
    ueNodes.Create(node_ue);

//eNODEb - RSU
    NodeContainer enbNodes;
    enbNodes.Create(node_enb);

//Instala pilha de Internet em UE e EnodeB
    internet.Install(ueNodes);
/*----------------------------------------------------------------------*/

/*-------------------- MOBILIDADE ------------------------*/
//Mobilidadade para Veículos
    MobilityHelper mobilityUe;
    mobilityUe.SetPositionAllocator ("ns3::GridPositionAllocator",
                             "MinX", DoubleValue (10.0),
                             "MinY", DoubleValue (10.0),
                             "DeltaX", DoubleValue (5.0),
                             "DeltaY", DoubleValue (2.0),
                             "GridWidth", UintegerValue (3),
                             "LayoutType", StringValue ("RowFirst"));
    mobilityUe.SetMobilityModel ("ns3::ConstantVelocityMobilityModel");
    mobilityUe.Install (ueNodes);
    //Cria três vias na Rodovia
    for (uint16_t i = 0; i < node_ue; i++){
        if(i % 3 == 0){
            ueNodes.Get (i)->GetObject<MobilityModel> ()->SetPosition (Vector (i*5, 0, 0));
        }
        else if(i % 3 == 1){
            ueNodes.Get (i)->GetObject<MobilityModel> ()->SetPosition (Vector (i*5, 3, 0));
        }
        else{
            ueNodes.Get (i)->GetObject<MobilityModel> ()->SetPosition (Vector (i*5, 6, 0));
        }
    }
    //Configura uma variável aleatória para Velocidade
    Ptr<UniformRandomVariable> rvar = CreateObject<UniformRandomVariable>();
    //Para cada nó configura sua velocidade de acordo com a variável aleatória
    for (NodeContainer::Iterator iter= ueNodes.Begin(); iter!=ueNodes.End(); ++iter){
        Ptr<Node> tmp_node = (*iter);
        //Seleciona a velocidade entre 15 e 25m/s
        double speed = rvar->GetValue(15, 25);
        tmp_node->GetObject<ConstantVelocityMobilityModel>()->SetVelocity(Vector(speed, 0, 0));
    }

//Mobilidade para eNb/RSU
    Ptr<ListPositionAllocator> positionAlloc = CreateObject<ListPositionAllocator> ();
    for (uint16_t i = 1; i <= enbNodes.GetN(); i++){
        positionAlloc->Add (Vector(150 * i, 15, 0)); //DISTANCIA ENTRE ENBs [m]
    }
    MobilityHelper mobilityEnb;
    mobilityEnb.SetMobilityModel("ns3::ConstantPositionMobilityModel");
    mobilityEnb.SetPositionAllocator(positionAlloc);
    mobilityEnb.Install(enbNodes);
/*----------------------------------------------------------------------*/

//-------------Instala LTE Devices para cada grupo de nós
    NetDeviceContainer enbLteDevs;
    enbLteDevs = lteHelper->InstallEnbDevice (enbNodes);
    NetDeviceContainer ueLteDevs;
    ueLteDevs = lteHelper->InstallUeDevice (ueNodes);
/*----------------------------------------------------------------------*/

    Ipv4InterfaceContainer ueIpIface;
    ueIpIface = epcHelper->AssignUeIpv4Address (NetDeviceContainer (ueLteDevs));

//-------------Definir endereços IPs e instala aplicação
    for (uint32_t u = 0; u < ueNodes.GetN(); ++u){
        Ptr<Node> ueNode = ueNodes.Get (u);
        Ptr<Ipv4StaticRouting> ueStaticRouting = ipv4RoutingHelper.GetStaticRouting (ueNode->GetObject<Ipv4> ());
        ueStaticRouting->SetDefaultRoute (epcHelper->GetUeDefaultGatewayAddress (), 1);
    }
/*----------------------------------------------------------------------*/

//-------------Anexa as UEs na eNodeB
    for (uint16_t i = 0; i < ueNodes.GetN(); i++){
        lteHelper->Attach (ueLteDevs.Get(i), enbLteDevs.Get(0));
    }

  NS_LOG_INFO ("Create Applications.");
/*----------------------------------------------------------------------*/

/*-------------------------- Configuração dos nós Wifi --------------------------*/
//Os comandos abaixo montam as placas wifi no padrão 802.11p
/*
    YansWifiChannelHelper wifiChannel = YansWifiChannelHelper::Default ();
    Ptr<YansWifiChannel> channel = wifiChannel.Create ();

    YansWifiPhyHelper wifiPhy =  YansWifiPhyHelper::Default ();
    wifiPhy.SetChannel (channel);
    wifiPhy.SetPcapDataLinkType (YansWifiPhyHelper::DLT_IEEE802_11);
    wifiPhy.Set ("TxPowerStart", DoubleValue(32));
    wifiPhy.Set ("TxPowerEnd", DoubleValue(32));
    wifiPhy.Set ("TxGain", DoubleValue(12));
    wifiPhy.Set ("RxGain", DoubleValue(12));
    wifiPhy.Set ("EnergyDetectionThreshold", DoubleValue(-61.8));
    wifiPhy.Set ("CcaMode1Threshold", DoubleValue(-64.8));

    NqosWaveMacHelper wifi80211pMac = NqosWaveMacHelper::Default ();
    Wifi80211pHelper wifi80211p = Wifi80211pHelper::Default ();
    //wifi80211p.EnableLogComponents ();

    wifi80211p.SetRemoteStationManager ("ns3::ConstantRateWifiManager",
        "DataMode",StringValue (phyMode),
        "ControlMode",StringValue (phyMode));
    //UE
    NetDeviceContainer wifiDevices1 = wifi80211p.Install (wifiPhy, wifi80211pMac, ueNodes);
    //RSU
    NetDeviceContainer wifiDevices2 = wifi80211p.Install (wifiPhy, wifi80211pMac, enbNodes);

    NS_LOG_INFO ("Assign IP Addresses.");
    Ipv4AddressHelper ipv4h;
    ipv4h.SetBase ("10.1.1.0", "255.255.255.0");
    Ipv4InterfaceContainer i1 = ipv4h.Assign (wifiDevices1);
    //Configuração de IP da RSU
    Ipv4InterfaceContainer i2 = ipv4h.Assign (wifiDevices2); //RSU
*/
/*----------------------------------------------------------------------*/

//TESTE DE TRANSMISSÃO DE VÍDEO

//-------------Rodar aplicação EvalVid
    for (uint32_t i = 0; i < ueNodes.GetN(); ++i){
    //Gera SD e RD para cada Veículo
        std::stringstream sdTrace;
        std::stringstream rdTrace;
        sdTrace << "resultados/normal_sd_a01_" << (int)i;
        rdTrace << "resultados/normal_rd_a01_" << (int)i;

        double start = 5.0;
        double stop = simTime; 

        uint16_t port = 2000;
        uint16_t m_port = port * i + 2000; //Para alcançar o nó ZERO quando i = 0

        //Servidor de vídeo
        EvalvidServerHelper server (m_port);
         server.SetAttribute ("SenderTraceFilename", StringValue("video/st_highway_cif.st"));
         server.SetAttribute ("SenderDumpFilename", StringValue(sdTrace.str()));
         server.SetAttribute ("PacketPayload",UintegerValue(1014));
         ApplicationContainer apps = server.Install(remoteHost);
         apps.Start (Seconds (start));
         apps.Stop (Seconds (stop));

        //Clientes do vídeo
        EvalvidClientHelper client (remoteHostAddr,m_port);
         client.SetAttribute ("ReceiverDumpFilename", StringValue(rdTrace.str()));
         apps = client.Install (ueNodes.Get(i));
         apps.Start (Seconds (start));
         apps.Stop (Seconds (stop));
    }
//FIM TESTE DE TRANSMISSÃO DE VÍDEO

//AsciiTraceHelper ascii;
//wifiPhy.EnableAsciiAll(ascii.CreateFileStream ("resultados/socket-options-ipv4.txt"));
//wifiPhy.EnablePcapAll ("resultados/socket.pcap", false);

/*----------------------------------------------------------------------*/
   
AnimationInterface anim ("resultados/normal_v2x.xml");
//Cor e Descrição para RSU
for (uint32_t i = 0; i < enbNodes.GetN (); ++i){
    anim.UpdateNodeDescription (enbNodes.Get (i), "RSU");
    anim.UpdateNodeColor (enbNodes.Get (i), 0, 255, 0);
}

Ptr<FlowMonitor> flowMonitor;
  FlowMonitorHelper flowHelper;
  flowMonitor = flowHelper.InstallAll();

/*---------------------- Simulation Stopping Time ----------------------*/
Simulator::Stop(SIMULATION_TIME_FORMAT(simTime));
/*----------------------------------------------------------------------*/

/*--------------------------- Simulation Run ---------------------------*/
Simulator::Run();

flowMonitor->SerializeToXmlFile("resultados/_flow.xml", true, true);

Simulator::Destroy();
/*----------------------------------------------------------------------*/

return EXIT_SUCCESS;
}

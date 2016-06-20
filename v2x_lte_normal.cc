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

//Padrão
#include "ns3/core-module.h"
#include "ns3/network-module.h"
#include "ns3/internet-module.h"
#include "ns3/mobility-module.h"
#include "ns3/applications-module.h"
#include "ns3/config-store-module.h"
#include "ns3/point-to-point-module.h"
#include "ns3/ipv4-global-routing-helper.h"
#include "ns3/ipv4-address.h"
//NetAnim & Evalvid
#include "ns3/netanim-module.h"
#include "ns3/evalvid-client-server-helper.h"
//Pacotes WiFi
#include "ns3/wave-mac-helper.h"
#include "ns3/ocb-wifi-mac.h"
#include "ns3/yans-wifi-helper.h"
#include "ns3/wifi-80211p-helper.h"
//Pacotes LTE
#include "ns3/point-to-point-helper.h"
#include "ns3/lte-helper.h"
#include "ns3/epc-helper.h"
#include "ns3/lte-module.h"
//Monitor de fluxo
#include "ns3/flow-monitor-module.h"
#include "ns3/flow-monitor.h"
#include "ns3/flow-monitor-helper.h"
#include "ns3/gnuplot.h"

#include "ns3/string.h"
#include "ns3/double.h"
#include <ns3/boolean.h>
#include <ns3/enum.h>
#include <iomanip>
#include <string>
#include <fstream>
#include <iostream>
#include <vector>
#include <stdio.h>
#include <stdlib.h>

#define SIMULATION_TIME_FORMAT(s) Seconds(s)

using namespace ns3;
using namespace std;

NS_LOG_COMPONENT_DEFINE("V2X-Example");

void ThroughputMonitor (FlowMonitorHelper* fmhelper, Ptr<FlowMonitor> monitor, Gnuplot2dDataset dataset){

  double tempThroughput = 0.0;
  monitor->CheckForLostPackets(); 
  std::map<FlowId, FlowMonitor::FlowStats> flowStats = monitor->GetFlowStats();
  Ptr<Ipv4FlowClassifier> classifier = DynamicCast<Ipv4FlowClassifier> (fmhelper->GetClassifier());

  for (std::map<FlowId, FlowMonitor::FlowStats>::const_iterator stats = flowStats.begin (); stats != flowStats.end (); ++stats){ 
      // A tuple: Source-ip, destination-ip, protocol, source-port, destination-port
      //Ipv4FlowClassifier::FiveTuple fiveTuple = classifier->FindFlow (stats->first);
      
      //std::cout<<"Flow ID: " << stats->first <<" ; "<< fiveTuple.sourceAddress <<" -----> "<<fiveTuple.destinationAddress<<std::endl;
      //std::cout<<"Tx Packets = " << stats->second.txPackets<<std::endl;
      //std::cout<<"Rx Packets = " << stats->second.rxPackets<<std::endl;
      //std::cout<<"Duration: " <<stats->second.timeLastRxPacket.GetSeconds() - stats->second.timeFirstTxPacket.GetSeconds()<<std::endl;
      //std::cout<<"Last Received Packet: "<< stats->second.timeLastRxPacket.GetSeconds()<<" Seconds"<<std::endl;
      tempThroughput = (stats->second.rxBytes * 8.0 / (stats->second.timeLastRxPacket.GetSeconds() - stats->second.timeFirstTxPacket.GetSeconds())/1024/1024);
      //std::cout<<"Throughput: "<< tempThroughput <<" Mbps"<<std::endl;
      //std::cout<<"Last Received Packet: "<< stats->second.timeLastRxPacket.GetSeconds()<<" Seconds ---->" << "Throughput: " << tempThroughput << " Kbps" << std::endl;
      dataset.Add((double)Simulator::Now().GetSeconds(), (double)tempThroughput);
      //std::cout<<"------------------------------------------"<<std::endl;
    }
  
  //Tempo que será iniciado
  Simulator::Schedule(Seconds(1),&ThroughputMonitor, fmhelper, monitor, dataset);
}

void DelayMonitor (FlowMonitorHelper* fmhelper, Ptr<FlowMonitor> monitor, Gnuplot2dDataset dataset1){
  
  double delay = 0.0;
  monitor->CheckForLostPackets(); 
  std::map<FlowId, FlowMonitor::FlowStats> flowStats = monitor->GetFlowStats();
  Ptr<Ipv4FlowClassifier> classifier = DynamicCast<Ipv4FlowClassifier> (fmhelper->GetClassifier());

  for (std::map<FlowId, FlowMonitor::FlowStats>::const_iterator stats = flowStats.begin (); stats != flowStats.end (); ++stats){ 
      //Ipv4FlowClassifier::FiveTuple fiveTuple = classifier->FindFlow (stats->first);
      delay = stats->second.delaySum.GetSeconds ();
      dataset1.Add((double)Simulator::Now().GetSeconds(), (double)delay);
    }
  
  //Tempo que será iniciado
  Simulator::Schedule(Seconds(1),&DelayMonitor, fmhelper, monitor, dataset1);
}

void LostPacketsMonitor (FlowMonitorHelper* fmhelper, Ptr<FlowMonitor> monitor, Gnuplot2dDataset dataset2){
  
  double packets = 0.0;
  monitor->CheckForLostPackets(); 
  std::map<FlowId, FlowMonitor::FlowStats> flowStats = monitor->GetFlowStats();
  Ptr<Ipv4FlowClassifier> classifier = DynamicCast<Ipv4FlowClassifier> (fmhelper->GetClassifier());

  for (std::map<FlowId, FlowMonitor::FlowStats>::const_iterator stats = flowStats.begin (); stats != flowStats.end (); ++stats){ 
      //Ipv4FlowClassifier::FiveTuple fiveTuple = classifier->FindFlow (stats->first);
      packets = stats->second.lostPackets;
      dataset2.Add((double)Simulator::Now().GetSeconds(), (double)packets);
    }
  
  //Tempo que será iniciado
  Simulator::Schedule(Seconds(1),&LostPacketsMonitor, fmhelper, monitor, dataset2);
}

void JitterMonitor (FlowMonitorHelper* fmhelper, Ptr<FlowMonitor> monitor, Gnuplot2dDataset dataset3){
  
  double jitter = 0.0;
  monitor->CheckForLostPackets(); 
  std::map<FlowId, FlowMonitor::FlowStats> flowStats = monitor->GetFlowStats();
  Ptr<Ipv4FlowClassifier> classifier = DynamicCast<Ipv4FlowClassifier> (fmhelper->GetClassifier());

  for (std::map<FlowId, FlowMonitor::FlowStats>::const_iterator stats = flowStats.begin (); stats != flowStats.end (); ++stats){ 
      //Ipv4FlowClassifier::FiveTuple fiveTuple = classifier->FindFlow (stats->first);
      jitter = stats->second.jitterSum.GetSeconds ();
      dataset3.Add((double)Simulator::Now().GetSeconds(), (double)jitter);
    }
  
  //Tempo que será iniciado
  Simulator::Schedule(Seconds(1),&LostPacketsMonitor, fmhelper, monitor, dataset3);
}

void ImprimeMetricas (FlowMonitorHelper* fmhelper, Ptr<FlowMonitor> monitor){
  double tempThroughput = 0.0;
  monitor->CheckForLostPackets(); 
  std::map<FlowId, FlowMonitor::FlowStats> flowStats = monitor->GetFlowStats();
  Ptr<Ipv4FlowClassifier> classifier = DynamicCast<Ipv4FlowClassifier> (fmhelper->GetClassifier());

  for (std::map<FlowId, FlowMonitor::FlowStats>::const_iterator stats = flowStats.begin (); stats != flowStats.end (); ++stats){ 
      // A tuple: Source-ip, destination-ip, protocol, source-port, destination-port
      Ipv4FlowClassifier::FiveTuple fiveTuple = classifier->FindFlow (stats->first);
      
      std::cout<<"Flow ID: " << stats->first <<" ; "<< fiveTuple.sourceAddress <<" -----> "<<fiveTuple.destinationAddress<<std::endl;
      std::cout<<"Tx Packets = " << stats->second.txPackets<<std::endl;
      std::cout<<"Rx Packets = " << stats->second.rxPackets<<std::endl;
      std::cout<<"Duration: " <<stats->second.timeLastRxPacket.GetSeconds() - stats->second.timeFirstTxPacket.GetSeconds()<<std::endl;
      std::cout<<"Last Received Packet: "<< stats->second.timeLastRxPacket.GetSeconds()<<" Seconds"<<std::endl;
      tempThroughput = (stats->second.rxBytes * 8.0 / (stats->second.timeLastRxPacket.GetSeconds() - stats->second.timeFirstTxPacket.GetSeconds())/1024/1024);
      std::cout<<"Throughput: "<< tempThroughput <<" Mbps"<<std::endl;
      std::cout<< "Delay: " << stats->second.delaySum.GetSeconds () << std::endl;
      std::cout<< "LostPackets: " << stats->second.lostPackets << std::endl;
      std::cout<< "Jitter: " << stats->second.jitterSum.GetSeconds () << std::endl;
      //std::cout<<"Last Received Packet: "<< stats->second.timeLastRxPacket.GetSeconds()<<" Seconds ---->" << "Throughput: " << tempThroughput << " Kbps" << std::endl;
      std::cout<<"------------------------------------------"<<std::endl;
    }
  
  //Tempo que será iniciado
  Simulator::Schedule(Seconds(1),&ImprimeMetricas, fmhelper, monitor);
}

int main(int argc, char *argv[]) {

	//Log da Transmissão de Vídeo
    LogComponentEnable ("EvalvidClient", LOG_LEVEL_INFO);
    LogComponentEnable ("EvalvidServer", LOG_LEVEL_INFO);

/*--------------------- Logging System Configuration -------------------*/
    NS_LOG_INFO("/------------------------------------------------\\");
    NS_LOG_INFO(" - V2vClusteringExample [Example] -> Cluster vehicles communication");
    NS_LOG_INFO("\\------------------------------------------------/");
/*----------------------------------------------------------------------*/

//-------------Parâmetros da simulação
    std::string phyMode ("OfdmRate6MbpsBW10MHz");
    uint16_t node_ue = 10; //VEÍCULOS
    uint16_t node_enb = 1; //ENB
    uint16_t node_remote = 1; //HOST_REMOTO
    double simTime = 70.0; //TEMPO_SIMULAÇÃO
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
	//turn off RTS/CTS for frames below 2200 bytes
	Config::SetDefault ("ns3::WifiRemoteStationManager::RtsCtsThreshold", StringValue ("2200"));
	//Fix non-unicast data rate to be the same as that of unicast
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

//Mobilidade para eNb
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
/*
    //RSU
    //NetDeviceContainer wifiDevices2 = wifi80211p.Install (wifiPhy, wifi80211pMac, enbNodes);

    NS_LOG_INFO ("Assign IP Addresses.");
    Ipv4AddressHelper ipv4h;
    ipv4h.SetBase ("1.0.0.0", "255.0.0.0");
    Ipv4InterfaceContainer i1 = ipv4h.Assign (wifiDevices1);
    //Configuração de IP da RSU
    //Ipv4InterfaceContainer i2 = ipv4h.Assign (wifiDevices2); //RSU
*/
/*----------------------------------------------------------------------*/

//Início Transmissão de Vídeo
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
//Fim Transmissão de Vídeo

//AsciiTraceHelper ascii;
//wifiPhy.EnableAsciiAll(ascii.CreateFileStream ("resultados/socket-options-ipv4.txt"));
//wifiPhy.EnablePcapAll ("resultados/socket.pcap", false);
/*----------------------------------------------------------------------*/


AnimationInterface anim ("resultados/normal_v2x.xml");
//Cor e Descrição para eNb
for (uint32_t i = 0; i < enbNodes.GetN (); ++i){
    anim.UpdateNodeDescription (enbNodes.Get (i), "eNb");
    anim.UpdateNodeColor (enbNodes.Get (i), 0, 255, 0);
}

Ptr<FlowMonitor> monitor;
  FlowMonitorHelper fmhelper;
  monitor = fmhelper.InstallAll();

/*---------------------- Simulation Stopping Time ----------------------*/
Simulator::Stop(SIMULATION_TIME_FORMAT(simTime));
/*----------------------------------------------------------------------*/
//GNUPLOT
//Throughput
string vazao = "FlowVSThroughput";
string graphicsFileName        = vazao + ".png";
string plotFileName            = vazao + ".plt";
string plotTitle               = "Flow vs Throughput";
string dataTitle               = "Throughput";

Gnuplot gnuplot (graphicsFileName);
gnuplot.SetTitle (plotTitle);
gnuplot.SetTerminal ("png");
gnuplot.SetLegend ("Flow", "Throughput");

Gnuplot2dDataset dataset;
dataset.SetTitle (dataTitle);
dataset.SetStyle (Gnuplot2dDataset::LINES_POINTS);

//Delay
string delay = "FlowVSDelay";
string graphicsFileName1        = delay + ".png";
string plotFileName1            = delay + ".plt";
string plotTitle1               = "Flow vs Delay";
string dataTitle1               = "Delay";

Gnuplot gnuplot1 (graphicsFileName1);
gnuplot1.SetTitle (plotTitle1);
gnuplot1.SetTerminal ("png");
gnuplot1.SetLegend ("Flow", "Delay");

Gnuplot2dDataset dataset1;
dataset1.SetTitle (dataTitle1);
dataset1.SetStyle (Gnuplot2dDataset::LINES_POINTS);

//LostPackets
string lost = "FlowVSLostPackets";
string graphicsFileName2        = lost + ".png";
string plotFileName2            = lost + ".plt";
string plotTitle2               = "Flow vs LostPackets";
string dataTitle2               = "LostPackets";

Gnuplot gnuplot2 (graphicsFileName2);
gnuplot2.SetTitle (plotTitle2);
gnuplot2.SetTerminal ("png");
gnuplot2.SetLegend ("Flow", "LostPackets");

Gnuplot2dDataset dataset2;
dataset2.SetTitle (dataTitle2);
dataset2.SetStyle (Gnuplot2dDataset::LINES_POINTS);

//Jitter
string jitter = "FlowVSJitter";
string graphicsFileName3        = jitter + ".png";
string plotFileName3            = jitter + ".plt";
string plotTitle3               = "Flow vs Jitter";
string dataTitle3               = "Jitter";

Gnuplot gnuplot3 (graphicsFileName3);
gnuplot3.SetTitle (plotTitle3);
gnuplot3.SetTerminal ("png");
gnuplot3.SetLegend ("Flow", "Jitter");

Gnuplot2dDataset dataset3;
dataset3.SetTitle (dataTitle3);
dataset3.SetStyle (Gnuplot2dDataset::LINES_POINTS);

//Chama classe de captura do fluxo
ThroughputMonitor (&fmhelper, monitor, dataset);
DelayMonitor (&fmhelper, monitor, dataset1);
LostPacketsMonitor (&fmhelper, monitor, dataset2);
JitterMonitor (&fmhelper, monitor, dataset3);
/*--------------------------- Simulation Run ---------------------------*/
Simulator::Run(); //Executa

ImprimeMetricas (&fmhelper, monitor);

//Throughput
gnuplot.AddDataset (dataset);
std::ofstream plotFile (plotFileName.c_str()); // Abre o arquivo.
gnuplot.GenerateOutput (plotFile);    //Escreve no arquivo.
plotFile.close ();        // fecha o arquivo.
//Delay
gnuplot1.AddDataset (dataset1);
std::ofstream plotFile1 (plotFileName1.c_str()); // Abre o arquivo.
gnuplot1.GenerateOutput (plotFile1);    //Escreve no arquivo.
plotFile1.close ();        // fecha o arquivo.
//LostPackets
gnuplot2.AddDataset (dataset2);
std::ofstream plotFile2 (plotFileName2.c_str()); // Abre o arquivo.
gnuplot2.GenerateOutput (plotFile2);    //Escreve no arquivo.
plotFile2.close ();        // fecha o arquivo.
//Jitter
gnuplot3.AddDataset (dataset3);
std::ofstream plotFile3 (plotFileName3.c_str()); // Abre o arquivo.
gnuplot3.GenerateOutput (plotFile3);    //Escreve no arquivo.
plotFile3.close ();        // fecha o arquivo.

monitor->SerializeToXmlFile ("resultados/01_flow.xml", true, true);

Simulator::Destroy();
/*----------------------------------------------------------------------*/

return EXIT_SUCCESS;

}
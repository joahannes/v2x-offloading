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
#include "ns3/v2v-control-client-helper.h"
#include "ns3/v2v-mobility-model.h"
#include "ns3/netanim-module.h"
#include "ns3/evalvid-client-server-helper.h"

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
NS_LOG_COMPONENT_DEFINE("V2vClusteringExample");

void ThroughputMonitor (FlowMonitorHelper* fmhelper, Ptr<FlowMonitor> monitor, Gnuplot2dDataset dataset){

  double tempThroughput = 0.0;
  monitor->CheckForLostPackets(); 
  std::map<FlowId, FlowMonitor::FlowStats> flowStats = monitor->GetFlowStats();
  Ptr<Ipv4FlowClassifier> classifier = DynamicCast<Ipv4FlowClassifier> (fmhelper->GetClassifier());

  for (std::map<FlowId, FlowMonitor::FlowStats>::const_iterator stats = flowStats.begin (); stats != flowStats.end (); ++stats){ 
      tempThroughput = (stats->second.rxBytes * 8.0 / (stats->second.timeLastRxPacket.GetSeconds() - stats->second.timeFirstTxPacket.GetSeconds())/1024/1024);
      dataset.Add((double)Simulator::Now().GetSeconds(), (double)tempThroughput);
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

    /*--------------------- Logging System Configuration -------------------*/
    LogLevel logLevel = (LogLevel) (LOG_PREFIX_ALL | LOG_LEVEL_WARN);
    LogComponentEnable("V2vClusteringExample", logLevel);
    LogComponentEnable("V2vControlClient", logLevel);

    NS_LOG_INFO("/------------------------------------------------\\");
    NS_LOG_INFO(" - V2vClusteringExample [Example] -> Cluster vehicles communication");
    NS_LOG_INFO("\\------------------------------------------------/");
    /*----------------------------------------------------------------------*/

    /*---------------------- Simulation Default Values ---------------------*/
    std::string phyMode ("OfdmRate6MbpsBW10MHz");

    uint16_t numberOfUes = 10; //VEÍCULOS

    uint16_t numberOfRsus = 1; //RSU

    double minimumTdmaSlot = 0.001;         /// Time difference between 2 transmissions
    double clusterTimeMetric = 3.0;         /// Clustering Time Metric for Waiting Time calculation
    double speedVariation = 5.0;
    double incidentWindow = 30.0;

    double simTime = 40.0;
    /*----------------------------------------------------------------------*/


    /*-------------------- Set explicitly default values -------------------*/
    Config::SetDefault ("ns3::WifiRemoteStationManager::FragmentationThreshold",
                            StringValue ("2200"));
    // turn off RTS/CTS for frames below 2200 bytes
    Config::SetDefault ("ns3::WifiRemoteStationManager::RtsCtsThreshold",
                            StringValue ("2200"));
    // Fix non-unicast data rate to be the same as that of unicast
    Config::SetDefault ("ns3::WifiRemoteStationManager::NonUnicastMode",
                            StringValue (phyMode));
    /*----------------------------------------------------------------------*/


    /*-------------------- Command Line Argument Values --------------------*/
    CommandLine cmd;
    cmd.AddValue("ueNumber", "Number of UE", numberOfUes);
    cmd.AddValue("simTime", "Simulation Time in Seconds", simTime);

    NS_LOG_INFO("");
    NS_LOG_INFO("|---"<< " SimTime -> " << simTime <<" ---|\n");
    NS_LOG_INFO("|---"<< " Number of UE -> " << numberOfUes <<" ---|\n");
    /*----------------------------------------------------------------------*/


    /*------------------------- Create UEs-RSUs ----------------------------*/
    NodeContainer ueNodes;
    ueNodes.Create(numberOfUes);

    //-RSU
    NodeContainer rsuNodes;
    rsuNodes.Create(numberOfRsus);

    //Instala pilha de Internet
    InternetStackHelper internet;
    internet.Install(ueNodes);
    internet.Install(rsuNodes);
    /*----------------------------------------------------------------------*/


    /*-------------------- Install Mobility Model in Ue --------------------*/
    MobilityHelper ueMobility;
    ueMobility.SetMobilityModel ("ns3::V2vMobilityModel",
         "Mode", StringValue ("Time"),
         "Time", StringValue ("40s"),
         "Speed", StringValue ("ns3::ConstantRandomVariable[Constant=30.0]"),
         "Bounds", RectangleValue (Rectangle (0, 9000, -1000, 1000))); //Valores padrão: 0, 10000, -1000, 1000
    ueMobility.Install(ueNodes);

    /// Create a 3 line grid of vehicles
    for (uint16_t i = 0; i < numberOfUes; i++)
    {
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
    /*----------------------------------------------------------------------*/

    //Mobilidade para RSU
    /*
    Ptr<ListPositionAllocator> positionAlloc = CreateObject<ListPositionAllocator> ();
    for (uint16_t i = 1; i <= rsuNodes.GetN(); i++){
     positionAlloc->Add (Vector(150 * i, 15, 0)); //DISTANCIA ENTRE RSUs [m] 
    }
    */

    Ptr<ListPositionAllocator> positionAlloc = CreateObject<ListPositionAllocator> ();
    positionAlloc->Add (Vector(300, 15, 0)); //DISTANCIA ENTRE RSUs [m] }

    MobilityHelper mobilityRsu;
    mobilityRsu.SetMobilityModel("ns3::ConstantPositionMobilityModel");
    mobilityRsu.SetPositionAllocator(positionAlloc);
    mobilityRsu.Install(rsuNodes);

    /*-------------------------- Setup Wifi nodes --------------------------*/
    // The below set of helpers will help us to put together the wifi NICs we want
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
    NetDeviceContainer wifiDevices1 = wifi80211p.Install (wifiPhy, wifi80211pMac, ueNodes);

    //RSU
    NetDeviceContainer wifiDevices2 = wifi80211p.Install (wifiPhy, wifi80211pMac, rsuNodes); //RSU

    NS_LOG_INFO ("Assign IP Addresses.");
    Ipv4AddressHelper ipv4h;
    ipv4h.SetBase ("10.1.1.0", "255.255.255.0");
    Ipv4InterfaceContainer i1 = ipv4h.Assign (wifiDevices1);

    //Configuração de IP da RSU
    Ipv4InterfaceContainer i2 = ipv4h.Assign (wifiDevices2); //RSU

    uint16_t controlPort = 3999;
    ApplicationContainer controlApps; //Para troca de mensagens

    /**
     * Setting Control Channel
     */
    for (uint32_t u = 0; u < ueNodes.GetN(); ++u) {

        //!< Initial TDMA UE synchronization Function
        double tdmaStart = (u+1)*minimumTdmaSlot;

        Ptr<V2vMobilityModel> mobilityModel = ueNodes.Get(u)->GetObject<V2vMobilityModel>();
        mobilityModel->SetSpeedVariation(speedVariation);
        V2vControlClientHelper ueClient("ns3::UdpSocketFactory", Address(InetSocketAddress(Ipv4Address::GetBroadcast(), controlPort)),
                "ns3::UdpSocketFactory",InetSocketAddress(Ipv4Address::GetAny(), controlPort),
                mobilityModel, tdmaStart, numberOfUes, minimumTdmaSlot, clusterTimeMetric);
        ueClient.SetAttribute ("IncidentWindow", DoubleValue(incidentWindow));
        controlApps.Add(ueClient.Install(ueNodes.Get(u)));
    }

//TESTE DE TRANSMISSÃO DE VÍDEO

//Caminho para salvar os arquivos de envio e recebimento
string caminho = "resultados/agrupamento/";

//01
//Transmissão do CH para demais veículos do grupo
    std::string sd_CH = caminho + "sd_a01_CH";
    std::string rd_CH = caminho + "rd_a01_CH";
  
  //ServidorEvalvid:
    uint16_t port_video = 9500;
    EvalvidServerHelper server (port_video);
    server.SetAttribute ("SenderTraceFilename", StringValue("video/st_a03.st"));
    server.SetAttribute ("SenderDumpFilename", StringValue(sd_CH));
    server.SetAttribute ("PacketPayload",UintegerValue(1014));
    ApplicationContainer apps = server.Install (rsuNodes.Get(0)); //Servidor na nuvem como fonte
    apps.Start (Seconds (4.0));
    apps.Stop (Seconds (15.0));

  //ClienteEvalvid:
    EvalvidClientHelper client (i2.GetAddress (0), port_video);
    client.SetAttribute ("ReceiverDumpFilename", StringValue(rd_CH));
    apps = client.Install (ueNodes.Get(9)); //N10 como destino
    apps.Start (Seconds (4.0));
    apps.Stop (Seconds (15.0));
//Fim_V2I

//02
//-------------Rodar aplicação EvalVid
  for (uint32_t i = 0; i < ueNodes.GetN() - 1; i++){
    //Gera SD e RD para cada Veículo
    
    string video_trans = "video/st_a03.st";

    std::stringstream sdTrace;
    std::stringstream rdTrace;
    sdTrace << caminho + "sd_a01_" << (int)i;
    rdTrace << caminho + "rd_a01_" << (int)i;
 
    double start = 17.0;
    double stop = simTime; 
    
    uint16_t port = 1000;
    uint16_t m_port = port * i + 1000; //Para alcançar o nó ZERO quando i = 0

    EvalvidServerHelper server (m_port);
     server.SetAttribute ("SenderTraceFilename", StringValue(video_trans));
     server.SetAttribute ("SenderDumpFilename", StringValue(sdTrace.str()));
     server.SetAttribute ("PacketPayload",UintegerValue(1014));
     ApplicationContainer apps = server.Install(ueNodes.Get(9));
     apps.Start (Seconds (start));
     apps.Stop (Seconds (stop));
  
    EvalvidClientHelper client (i1.GetAddress (9),m_port);
     client.SetAttribute ("ReceiverDumpFilename", StringValue(rdTrace.str()));
     apps = client.Install (ueNodes.Get(i));
     apps.Start (Seconds (start));
     apps.Stop (Seconds (stop));
   }
//FIM TESTE DE TRANSMISSÃO DE VÍDEO

    controlApps.Start (Seconds(0.1));
    controlApps.Stop (Seconds(simTime-0.1));

    //AsciiTraceHelper ascii;
    //wifiPhy.EnableAsciiAll(ascii.CreateFileStream ("resultados/socket-options-ipv4.txt"));
    //wifiPhy.EnablePcapAll ("resultados/socket.pcap", false);

    /*----------------------------------------------------------------------*/
       
    AnimationInterface anim ("resultados/agrupamento/agrup_v2x_netanim.xml");
    //Cor e Descrição para RSU
    for (uint32_t i = 0; i < rsuNodes.GetN (); ++i){
        anim.UpdateNodeDescription (rsuNodes.Get (i), "RSU");
        anim.UpdateNodeColor (rsuNodes.Get (i), 0, 255, 0);
    }

    //Monitor de fluxo
    Ptr<FlowMonitor> monitor;
    FlowMonitorHelper fmhelper;
    monitor = fmhelper.InstallAll();
    
    /*---------------------- Simulation Stopping Time ----------------------*/
    Simulator::Stop(SIMULATION_TIME_FORMAT(simTime));
    /*----------------------------------------------------------------------*/

    string tipo = "graficos/agrupamento/Agrupamento_";

    //Throughput
    string vazao = tipo + "FlowVSThroughput";
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
    string delay = tipo + "FlowVSDelay";
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
    string lost = tipo + "FlowVSLostPackets";
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
    string jitter = tipo + "FlowVSJitter";
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
    Simulator::Run();

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

    monitor->SerializeToXmlFile("resultados/agrupamento/flow_agrupamento.xml", true, true);

    Simulator::Destroy();
    /*----------------------------------------------------------------------*/

    return EXIT_SUCCESS;
}

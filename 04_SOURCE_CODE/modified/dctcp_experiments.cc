/*
 * Dự án BaoCao_NS3 — Tái hiện và Kiểm chứng DCTCP trên NS-3.42
 * Bản sao thực thi trong thư mục scratch của NS-3.42
 */

#include "ns3/applications-module.h"
#include "ns3/core-module.h"
#include "ns3/internet-module.h"
#include "ns3/network-module.h"
#include "ns3/point-to-point-module.h"
#include "ns3/traffic-control-module.h"

#include <cmath>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <numeric>
#include <sstream>
#include <string>
#include <vector>

using namespace ns3;

namespace SingleFlowSim
{
std::ofstream cwndStream;
std::ofstream alphaStream;
std::ofstream queueStream;
std::ofstream tputStream;
uint64_t rxBytesWindow = 0;
uint64_t totalRxBytes = 0;

void
TraceCwnd(uint32_t oldCwnd, uint32_t newCwnd)
{
    cwndStream << std::fixed << std::setprecision(5) << Simulator::Now().GetSeconds() << ","
               << static_cast<double>(newCwnd) / 1448.0 << std::endl;
}

void
TraceAlpha(uint32_t bytesMarked, uint32_t bytesAcked, double alpha)
{
    alphaStream << std::fixed << std::setprecision(5) << Simulator::Now().GetSeconds() << ","
                << bytesMarked << "," << bytesAcked << "," << alpha << std::endl;
}

void
ConnectSocketTraces(bool isDctcp)
{
    Config::ConnectWithoutContextFailSafe(
        "/NodeList/0/$ns3::TcpL4Protocol/SocketList/0/CongestionWindow",
        MakeCallback(&TraceCwnd));
    if (isDctcp)
    {
        Config::ConnectWithoutContextFailSafe(
            "/NodeList/0/$ns3::TcpL4Protocol/SocketList/0/CongestionOps/$ns3::TcpDctcp/"
            "CongestionEstimate",
            MakeCallback(&TraceAlpha));
    }
}

void
TraceSinkRx(Ptr<const Packet> p, const Address& a)
{
    rxBytesWindow += p->GetSize();
    totalRxBytes += p->GetSize();
}

void
SampleQueueAndTput(Ptr<QueueDisc> qDisc, Time interval)
{
    uint32_t pkts = qDisc->GetNPackets();
    uint32_t bytes = qDisc->GetNBytes();
    double delayMs = (bytes * 8.0) / 50000.0;
    queueStream << std::fixed << std::setprecision(4) << Simulator::Now().GetSeconds() << ","
                << pkts << "," << delayMs << std::endl;

    double mbps = (rxBytesWindow * 8.0) / interval.GetSeconds() / 1e6;
    tputStream << std::fixed << std::setprecision(4) << Simulator::Now().GetSeconds() << "," << mbps
               << std::endl;
    rxBytesWindow = 0;

    Simulator::Schedule(interval, &SampleQueueAndTput, qDisc, interval);
}

void
Run(const std::string& tcpTypeId,
    Time baseRtt,
    bool enableEcn,
    Time stopTime,
    const std::string& prefix)
{
    rxBytesWindow = 0;
    totalRxBytes = 0;

    Config::SetDefault("ns3::TcpL4Protocol::SocketType", StringValue("ns3::" + tcpTypeId));
    Config::SetDefault("ns3::TcpSocket::SegmentSize", UintegerValue(1448));
    Config::SetDefault("ns3::TcpSocket::SndBufSize", UintegerValue(8192000));
    Config::SetDefault("ns3::TcpSocket::RcvBufSize", UintegerValue(8192000));
    Config::SetDefault("ns3::TcpSocket::InitialCwnd", UintegerValue(10));
    Config::SetDefault("ns3::TcpSocketState::EnablePacing", BooleanValue(true));
    Config::SetDefault("ns3::TcpSocketState::PaceInitialWindow", BooleanValue(true));
    Config::SetDefault("ns3::TcpSocketBase::UseEcn", StringValue(enableEcn ? "On" : "Off"));
    GlobalValue::Bind("ChecksumEnabled", BooleanValue(false));

    Config::SetDefault("ns3::RedQueueDisc::UseEcn", BooleanValue(enableEcn));
    Config::SetDefault("ns3::RedQueueDisc::UseHardDrop", BooleanValue(!enableEcn));
    Config::SetDefault("ns3::RedQueueDisc::MeanPktSize", UintegerValue(1500));
    Config::SetDefault("ns3::RedQueueDisc::QW", DoubleValue(1.0));
    Config::SetDefault("ns3::RedQueueDisc::MinTh", DoubleValue(17));
    Config::SetDefault("ns3::RedQueueDisc::MaxTh", DoubleValue(34));
    Config::SetDefault("ns3::RedQueueDisc::MaxSize", QueueSizeValue(QueueSize("500p")));

    Ptr<Node> sender = CreateObject<Node>();
    Ptr<Node> router1 = CreateObject<Node>();
    Ptr<Node> router2 = CreateObject<Node>();
    Ptr<Node> receiver = CreateObject<Node>();

    Time linkDelay = baseRtt / 4;
    Time accessDelay = MicroSeconds(0);

    PointToPointHelper p2pAccess1;
    p2pAccess1.SetDeviceAttribute("DataRate", StringValue("1Gbps"));
    p2pAccess1.SetChannelAttribute("Delay", TimeValue(linkDelay));

    PointToPointHelper p2pBottleneck;
    p2pBottleneck.SetQueue("ns3::DropTailQueue", "MaxSize", QueueSizeValue(QueueSize("1p")));
    p2pBottleneck.SetDeviceAttribute("DataRate", StringValue("50Mbps"));
    p2pBottleneck.SetChannelAttribute("Delay", TimeValue(linkDelay));

    PointToPointHelper p2pAccess2;
    p2pAccess2.SetDeviceAttribute("DataRate", StringValue("1Gbps"));
    p2pAccess2.SetChannelAttribute("Delay", TimeValue(accessDelay));

    NetDeviceContainer devS_R1 = p2pAccess1.Install(sender, router1);
    NetDeviceContainer devR1_R2 = p2pBottleneck.Install(router1, router2);
    NetDeviceContainer devR2_R = p2pAccess2.Install(router2, receiver);

    InternetStackHelper stack;
    stack.InstallAll();

    TrafficControlHelper tchRed;
    tchRed.SetRootQueueDisc("ns3::RedQueueDisc",
                            "LinkBandwidth",
                            StringValue("50Mbps"),
                            "LinkDelay",
                            TimeValue(linkDelay),
                            "MinTh",
                            DoubleValue(17),
                            "MaxTh",
                            DoubleValue(34));
    QueueDiscContainer qDiscs = tchRed.Install(devR1_R2.Get(0));

    Ipv4AddressHelper address;
    address.SetBase("10.1.1.0", "255.255.255.0");
    address.Assign(devS_R1);
    address.SetBase("10.1.2.0", "255.255.255.0");
    address.Assign(devR1_R2);
    address.SetBase("10.1.3.0", "255.255.255.0");
    Ipv4InterfaceContainer rxIfaces = address.Assign(devR2_R);

    Ipv4GlobalRoutingHelper::PopulateRoutingTables();

    uint16_t port = 50000;
    PacketSinkHelper sinkHelper("ns3::TcpSocketFactory",
                                InetSocketAddress(Ipv4Address::GetAny(), port));
    ApplicationContainer sinkApp = sinkHelper.Install(receiver);
    sinkApp.Get(0)->GetObject<PacketSink>()->TraceConnectWithoutContext("Rx",
                                                                        MakeCallback(&TraceSinkRx));
    sinkApp.Start(Seconds(0.0));
    sinkApp.Stop(stopTime);

    BulkSendHelper source("ns3::TcpSocketFactory",
                          InetSocketAddress(rxIfaces.GetAddress(1), port));
    source.SetAttribute("MaxBytes", UintegerValue(0));
    ApplicationContainer srcApp = source.Install(sender);
    srcApp.Start(Seconds(0.0));
    srcApp.Stop(stopTime);

    cwndStream.open(prefix + "-cwnd.csv", std::ios::out);
    cwndStream << "time_s,cwnd_segments" << std::endl;
    alphaStream.open(prefix + "-alpha.csv", std::ios::out);
    alphaStream << "time_s,bytes_marked,bytes_acked,alpha" << std::endl;
    queueStream.open(prefix + "-queue.csv", std::ios::out);
    queueStream << "time_s,queue_pkts,queue_delay_ms" << std::endl;
    tputStream.open(prefix + "-throughput.csv", std::ios::out);
    tputStream << "time_s,throughput_mbps" << std::endl;

    bool isDctcp = (tcpTypeId == "TcpDctcp");
    Simulator::Schedule(MicroSeconds(1), &ConnectSocketTraces, isDctcp);
    Simulator::Schedule(MilliSeconds(50), &SampleQueueAndTput, qDiscs.Get(0), MilliSeconds(50));
    Simulator::Stop(stopTime + TimeStep(1));
    Simulator::Run();

    cwndStream.close();
    alphaStream.close();
    queueStream.close();
    tputStream.close();
    Simulator::Destroy();
}
} // namespace SingleFlowSim

namespace MultiBottleneckSim
{
std::ofstream rxS1R1Throughput;
std::ofstream rxS2R2Throughput;
std::ofstream rxS3R1Throughput;
std::ofstream fairnessIndex;
std::ofstream t1QueueLength;
std::ofstream t2QueueLength;
std::vector<uint64_t> rxS1R1Bytes;
std::vector<uint64_t> rxS2R2Bytes;
std::vector<uint64_t> rxS3R1Bytes;

void
TraceS1R1Sink(std::size_t index, Ptr<const Packet> p, const Address& a)
{
    rxS1R1Bytes[index] += p->GetSize();
}

void
TraceS2R2Sink(std::size_t index, Ptr<const Packet> p, const Address& a)
{
    rxS2R2Bytes[index] += p->GetSize();
}

void
TraceS3R1Sink(std::size_t index, Ptr<const Packet> p, const Address& a)
{
    rxS3R1Bytes[index] += p->GetSize();
}

void
InitializeCounters()
{
    std::fill(rxS1R1Bytes.begin(), rxS1R1Bytes.end(), 0);
    std::fill(rxS2R2Bytes.begin(), rxS2R2Bytes.end(), 0);
    std::fill(rxS3R1Bytes.begin(), rxS3R1Bytes.end(), 0);
}

void
PrintThroughput(Time measurementWindow)
{
    for (std::size_t i = 0; i < 10; i++)
    {
        rxS1R1Throughput << measurementWindow.GetSeconds() << "s " << i << " "
                         << (rxS1R1Bytes[i] * 8.0) / measurementWindow.GetSeconds() / 1e6
                         << std::endl;
    }
    for (std::size_t i = 0; i < 20; i++)
    {
        rxS2R2Throughput << Simulator::Now().GetSeconds() << "s " << i << " "
                         << (rxS2R2Bytes[i] * 8.0) / measurementWindow.GetSeconds() / 1e6
                         << std::endl;
    }
    for (std::size_t i = 0; i < 10; i++)
    {
        rxS3R1Throughput << Simulator::Now().GetSeconds() << "s " << i << " "
                         << (rxS3R1Bytes[i] * 8.0) / measurementWindow.GetSeconds() / 1e6
                         << std::endl;
    }
}

void
PrintFairness(Time measurementWindow)
{
    auto calcGroup = [&](const std::vector<uint64_t>& bytes, std::size_t n, const std::string& label) {
        double sum = 0;
        double sumSquares = 0;
        for (std::size_t i = 0; i < n; i++)
        {
            double b = static_cast<double>(bytes[i]);
            sum += b;
            sumSquares += b * b;
        }
        double avgMbps = ((sum / n) * 8.0 / measurementWindow.GetSeconds()) / 1e6;
        double fairness = (sumSquares > 0) ? (sum * sum) / (n * sumSquares) : 0.0;
        fairnessIndex << "Average throughput for " << label << " flows: " << std::fixed
                      << std::setprecision(2) << avgMbps << " Mbps; fairness: " << std::fixed
                      << std::setprecision(3) << fairness << std::endl;
    };

    calcGroup(rxS1R1Bytes, 10, "S1-R1");
    calcGroup(rxS2R2Bytes, 20, "S2-R2");
    calcGroup(rxS3R1Bytes, 10, "S3-R1");

    double normSum = 0;
    double normSumSq = 0;
    for (std::size_t i = 0; i < 10; i++)
    {
        double r = ((rxS1R1Bytes[i] * 8.0 / measurementWindow.GetSeconds()) / 1e6) / 50.0;
        normSum += r;
        normSumSq += r * r;
    }
    for (std::size_t i = 0; i < 20; i++)
    {
        double r = ((rxS2R2Bytes[i] * 8.0 / measurementWindow.GetSeconds()) / 1e6) / 475.0;
        normSum += r;
        normSumSq += r * r;
    }
    for (std::size_t i = 0; i < 10; i++)
    {
        double r = ((rxS3R1Bytes[i] * 8.0 / measurementWindow.GetSeconds()) / 1e6) / 50.0;
        normSum += r;
        normSumSq += r * r;
    }
    double overallFairness = (normSumSq > 0) ? (normSum * normSum) / (40.0 * normSumSq) : 0.0;

    uint64_t sumT1 = 0;
    for (std::size_t i = 0; i < 10; i++)
    {
        sumT1 += rxS1R1Bytes[i];
    }
    for (std::size_t i = 0; i < 20; i++)
    {
        sumT1 += rxS2R2Bytes[i];
    }
    fairnessIndex << "Aggregate user-level throughput for flows through T1: "
                  << static_cast<double>(sumT1 * 8) / measurementWindow.GetSeconds() / 1e9
                  << " Gbps" << std::endl;

    uint64_t sumR1 = 0;
    for (std::size_t i = 0; i < 10; i++)
    {
        sumR1 += rxS3R1Bytes[i] + rxS1R1Bytes[i];
    }
    fairnessIndex << "Aggregate user-level throughput for flows to R1: "
                  << static_cast<double>(sumR1 * 8) / measurementWindow.GetSeconds() / 1e9
                  << " Gbps" << std::endl;
    fairnessIndex << "Normalized overall Jain fairness index (40 flows): " << std::fixed
                  << std::setprecision(4) << overallFairness << std::endl;
}

void
CheckT1QueueSize(Ptr<QueueDisc> queue)
{
    uint32_t qSize = queue->GetNPackets();
    Time backlog = Seconds(static_cast<double>(qSize * 1500 * 8) / 1e10);
    t1QueueLength << std::fixed << std::setprecision(3) << Simulator::Now().GetSeconds() << " "
                  << qSize << " " << backlog.GetMicroSeconds() << std::endl;
    Simulator::Schedule(MilliSeconds(10), &CheckT1QueueSize, queue);
}

void
CheckT2QueueSize(Ptr<QueueDisc> queue)
{
    uint32_t qSize = queue->GetNPackets();
    Time backlog = Seconds(static_cast<double>(qSize * 1500 * 8) / 1e9);
    t2QueueLength << std::fixed << std::setprecision(3) << Simulator::Now().GetSeconds() << " "
                  << qSize << " " << backlog.GetMicroSeconds() << std::endl;
    Simulator::Schedule(MilliSeconds(10), &CheckT2QueueSize, queue);
}

void
Run(const std::string& tcpTypeId,
    Time flowStartupWindow,
    Time convergenceTime,
    Time measurementWindow,
    bool enableSwitchEcn,
    bool instantMarking,
    double k1G,
    double k10G,
    const std::string& prefix)
{
    Config::SetDefault("ns3::TcpL4Protocol::SocketType", StringValue("ns3::" + tcpTypeId));

    Time startTime = Seconds(0);
    Time stopTime = flowStartupWindow + convergenceTime + measurementWindow;

    rxS1R1Bytes.assign(10, 0);
    rxS2R2Bytes.assign(20, 0);
    rxS3R1Bytes.assign(10, 0);

    NodeContainer S1;
    NodeContainer S2;
    NodeContainer S3;
    NodeContainer R2;
    Ptr<Node> T1 = CreateObject<Node>();
    Ptr<Node> T2 = CreateObject<Node>();
    Ptr<Node> R1 = CreateObject<Node>();
    S1.Create(10);
    S2.Create(20);
    S3.Create(10);
    R2.Create(20);

    Config::SetDefault("ns3::TcpSocket::SegmentSize", UintegerValue(1448));
    Config::SetDefault("ns3::TcpSocket::DelAckCount", UintegerValue(2));
    GlobalValue::Bind("ChecksumEnabled", BooleanValue(false));

    Config::SetDefault("ns3::RedQueueDisc::UseEcn", BooleanValue(enableSwitchEcn));
    Config::SetDefault("ns3::RedQueueDisc::UseHardDrop", BooleanValue(false));
    Config::SetDefault("ns3::RedQueueDisc::MeanPktSize", UintegerValue(1500));
    Config::SetDefault("ns3::RedQueueDisc::MaxSize", QueueSizeValue(QueueSize("2666p")));
    Config::SetDefault("ns3::RedQueueDisc::QW", DoubleValue(1));

    double minTh1G = instantMarking ? k1G : 20.0;
    double maxTh1G = instantMarking ? k1G : 60.0;
    double minTh10G = instantMarking ? k10G : 50.0;
    double maxTh10G = instantMarking ? k10G : 150.0;

    Config::SetDefault("ns3::RedQueueDisc::MinTh", DoubleValue(minTh1G));
    Config::SetDefault("ns3::RedQueueDisc::MaxTh", DoubleValue(maxTh1G));

    PointToPointHelper pointToPointSR;
    pointToPointSR.SetDeviceAttribute("DataRate", StringValue("1Gbps"));
    pointToPointSR.SetChannelAttribute("Delay", StringValue("10us"));

    PointToPointHelper pointToPointT;
    pointToPointT.SetDeviceAttribute("DataRate", StringValue("10Gbps"));
    pointToPointT.SetChannelAttribute("Delay", StringValue("10us"));

    std::vector<NetDeviceContainer> S1T1;
    std::vector<NetDeviceContainer> S2T1;
    std::vector<NetDeviceContainer> S3T2;
    std::vector<NetDeviceContainer> R2T2;
    NetDeviceContainer T1T2 = pointToPointT.Install(T1, T2);
    NetDeviceContainer R1T2 = pointToPointSR.Install(R1, T2);

    for (std::size_t i = 0; i < 10; i++)
    {
        S1T1.push_back(pointToPointSR.Install(S1.Get(i), T1));
    }
    for (std::size_t i = 0; i < 20; i++)
    {
        S2T1.push_back(pointToPointSR.Install(S2.Get(i), T1));
    }
    for (std::size_t i = 0; i < 10; i++)
    {
        S3T2.push_back(pointToPointSR.Install(S3.Get(i), T2));
    }
    for (std::size_t i = 0; i < 20; i++)
    {
        R2T2.push_back(pointToPointSR.Install(R2.Get(i), T2));
    }

    InternetStackHelper stack;
    stack.InstallAll();

    TrafficControlHelper tchRed10;
    tchRed10.SetRootQueueDisc("ns3::RedQueueDisc",
                              "LinkBandwidth",
                              StringValue("10Gbps"),
                              "LinkDelay",
                              StringValue("10us"),
                              "MinTh",
                              DoubleValue(minTh10G),
                              "MaxTh",
                              DoubleValue(maxTh10G));
    QueueDiscContainer queueDiscs1 = tchRed10.Install(T1T2);

    TrafficControlHelper tchRed1;
    tchRed1.SetRootQueueDisc("ns3::RedQueueDisc",
                             "LinkBandwidth",
                             StringValue("1Gbps"),
                             "LinkDelay",
                             StringValue("10us"),
                             "MinTh",
                             DoubleValue(minTh1G),
                             "MaxTh",
                             DoubleValue(maxTh1G));
    QueueDiscContainer queueDiscs2 = tchRed1.Install(R1T2.Get(1));
    for (std::size_t i = 0; i < 10; i++)
    {
        tchRed1.Install(S1T1[i].Get(1));
    }
    for (std::size_t i = 0; i < 20; i++)
    {
        tchRed1.Install(S2T1[i].Get(1));
    }
    for (std::size_t i = 0; i < 10; i++)
    {
        tchRed1.Install(S3T2[i].Get(1));
    }
    for (std::size_t i = 0; i < 20; i++)
    {
        tchRed1.Install(R2T2[i].Get(1));
    }

    Ipv4AddressHelper address;
    std::vector<Ipv4InterfaceContainer> ipS1T1;
    std::vector<Ipv4InterfaceContainer> ipS2T1;
    std::vector<Ipv4InterfaceContainer> ipS3T2;
    std::vector<Ipv4InterfaceContainer> ipR2T2;
    address.SetBase("172.16.1.0", "255.255.255.0");
    address.Assign(T1T2);
    address.SetBase("192.168.0.0", "255.255.255.0");
    Ipv4InterfaceContainer ipR1T2 = address.Assign(R1T2);
    address.SetBase("10.1.1.0", "255.255.255.0");
    for (std::size_t i = 0; i < 10; i++)
    {
        ipS1T1.push_back(address.Assign(S1T1[i]));
        address.NewNetwork();
    }
    address.SetBase("10.2.1.0", "255.255.255.0");
    for (std::size_t i = 0; i < 20; i++)
    {
        ipS2T1.push_back(address.Assign(S2T1[i]));
        address.NewNetwork();
    }
    address.SetBase("10.3.1.0", "255.255.255.0");
    for (std::size_t i = 0; i < 10; i++)
    {
        ipS3T2.push_back(address.Assign(S3T2[i]));
        address.NewNetwork();
    }
    address.SetBase("10.4.1.0", "255.255.255.0");
    for (std::size_t i = 0; i < 20; i++)
    {
        ipR2T2.push_back(address.Assign(R2T2[i]));
        address.NewNetwork();
    }

    Ipv4GlobalRoutingHelper::PopulateRoutingTables();

    std::vector<Ptr<PacketSink>> r2Sinks;
    for (std::size_t i = 0; i < 20; i++)
    {
        uint16_t port = 50000 + i;
        Address sinkLocalAddress(InetSocketAddress(Ipv4Address::GetAny(), port));
        PacketSinkHelper sinkHelper("ns3::TcpSocketFactory", sinkLocalAddress);
        ApplicationContainer sinkApp = sinkHelper.Install(R2.Get(i));
        r2Sinks.push_back(sinkApp.Get(0)->GetObject<PacketSink>());
        sinkApp.Start(startTime);
        sinkApp.Stop(stopTime);

        OnOffHelper clientHelper1("ns3::TcpSocketFactory", Address());
        clientHelper1.SetAttribute("OnTime",
                                   StringValue("ns3::ConstantRandomVariable[Constant=1]"));
        clientHelper1.SetAttribute("OffTime",
                                   StringValue("ns3::ConstantRandomVariable[Constant=0]"));
        clientHelper1.SetAttribute("DataRate", DataRateValue(DataRate("1Gbps")));
        clientHelper1.SetAttribute("PacketSize", UintegerValue(1000));

        AddressValue remoteAddress(InetSocketAddress(ipR2T2[i].GetAddress(0), port));
        clientHelper1.SetAttribute("Remote", remoteAddress);
        ApplicationContainer clientApps1 = clientHelper1.Install(S2.Get(i));
        clientApps1.Start(i * flowStartupWindow / 20 + startTime + MilliSeconds(i * 5));
        clientApps1.Stop(stopTime);
    }

    std::vector<Ptr<PacketSink>> s1r1Sinks;
    std::vector<Ptr<PacketSink>> s3r1Sinks;
    for (std::size_t i = 0; i < 20; i++)
    {
        uint16_t port = 50000 + i;
        Address sinkLocalAddress(InetSocketAddress(Ipv4Address::GetAny(), port));
        PacketSinkHelper sinkHelper("ns3::TcpSocketFactory", sinkLocalAddress);
        ApplicationContainer sinkApp = sinkHelper.Install(R1);
        Ptr<PacketSink> packetSink = sinkApp.Get(0)->GetObject<PacketSink>();
        if (i < 10)
        {
            s1r1Sinks.push_back(packetSink);
        }
        else
        {
            s3r1Sinks.push_back(packetSink);
        }
        sinkApp.Start(startTime);
        sinkApp.Stop(stopTime);

        OnOffHelper clientHelper1("ns3::TcpSocketFactory", Address());
        clientHelper1.SetAttribute("OnTime",
                                   StringValue("ns3::ConstantRandomVariable[Constant=1]"));
        clientHelper1.SetAttribute("OffTime",
                                   StringValue("ns3::ConstantRandomVariable[Constant=0]"));
        clientHelper1.SetAttribute("DataRate", DataRateValue(DataRate("1Gbps")));
        clientHelper1.SetAttribute("PacketSize", UintegerValue(1000));

        AddressValue remoteAddress(InetSocketAddress(ipR1T2.GetAddress(0), port));
        clientHelper1.SetAttribute("Remote", remoteAddress);
        ApplicationContainer clientApps1;
        if (i < 10)
        {
            clientApps1.Add(clientHelper1.Install(S1.Get(i)));
            clientApps1.Start(i * flowStartupWindow / 10 + startTime + MilliSeconds(i * 5));
        }
        else
        {
            clientApps1.Add(clientHelper1.Install(S3.Get(i - 10)));
            clientApps1.Start((i - 10) * flowStartupWindow / 10 + startTime + MilliSeconds(i * 5));
        }
        clientApps1.Stop(stopTime);
    }

    rxS1R1Throughput.open(prefix + "-s1-r1-throughput.dat", std::ios::out);
    rxS1R1Throughput << "#Time(s) flow thruput(Mb/s)" << std::endl;
    rxS2R2Throughput.open(prefix + "-s2-r2-throughput.dat", std::ios::out);
    rxS2R2Throughput << "#Time(s) flow thruput(Mb/s)" << std::endl;
    rxS3R1Throughput.open(prefix + "-s3-r1-throughput.dat", std::ios::out);
    rxS3R1Throughput << "#Time(s) flow thruput(Mb/s)" << std::endl;
    fairnessIndex.open(prefix + "-fairness.dat", std::ios::out);
    t1QueueLength.open(prefix + "-t1-length.dat", std::ios::out);
    t1QueueLength << "#Time(s) qlen(pkts) qlen(us)" << std::endl;
    t2QueueLength.open(prefix + "-t2-length.dat", std::ios::out);
    t2QueueLength << "#Time(s) qlen(pkts) qlen(us)" << std::endl;

    for (std::size_t i = 0; i < 10; i++)
    {
        s1r1Sinks[i]->TraceConnectWithoutContext("Rx", MakeBoundCallback(&TraceS1R1Sink, i));
    }
    for (std::size_t i = 0; i < 20; i++)
    {
        r2Sinks[i]->TraceConnectWithoutContext("Rx", MakeBoundCallback(&TraceS2R2Sink, i));
    }
    for (std::size_t i = 0; i < 10; i++)
    {
        s3r1Sinks[i]->TraceConnectWithoutContext("Rx", MakeBoundCallback(&TraceS3R1Sink, i));
    }

    Simulator::Schedule(flowStartupWindow + convergenceTime, &InitializeCounters);
    Simulator::Schedule(flowStartupWindow + convergenceTime + measurementWindow,
                        &PrintThroughput,
                        measurementWindow);
    Simulator::Schedule(flowStartupWindow + convergenceTime + measurementWindow,
                        &PrintFairness,
                        measurementWindow);
    Simulator::Schedule(flowStartupWindow + convergenceTime, &CheckT1QueueSize, queueDiscs1.Get(0));
    Simulator::Schedule(flowStartupWindow + convergenceTime, &CheckT2QueueSize, queueDiscs2.Get(0));
    Simulator::Stop(stopTime + TimeStep(1));

    Simulator::Run();

    rxS1R1Throughput.close();
    rxS2R2Throughput.close();
    rxS3R1Throughput.close();
    fairnessIndex.close();
    t1QueueLength.close();
    t2QueueLength.close();
    Simulator::Destroy();
}
} // namespace MultiBottleneckSim

namespace QueueCdfSim
{
std::ofstream qTimeSeriesFile;
uint64_t totalRxBytes = 0;

void
TraceSinkRx(Ptr<const Packet> p, const Address& a)
{
    totalRxBytes += p->GetSize();
}

void
SampleQueue(Ptr<QueueDisc> qDisc, Time interval)
{
    uint32_t pkts = qDisc->GetNPackets();
    uint32_t bytes = qDisc->GetNBytes();
    double kb = bytes / 1000.0;
    qTimeSeriesFile << std::fixed << std::setprecision(4) << Simulator::Now().GetSeconds() << ","
                    << pkts << "," << kb << std::endl;
    Simulator::Schedule(interval, &SampleQueue, qDisc, interval);
}

void
Run(const std::string& tcpTypeId,
    uint32_t nFlows,
    double kThreshold,
    bool enableEcn,
    Time stopTime,
    const std::string& prefix)
{
    totalRxBytes = 0;
    Config::SetDefault("ns3::TcpL4Protocol::SocketType", StringValue("ns3::" + tcpTypeId));
    Config::SetDefault("ns3::TcpSocket::SegmentSize", UintegerValue(1448));
    Config::SetDefault("ns3::TcpSocket::DelAckCount", UintegerValue(2));
    Config::SetDefault("ns3::TcpSocket::SndBufSize", UintegerValue(8192000));
    Config::SetDefault("ns3::TcpSocket::RcvBufSize", UintegerValue(8192000));
    GlobalValue::Bind("ChecksumEnabled", BooleanValue(false));

    Config::SetDefault("ns3::RedQueueDisc::UseEcn", BooleanValue(enableEcn));
    Config::SetDefault("ns3::RedQueueDisc::UseHardDrop", BooleanValue(!enableEcn));
    Config::SetDefault("ns3::RedQueueDisc::MeanPktSize", UintegerValue(1500));
    Config::SetDefault("ns3::RedQueueDisc::MaxSize", QueueSizeValue(QueueSize("466p")));
    Config::SetDefault("ns3::RedQueueDisc::QW", DoubleValue(1.0));
    double th = enableEcn ? kThreshold : 466.0;
    Config::SetDefault("ns3::RedQueueDisc::MinTh", DoubleValue(th));
    Config::SetDefault("ns3::RedQueueDisc::MaxTh", DoubleValue(th));

    NodeContainer senders;
    senders.Create(nFlows);
    Ptr<Node> switchNode = CreateObject<Node>();
    Ptr<Node> receiver = CreateObject<Node>();

    PointToPointHelper p2p;
    p2p.SetDeviceAttribute("DataRate", StringValue("1Gbps"));
    p2p.SetChannelAttribute("Delay", StringValue("25us"));

    std::vector<NetDeviceContainer> senderLinks;
    for (uint32_t i = 0; i < nFlows; i++)
    {
        senderLinks.push_back(p2p.Install(senders.Get(i), switchNode));
    }
    NetDeviceContainer bottleneckLink = p2p.Install(switchNode, receiver);

    InternetStackHelper stack;
    stack.InstallAll();

    TrafficControlHelper tch;
    tch.SetRootQueueDisc("ns3::RedQueueDisc",
                         "LinkBandwidth",
                         StringValue("1Gbps"),
                         "LinkDelay",
                         StringValue("25us"),
                         "MinTh",
                         DoubleValue(th),
                         "MaxTh",
                         DoubleValue(th));
    QueueDiscContainer qDiscs = tch.Install(bottleneckLink.Get(0));

    Ipv4AddressHelper address;
    address.SetBase("10.1.1.0", "255.255.255.0");
    for (uint32_t i = 0; i < nFlows; i++)
    {
        address.Assign(senderLinks[i]);
        address.NewNetwork();
    }
    address.SetBase("10.2.1.0", "255.255.255.0");
    Ipv4InterfaceContainer rxIfaces = address.Assign(bottleneckLink);

    Ipv4GlobalRoutingHelper::PopulateRoutingTables();

    for (uint32_t i = 0; i < nFlows; i++)
    {
        uint16_t port = 50000 + i;
        PacketSinkHelper sinkHelper("ns3::TcpSocketFactory",
                                    InetSocketAddress(Ipv4Address::GetAny(), port));
        ApplicationContainer sinkApp = sinkHelper.Install(receiver);
        sinkApp.Get(0)->GetObject<PacketSink>()->TraceConnectWithoutContext(
            "Rx",
            MakeCallback(&TraceSinkRx));
        sinkApp.Start(Seconds(0.0));
        sinkApp.Stop(stopTime);

        BulkSendHelper source("ns3::TcpSocketFactory",
                              InetSocketAddress(rxIfaces.GetAddress(1), port));
        source.SetAttribute("MaxBytes", UintegerValue(0));
        ApplicationContainer srcApp = source.Install(senders.Get(i));
        srcApp.Start(Seconds(0.01) + MicroSeconds(i * 500));
        srcApp.Stop(stopTime);
    }

    qTimeSeriesFile.open(prefix + "-queue.csv", std::ios::out);
    qTimeSeriesFile << "time_s,queue_pkts,queue_kb" << std::endl;

    Simulator::Schedule(Seconds(0.1), &SampleQueue, qDiscs.Get(0), MilliSeconds(1));
    Simulator::Stop(stopTime + TimeStep(1));
    Simulator::Run();

    qTimeSeriesFile.close();

    std::ofstream summaryFile(prefix + "-summary.txt", std::ios::out);
    double duration = stopTime.GetSeconds() - 0.01;
    double throughputGbps = (totalRxBytes * 8.0) / duration / 1e9;
    summaryFile << "protocol=" << tcpTypeId << "\n"
                << "nFlows=" << nFlows << "\n"
                << "enableEcn=" << enableEcn << "\n"
                << "K=" << kThreshold << "\n"
                << "aggregateThroughputGbps=" << std::fixed << std::setprecision(4)
                << throughputGbps << std::endl;
    summaryFile.close();

    Simulator::Destroy();
}
} // namespace QueueCdfSim

namespace ConvergenceSim
{
std::ofstream convFile;
std::vector<uint64_t> flowBytes(5, 0);

void
TraceFlowRx(std::size_t idx, Ptr<const Packet> p, const Address& a)
{
    flowBytes[idx] += p->GetSize();
}

void
SampleThroughput(Time interval)
{
    convFile << std::fixed << std::setprecision(2) << Simulator::Now().GetSeconds();
    for (std::size_t i = 0; i < 5; i++)
    {
        double gbps = (flowBytes[i] * 8.0) / interval.GetSeconds() / 1e9;
        convFile << "," << std::fixed << std::setprecision(4) << gbps;
        flowBytes[i] = 0;
    }
    convFile << std::endl;
    Simulator::Schedule(interval, &SampleThroughput, interval);
}

void
Run(const std::string& tcpTypeId, bool enableEcn, Time stepTime, const std::string& prefix)
{
    std::fill(flowBytes.begin(), flowBytes.end(), 0);
    Config::SetDefault("ns3::TcpL4Protocol::SocketType", StringValue("ns3::" + tcpTypeId));
    Config::SetDefault("ns3::TcpSocket::SegmentSize", UintegerValue(1448));
    Config::SetDefault("ns3::TcpSocket::DelAckCount", UintegerValue(2));
    Config::SetDefault("ns3::TcpSocket::SndBufSize", UintegerValue(8192000));
    Config::SetDefault("ns3::TcpSocket::RcvBufSize", UintegerValue(8192000));
    GlobalValue::Bind("ChecksumEnabled", BooleanValue(false));

    double th = enableEcn ? 20.0 : 466.0;
    Config::SetDefault("ns3::RedQueueDisc::UseEcn", BooleanValue(enableEcn));
    Config::SetDefault("ns3::RedQueueDisc::UseHardDrop", BooleanValue(!enableEcn));
    Config::SetDefault("ns3::RedQueueDisc::MeanPktSize", UintegerValue(1500));
    Config::SetDefault("ns3::RedQueueDisc::MaxSize", QueueSizeValue(QueueSize("466p")));
    Config::SetDefault("ns3::RedQueueDisc::QW", DoubleValue(1.0));
    Config::SetDefault("ns3::RedQueueDisc::MinTh", DoubleValue(th));
    Config::SetDefault("ns3::RedQueueDisc::MaxTh", DoubleValue(th));

    NodeContainer senders;
    senders.Create(5);
    Ptr<Node> switchNode = CreateObject<Node>();
    Ptr<Node> receiver = CreateObject<Node>();

    PointToPointHelper p2p;
    p2p.SetDeviceAttribute("DataRate", StringValue("1Gbps"));
    p2p.SetChannelAttribute("Delay", StringValue("25us"));

    std::vector<NetDeviceContainer> senderLinks;
    for (uint32_t i = 0; i < 5; i++)
    {
        senderLinks.push_back(p2p.Install(senders.Get(i), switchNode));
    }
    NetDeviceContainer bottleneckLink = p2p.Install(switchNode, receiver);

    InternetStackHelper stack;
    stack.InstallAll();

    TrafficControlHelper tch;
    tch.SetRootQueueDisc("ns3::RedQueueDisc",
                         "LinkBandwidth",
                         StringValue("1Gbps"),
                         "LinkDelay",
                         StringValue("25us"),
                         "MinTh",
                         DoubleValue(th),
                         "MaxTh",
                         DoubleValue(th));
    tch.Install(bottleneckLink.Get(0));

    Ipv4AddressHelper address;
    address.SetBase("10.1.1.0", "255.255.255.0");
    for (uint32_t i = 0; i < 5; i++)
    {
        address.Assign(senderLinks[i]);
        address.NewNetwork();
    }
    address.SetBase("10.2.1.0", "255.255.255.0");
    Ipv4InterfaceContainer rxIfaces = address.Assign(bottleneckLink);

    Ipv4GlobalRoutingHelper::PopulateRoutingTables();

    Time totalTime = stepTime * 10;
    for (uint32_t i = 0; i < 5; i++)
    {
        uint16_t port = 50000 + i;
        PacketSinkHelper sinkHelper("ns3::TcpSocketFactory",
                                    InetSocketAddress(Ipv4Address::GetAny(), port));
        ApplicationContainer sinkApp = sinkHelper.Install(receiver);
        sinkApp.Get(0)->GetObject<PacketSink>()->TraceConnectWithoutContext(
            "Rx",
            MakeBoundCallback(&TraceFlowRx, i));
        sinkApp.Start(Seconds(0.0));
        sinkApp.Stop(totalTime);

        BulkSendHelper source("ns3::TcpSocketFactory",
                              InetSocketAddress(rxIfaces.GetAddress(1), port));
        source.SetAttribute("MaxBytes", UintegerValue(0));
        ApplicationContainer srcApp = source.Install(senders.Get(i));
        srcApp.Start(i * stepTime);
        srcApp.Stop((10 - i) * stepTime);
    }

    convFile.open(prefix + "-convergence.csv", std::ios::out);
    convFile << "time_s,flow1_gbps,flow2_gbps,flow3_gbps,flow4_gbps,flow5_gbps" << std::endl;

    Time sampleInt = (stepTime >= Seconds(10)) ? Seconds(1.0) : MilliSeconds(200);
    Simulator::Schedule(sampleInt, &SampleThroughput, sampleInt);
    Simulator::Stop(totalTime + TimeStep(1));
    Simulator::Run();

    convFile.close();
    Simulator::Destroy();
}
} // namespace ConvergenceSim

int
main(int argc, char* argv[])
{
    std::string mode = "multi-bottleneck";
    std::string tcpTypeId = "TcpDctcp";
    Time baseRtt = MilliSeconds(10);
    Time flowStartupWindow = Seconds(1);
    Time convergenceTime = Seconds(3);
    Time measurementWindow = Seconds(1);
    bool enableSwitchEcn = true;
    bool instantMarking = false;
    double k1G = 20.0;
    double k10G = 65.0;
    uint32_t nFlows = 2;
    Time stopTime = Seconds(6.0);
    Time stepTime = Seconds(5.0);
    uint32_t rngRun = 1;
    std::string prefix = "dctcp-exp";

    CommandLine cmd(__FILE__);
    cmd.AddValue("mode",
                 "Simulation mode: single-flow | multi-bottleneck | queue-cdf | convergence",
                 mode);
    cmd.AddValue("tcpTypeId", "ns-3 TCP TypeId (TcpDctcp, TcpLinuxReno, TcpNewReno)", tcpTypeId);
    cmd.AddValue("baseRtt", "Base RTT for single-flow mode", baseRtt);
    cmd.AddValue("flowStartupWindow", "Startup window for multi-bottleneck", flowStartupWindow);
    cmd.AddValue("convergenceTime", "Convergence time for multi-bottleneck", convergenceTime);
    cmd.AddValue("measurementWindow", "Measurement window for multi-bottleneck", measurementWindow);
    cmd.AddValue("enableSwitchEcn", "Enable ECN at switches", enableSwitchEcn);
    cmd.AddValue("instantMarking", "Set MinTh=MaxTh=K as in SIGCOMM'10 paper", instantMarking);
    cmd.AddValue("k1G", "Marking threshold K at 1Gbps (packets)", k1G);
    cmd.AddValue("k10G", "Marking threshold K at 10Gbps (packets)", k10G);
    cmd.AddValue("nFlows", "Number of flows for queue-cdf mode", nFlows);
    cmd.AddValue("stopTime", "Stop time for single-flow / queue-cdf mode", stopTime);
    cmd.AddValue("stepTime", "Step time for convergence mode", stepTime);
    cmd.AddValue("rngRun", "RNG run number (seed)", rngRun);
    cmd.AddValue("prefix", "Output file prefix (including path)", prefix);
    cmd.Parse(argc, argv);

    RngSeedManager::SetSeed(1);
    RngSeedManager::SetRun(rngRun);

    if (mode == "single-flow")
    {
        SingleFlowSim::Run(tcpTypeId, baseRtt, enableSwitchEcn, stopTime, prefix);
    }
    else if (mode == "multi-bottleneck")
    {
        MultiBottleneckSim::Run(tcpTypeId,
                                flowStartupWindow,
                                convergenceTime,
                                measurementWindow,
                                enableSwitchEcn,
                                instantMarking,
                                k1G,
                                k10G,
                                prefix);
    }
    else if (mode == "queue-cdf")
    {
        QueueCdfSim::Run(tcpTypeId, nFlows, k1G, enableSwitchEcn, stopTime, prefix);
    }
    else if (mode == "convergence")
    {
        ConvergenceSim::Run(tcpTypeId, enableSwitchEcn, stepTime, prefix);
    }
    else
    {
        NS_FATAL_ERROR("Unknown mode: " << mode);
    }

    return 0;
}

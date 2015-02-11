#include "networkgenerator.h"
#include "ns3/double.h"

using namespace ns3;
using namespace ns3::ndn;

NS_LOG_COMPONENT_DEFINE ("NetworkGenerator");

NetworkGenerator::NetworkGenerator(std::string conf_file)
{
  rvariable = CreateObject<UniformRandomVariable>();
  this->briteHelper = new NDNBriteHelper(conf_file);
  briteHelper->BuildBriteTopology ();

  for(int i=0; i<getAllASNodes ().size (); i++)
  {
    Names::Add (std::string("Node_" + boost::lexical_cast<std::string>(i)), getAllASNodes ().Get (i));
  }
}

void NetworkGenerator::randomlyPlaceNodes(int nodeCount, std::string setIdentifier, NodePlacement place, PointToPointHelper *p2p)
{
  std::vector<int> allAS;

  for(int i=0; i < getNumberOfAS (); i++)
    allAS.push_back (i);

  randomlyPlaceNodes(nodeCount,setIdentifier,place,p2p,allAS);
}

void NetworkGenerator::randomlyPlaceNodes (int nodeCount, std::string setIdentifier, NodePlacement place, PointToPointHelper *p2p, std::vector<int> ASnumbers)
{
  NodeContainer container;

  for(std::vector<int>::iterator it = ASnumbers.begin (); it != ASnumbers.end (); it++)
  {
    if(place == NetworkGenerator::ASNode)
    {
      container.Add (getAllASNodesFromAS(*it));
    }
    else
    {
      container.Add (getAllLeafNodesFromAS(*it));
    }
  }

  if(container.size () == 0)
  {
    NS_LOG_UNCOND("Could not place nodes, as no nodes are provided by the topology.");
    return;
  }

  NodeContainer customNodes;
  customNodes.Create (nodeCount);

  for(int i=0; i<customNodes.size (); i++)
  {
    Names::Add (std::string(setIdentifier + "_" + boost::lexical_cast<std::string>(i)), customNodes.Get (i));

    int rand = rvariable->GetInteger (0,container.size ()-1);
    p2p->Install (customNodes.Get (i), container.Get (rand));
  }
  nodeContainerMap[setIdentifier] = customNodes;
}

void NetworkGenerator::randomlyAddConnectionsBetweenAllAS(int numberOfConnectionsPerAsPair, int minBW_kbits, int maxBw_kbits, int minDelay_ms, int maxDelay_ms)
{
  PointToPointHelper p2p;

  for(int i = 0; i<getNumberOfAS (); i++)
  {
    int j = i+1;
    while(j < getNumberOfAS ())
    {
      for(int c = 0; c < numberOfConnectionsPerAsPair; c++)
      {
        std::string delay(boost::lexical_cast<std::string>(rvariable->GetValue (minDelay_ms,maxDelay_ms)));
        delay.append ("ms");

        std::string bw(boost::lexical_cast<std::string>(rvariable->GetValue (minBW_kbits,maxBw_kbits)));
        bw.append ("Kbps");

        p2p.SetDeviceAttribute ("DataRate", StringValue (bw));
        p2p.SetChannelAttribute ("Delay", StringValue (delay));

        NodeContainer container = getPairOfUnconnectedNodes(i, j);
        if(container.size () == 2)
          p2p.Install (container.Get (0), container.Get (1));
        else
          NS_LOG_UNCOND("Unable to add new Connections");
      }
      j++;
    }
  }
}

void NetworkGenerator::randomlyAddConnectionsBetweenTwoAS(int numberOfConnectionsPairs, int minBW_kbits, int maxBw_kbits, int minDelay_ms, int maxDelay_ms)
{
  if(getNumberOfAS() <= 1)
  {
    NS_LOG_UNCOND("Error, at least two AS have to exists to add Connections between ASs!");
    return;
  }

  PointToPointHelper p2p;

  for(int i = 0; i < numberOfConnectionsPairs; i++)
  {
    std::string delay(boost::lexical_cast<std::string>(rvariable->GetValue (minDelay_ms,maxDelay_ms)));
    delay.append ("ms");

    std::string bw(boost::lexical_cast<std::string>(rvariable->GetValue (minBW_kbits,maxBw_kbits)));
    bw.append ("Kbps");

    p2p.SetDeviceAttribute ("DataRate", StringValue (bw));
    p2p.SetChannelAttribute ("Delay", StringValue (delay));

    int number_as1 = rvariable->GetInteger (0,getNumberOfAS()-1);
    int number_as2 = number_as1;

    while(number_as2 == number_as1)
      number_as2 = rvariable->GetInteger (0,getNumberOfAS()-1);

    NodeContainer container = getPairOfUnconnectedNodes(number_as1, number_as2);
    if(container.size () == 2)
      p2p.Install (container.Get (0), container.Get (1));
    else
      NS_LOG_UNCOND("Unable to add new Connections");
  }
}

void NetworkGenerator::randomlyAddConnectionsBetweenTwoNodesPerAS(int numberOfConnectionsPerAs, int minBW_kbits, int maxBw_kbits, int minDelay_ms, int maxDelay_ms)
{
  PointToPointHelper p2p;

  for(int as = 0; as < getNumberOfAS (); as++)
  {
    for(int i = 0; i < numberOfConnectionsPerAs; i++)
    {
      std::string delay(boost::lexical_cast<std::string>(rvariable->GetValue (minDelay_ms,maxDelay_ms)));
      delay.append ("ms");

      std::string bw(boost::lexical_cast<std::string>(rvariable->GetValue (minBW_kbits,maxBw_kbits)));
      bw.append ("Kbps");

      p2p.SetDeviceAttribute ("DataRate", StringValue (bw));
      p2p.SetChannelAttribute ("Delay", StringValue (delay));

      NodeContainer container = getPairOfUnconnectedNodes(as, as);
      if(container.size () == 2)
        p2p.Install (container.Get (0), container.Get (1));
      else
        NS_LOG_UNCOND("Unable to add new Connections");
    }
  }
}

int NetworkGenerator::getNumberOfAS ()
{
  return briteHelper->GetNAs ();
}

int NetworkGenerator::getNumberOfNodesInAS (int ASnumber)
{
  if(getNumberOfAS () < ASnumber)
  {
    return briteHelper->GetNNodesForAs (ASnumber);
  }
  return 0;
}

NodeContainer NetworkGenerator::getAllASNodes()
{
  NodeContainer container;

  for(int as=0; as < getNumberOfAS (); as++)
  {
    container.Add (getAllASNodesFromAS(as));
  }
  return container;
}

NodeContainer NetworkGenerator::getAllASNodesFromAS(int ASnumber)
{
  NodeContainer container;

  if(getNumberOfAS () < ASnumber)
    return container;

  for(int node=0; node < briteHelper->GetNNodesForAs(ASnumber); node++)
  {
    container.Add (briteHelper->GetNodeForAs (ASnumber,node));
  }

  return container;
}

NodeContainer NetworkGenerator::getAllLeafNodes()
{
  NodeContainer container;

  for(int as=0; as < getNumberOfAS (); as++)
  {
    container.Add (getAllLeafNodesFromAS(as));
  }

  return container;
}

NodeContainer NetworkGenerator::getAllLeafNodesFromAS(int ASnumber)
{
  NodeContainer container;

  if(getNumberOfAS () < ASnumber)
    return container;

  for(int node=0; node < briteHelper->GetNLeafNodesForAs (ASnumber); node++)
  {
    container.Add (briteHelper->GetLeafNodeForAs(ASnumber,node));
  }
  return container;
}

NodeContainer NetworkGenerator::getCustomNodes(std::string setIdentifier)
{
  return nodeContainerMap[setIdentifier];
}

void NetworkGenerator::creatRandomLinkFailure(double minTimestamp, double maxTimestamp, double minDuration, double maxDuration)
{
  int rand = rvariable->GetInteger(0,getNumberOfAS() - 1);

  NodeContainer c = getAllASNodesFromAS(rand);
  rand = rvariable->GetInteger (0, c.size ()-1);

  Ptr<Node> node = c.Get (rand);

  rand = rvariable->GetInteger (0,node->GetNDevices ()-1);
  Ptr<Channel> channel = node->GetDevice (rand)->GetChannel ();

  NodeContainer channelNodes;

  for(int i = 0; i < channel->GetNDevices (); i++)
  {
    Ptr<NetDevice> dev = channel->GetDevice (i);
    channelNodes.Add (dev->GetNode ());
  }

  if(channelNodes.size () != 2)
    NS_LOG_ERROR("Invalid Channel with more than 2 nodes...");
  else
  {
    double startTime = rvariable->GetValue (minTimestamp, maxTimestamp);
    double stopTime = startTime + rvariable->GetValue (minDuration, maxDuration);

    Simulator::Schedule (Seconds (startTime), ns3::ndn::LinkControlHelper::FailLink, channelNodes.Get (0), channelNodes.Get (1));
    Simulator::Schedule (Seconds (stopTime), ns3::ndn::LinkControlHelper::UpLink,   channelNodes.Get (0), channelNodes.Get (1));

    //fprintf(stderr, "Start LinkFail between %s and %s: %f\n",Names::FindName (channelNodes.Get (0)).c_str (),Names::FindName (channelNodes.Get (1)).c_str (), startTime);
    //fprintf(stderr, "Stop LinkFail between %s and %s: %f\n\n",Names::FindName (channelNodes.Get (0)).c_str (),Names::FindName (channelNodes.Get (1)).c_str (),stopTime);
  }
}

bool NetworkGenerator::nodesConnected(Ptr<Node> n1, Ptr<Node> n2)
{
  int n1_nr_dev = n1->GetNDevices ();

  for(int i = 0; i < n1_nr_dev; i++)
  {
    Ptr<NetDevice> dev = n1->GetDevice (i);
    Ptr<Channel> channel = dev->GetChannel ();
    int channel_nr_dev = channel->GetNDevices ();

    for(int j = 0; j < channel_nr_dev; j++)
    {
      Ptr<Node> con_node = channel->GetDevice (j)->GetNode ();
      if(n2->GetId () == con_node->GetId ())
        return true;
    }
  }
  return false;
}

NodeContainer NetworkGenerator::getPairOfUnconnectedNodes(int as1, int as2)
{
  NodeContainer as1_nodes = getAllASNodesFromAS (as1);
  NodeContainer as2_nodes = getAllASNodesFromAS (as2);

  while(as1_nodes.size () > 0)
  {
    int rand_node_1 = rvariable->GetInteger (0,as1_nodes.size ()-1);
    Ptr<Node> as1_node = as1_nodes.Get (rand_node_1);
    removeNode (as1_nodes, as1_node);

    NodeContainer as2_nodes_cp = as2_nodes;
    while(as2_nodes_cp.size () > 0)
    {
      int rand_node_2 = rvariable->GetInteger (0,as2_nodes_cp.size ()-1);
      Ptr<Node> as2_node = as2_nodes_cp.Get (rand_node_2);
      removeNode (as2_nodes, as2_node);

      if(as1_node->GetId () != as2_node->GetId () &&
         !nodesConnected(as1_node, as2_node))
      {
        NodeContainer c;
        c.Add (as1_node);
        c.Add (as2_node);
        return c;
      }
    }
  }
  return NodeContainer();
}

NodeContainer NetworkGenerator::removeNode(NodeContainer container, Ptr<Node> node)
{
  NodeContainer result;
  for(NodeContainer::iterator it = container.begin (); it!=container.end (); ++it)
  {
    if( (*it)->GetId() != node->GetId ())
      result.Add (*it);
  }
  return result;
}

double NetworkGenerator::calculateConnectivity ()
{
  NodeContainer allNodes;
  allNodes.Add (getAllASNodes ());

  Ptr<Node> n;
  double connectivity = 0.0;

  for(int i = 0; i < allNodes.size (); i++)
  {
    n = allNodes.Get (i);
    connectivity += n->GetNDevices (); // degree summation
  }

  connectivity /= allNodes.size ();
  connectivity /= (allNodes.size () - 1);

  return connectivity;
}

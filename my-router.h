// CS 118 - Project 2
// Class description for Router class

#include <iostream>
#include <string>
#include <vector>
#include <map>

using namespace std;

// Data structures

// Individual entry in a distance vector
struct DistanceVectorEntry {
    string nodeName;
    int cost;
    int port;

    DistanceVectorEntry() {}
    DistanceVectorEntry(string n, int c, int p) {
        nodeName = n;
        cost = c;
        port = p;
    }
};

// Individual entry in the routing table
struct RoutingTableEntry {
    int cost;
    int outgoingPort;
    int incomingPort;
    string nextHop;

    RoutingTableEntry() {}
    RoutingTableEntry(int c, int oP, int iP, string nH) {
        cost = c;
        outgoingPort = oP;
        incomingPort = iP;
        nextHop = nH;
    }
};

// Router class
class Router {
public:
    Router(string name, int port);
    ~Router();

    void initializeRoutingTable(const char* fileName);                      // Initialize the routing table from the network topology file
    void updateRoutingTable(string node, vector<DistanceVectorEntry> dv);   // Update the routing table according to a distance vector
    void printRoutingTable();                                               // Print the routing table to file
    map<string, RoutingTableEntry> getRoutingTable();                       // Get the routing table
    string getName();                                                       // Get node name

private:
    string name;
    int port;
    map<string, RoutingTableEntry> routingTable;
};

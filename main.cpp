// CS 118 - Project 2
// Main file

#include <iostream>
#include <string>
#include <vector>
#include <map>
#include <cstdio>
#include <fstream>
#include "my-router.h"

int main() {
    // Start router "A"
    Router r1("A", 10000);
    r1.initializeRoutingTable("topology.txt");
    r1.printRoutingTable();

    // Start router "B"
    Router r2("B", 10001);
    r2.initializeRoutingTable("topology.txt");
    r2.printRoutingTable();

    // Send distance vector from B to A
    map<string, RoutingTableEntry> bTable = r2.getRoutingTable();
    map<string, RoutingTableEntry>::iterator it;
    vector<DistanceVectorEntry> dv;
    for (it = bTable.begin(); it != bTable.end(); it++) {
        DistanceVectorEntry entry(it->first, (it->second).cost, (it->second).incomingPort);
        dv.push_back(entry);
    }
    r1.updateRoutingTable(r2.getName(), dv);
    r1.printRoutingTable();

    return 0;
}

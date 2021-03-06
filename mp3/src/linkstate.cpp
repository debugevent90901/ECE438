#include <iostream>
#include <fstream>
#include <string>
#include <sstream>
#include <list>
#include <map>
#include <set>
#include <queue>
#include <climits>

#define BROKEN      -999        // cost -999 indicates a broken link
#define INFINITY    INT_MAX    
#define UNDEFINED   -INT_MAX

using namespace std;

void parse_input(char *topofile, char *messagefile, char *changefile);
void dijkstra(int source);
void write_topology_entry(int nodeID);
void write_msg();
void update(string& s);
void run();

set<int> all_nodes;
set<int> nodes;                                 // nodeID
map<int, map<int, int>> costs;                  // c(x, y)
map<int, map<int, int>> shortest_path_costs;    // D(v) for node x
map<int, map<int, list<int>>> hop_sequences;    // x to y, list<int> of hops with min cost

list<string> msgs;
list<string> changes;

set<int> removed_nodes;

void parse_input(char *topofile, char *messagefile, char *changefile)
{
    ifstream infile;
    string line;
    // read messagefile
    infile.open(messagefile);
    while (getline(infile, line))
        msgs.push_back(line);
    infile.close();
    // read changefile
    infile.open(changefile);
    while (getline(infile, line))
        changes.push_back(line);
    infile.close();

    // read topofile and init nodeInfo list
    infile.open(topofile);
    while (getline(infile, line))
    {
        stringstream topo(line);
        int node1, node2, cost;
        topo >> node1;
        topo >> node2;
        topo >> cost;
        nodes.insert(node1);
        nodes.insert(node2);
        costs[node1][node2] = cost;
        costs[node2][node1] = cost;
    }
    infile.close();
    all_nodes = nodes;

    return;
}


void dijkstra(int source)
{   
    // init
    set<int>        Q;
    map<int, int>   dist, prev;
    queue<int>      path;

    for (int node: nodes)
    {
        dist[node] = INFINITY;
        prev[node] = UNDEFINED;
        Q.insert(node);
    }
    dist[source] = 0;

    while (!Q.empty())
    {
        int min_dist=INFINITY, min_node=INFINITY;
        for (int q: Q)
        {
            if (dist[q] < min_dist)
            {
                min_dist = dist[q];
                min_node = q;
            }
            else if (dist[q] == min_dist)
            {
                if (min_node > q)
                    min_node = q;
            }
        }

        Q.erase(min_node);
        path.push(min_node);

        for (int q: Q)
        {
            if (costs[min_node].count(q) == 1)
            {
                int alt = dist[min_node] + costs[min_node][q];
                if (alt < dist[q])
                {
                    dist[q] = alt;
                    prev[q] = min_node;
                }
                else if (alt == dist[q])
                {
                    if (min_node < prev[q])
                        prev[q] = min_node;
                }
            }
        }
    }

    for (auto& i: dist)
        shortest_path_costs[source][i.first] = i.second;

    // backtracking
    while (!path.empty())
    {
        int dest = path.front();
        int u = dest;
        if (u==source)
        {
            hop_sequences[source][dest].push_front(source);
            path.pop();
            continue;
        }
        else if (prev[u]!=UNDEFINED)
        {
            while (u!=UNDEFINED)
            {
                hop_sequences[source][dest].push_front(u);
                u = prev[u];
            }
            hop_sequences[source][dest].pop_front();
            path.pop();
        }
    }

    return;
}


void write_topology_entry(int nodeID)
{
    ofstream outfile;
    outfile.open("output.txt", ios::app);
    
    for (auto& i: hop_sequences[nodeID])
    {
        int dest = i.first;
        int next_hop = i.second.front();
        int cost = shortest_path_costs[nodeID][dest];
        outfile << dest << " " << next_hop << " " << cost << endl;
    }
    // outfile << endl;

    outfile.close();
    return;
}


void write_msg()
{
    ofstream outfile;
    outfile.open("output.txt", ios::app);

    for (string& s: msgs)
    {
        stringstream __temp(s);
        int src, dest;
        __temp >> src; __temp >> dest;
        string msg(s, s.find(to_string(dest))+2);

        if (shortest_path_costs[src][dest] == INT_MAX || hop_sequences[src][dest].size() == 0)
            outfile << "from " << src << " to " << dest << " cost infinite hops unreachable message " << msg << endl;
        else
        {
            list<int> __sequence = hop_sequences[src][dest];
            outfile << "from " << src << " to " << dest << " cost " << shortest_path_costs[src][dest] << " hops " << src << " "; 
            __sequence.pop_back();
            for (int i: __sequence)
                outfile << i << " ";
            outfile << "message " << msg << endl;
        }
    }   
    // outfile << endl;
    // outfile << endl;
    // outfile << endl;

    outfile.close();
    return;
}


void update(string& s)
{
    stringstream change(s);
    int node1, node2, cost;
    change >> node1; change >> node2; change >> cost;

    if (cost == BROKEN)
    {
        costs[node1].erase(node2);
        costs[node2].erase(node1);

        set<int> new_nodes;
        for (auto& i: costs)
        {
            for (auto& j: i.second)
            {
                new_nodes.insert(i.first);
                new_nodes.insert(j.first);
            }
        }
        for (int i: nodes)
            if (new_nodes.count(i) == 0)
                removed_nodes.insert(i);

        nodes = new_nodes;
    }
    else
    {
        nodes.insert(node1);
        nodes.insert(node2);
        costs[node1][node2] = cost;
        costs[node2][node1] = cost;       
        if (removed_nodes.count(node1) == 1)
            removed_nodes.erase(node1);
        if (removed_nodes.count(node2) == 1)
            removed_nodes.erase(node2);
    }

    return;
}

void run()
{
    shortest_path_costs.clear();
    hop_sequences.clear();

    for (int i: all_nodes)
    {
        if (nodes.count(i) == 1)
        {   
            dijkstra(i);
            write_topology_entry(i);
        }
        else
        {
            ofstream outfile;
            outfile.open("output.txt", ios::app);
            outfile << i << " " << i << " " << 0 << endl;
            outfile.close();
        }
    }
    write_msg();

    return;
}

int main(int argc, char **argv)
{
    //printf("Number of arguments: %d", argc);
    if (argc != 4)
    {
        printf("Usage: ./linkstate topofile messagefile changesfile\n");
        return -1;
    }

    FILE *fpOut;
    fpOut = fopen("output.txt", "w");
    fclose(fpOut);

    parse_input(argv[1], argv[2], argv[3]);

    run();
    for(string &s: changes)
    {
        update(s);
        run();
    }
    return 0;
}
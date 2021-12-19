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
void dijkstra(int nodeID);
void write_topology_entry(int nodeID);
void write_msg();
void update(string& s);
void run();

set<int> nodes;                                 // nodeID
map<int, map<int, int>> costs;                  // c(x, y)
map<int, map<int, int>> shortest_path_costs;    // D(v) for node x
map<int, map<int, list<int>>> hop_sequences;    // x to y, list<int> of hops with min cost

list<string> msgs;
list<string> changes;

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

    return;
}


void dijkstra(int nodeID)
{   
    // init
    set<int> N_prime;
    N_prime.insert(nodeID);

    queue<int> __node_path;

    map<int, int> p, D;
    for (int node: nodes) 
    {
        if (costs[nodeID].count(node) == 1)
        {
            D[node] = costs[nodeID][node];
            p[node] = nodeID;
        }
        else if (node == nodeID)
        {
            D[node] = 0;
            p[node] = INT_MAX;
        }
        else
        {
            D[node] = INT_MAX;
            p[node] = INT_MAX;
        }
    }
    // start to loop
    while (N_prime.size() < nodes.size())
    {
        int __min_dist=INT_MAX, __min_node=INT_MAX;
        for (int w: nodes) 
        {
            if (N_prime.count(w) == 0)
            {
                if (D[w] < __min_dist)
                {
                    __min_dist = D[w];
                    __min_node = w;
                }
                else if (D[w] == __min_dist)
                {
                    if (__min_node > w)
                        __min_node = w;
                }
            }
        }

        __node_path.push(__min_node);
        N_prime.insert(__min_node);
        for (int v: nodes) 
        {
            if (costs[__min_node].count(v) == 1)
            {
                if (N_prime.count(v) == 0)
                {
                    if (D[v] > D[__min_node]+costs[__min_node][v])
                    {
                        D[v] = D[__min_node]+costs[__min_node][v];
                        p[v] = __min_node;
                    }
                    else if (D[v] == D[__min_node]+costs[__min_node][v])
                    {
                        if (__min_node < p[v])
                            p[v] = __min_node;
                    }
                }
            }
        }
    }

    for (auto &t: D)
        shortest_path_costs[nodeID][t.first] = t.second;
    
    hop_sequences[nodeID][nodeID].push_front(nodeID);
    while (!__node_path.empty())
    {
        int __node = __node_path.front();
        hop_sequences[nodeID][__node].push_front(__node);
        int __temp = p[__node];
        while (__temp != nodeID)
        {
            cout << __temp << endl;
            hop_sequences[nodeID][__node].push_front(__temp);
            __temp = p[__temp];
            if (__temp == INT_MAX || __temp == 0)
            {
                cout << "reach here" << endl;
                hop_sequences[nodeID][__node].clear();
                break;
            }
        }
        __node_path.pop();
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
        // if (next_hop != INT_MAX && cost != INT_MAX)
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
            outfile << "from " << src << " to " << dest << " cost infinite hops unreachable message" << msg << endl;
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
        if (costs.count(node1) <= 0)
            nodes.erase(node1);
        costs[node2].erase(node1);
        if (costs.count(node2) <= 0)
            nodes.erase(node2);
    }
    else
    {
        costs[node1][node2] = cost;
        costs[node2][node1] = cost;       
    }

    return;
}

void run()
{
    shortest_path_costs.clear();
    hop_sequences.clear();
    for (int i: nodes)
    {
        dijkstra(i);
        write_topology_entry(i);
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
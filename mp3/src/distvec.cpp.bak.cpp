#include<stdio.h>
#include<string.h>
#include<stdlib.h>

#include <iostream>
#include <fstream>
#include <string>
#include <sstream>
#include <list>
#include <map>
#include <set>
#include <queue>
#include <climits>

#define BROKEN -999     // cost -999 indicates a broken link

using namespace std;

void parse_input(char *topofile, char *messagefile, char *changefile);
void write_topology_entry(int nodeID);
void write_msg();
void update(string& s);
void init_dv_algorithm();
void run_dv_algorithm();
void run();

set<int> nodes;                                  
map<int, map<int, int>> costs;                  
map<int, map<int, int>> shortest_path_costs;    
map<int, map<int, list<int>>> hop_sequences;  

list<string> msgs;
list<string> changes;


void init_dv_algorithm()
{
    for (int i: nodes)
    {
        for (int j: nodes)
        {
            if (costs[i].count(j) == 1)
            {
                hop_sequences[i][j].push_back(j);
                shortest_path_costs[i][j] = costs[i][j];
            }
            else
            {
                hop_sequences[i][j].push_back(INT_MAX);
                shortest_path_costs[i][j] = INT_MAX;
            }
            if (i == j)
            {
                hop_sequences[i][j].push_back(i);
                shortest_path_costs[i][j] = 0;
            }
        }
    }

    return;
}


void run_dv_algorithm()
{
    bool is_converged = true;
    // iteration
    do
    {
        bool is_round_converged = true;
        // for each node
        for (int i: nodes)
        {
            // for every dest
            for (int j: nodes)
            {
                int __curr_next_hop = hop_sequences[i][j].front();
                int __curr_min_cost = shortest_path_costs[i][j];
                int __next_hop = __curr_next_hop, __min_cost = __curr_min_cost;
                // for every neighbor
                for (int k: nodes)
                {
                    if (costs[i].count(k) == 1 && shortest_path_costs[k][j] >= 0 && shortest_path_costs[k][j] != INT_MAX)
                    {
                        int __tmp = costs[i][k] + shortest_path_costs[k][j];
                        if (__tmp < __min_cost)
                        {
                            __next_hop = k;
                            __min_cost = __tmp;
                        }
                    }
                }
                hop_sequences[i][j].clear();
                hop_sequences[i][j].push_back(__next_hop);
                shortest_path_costs[i][j] = __min_cost;
                
                if (i == j)
                {
                    hop_sequences[i][j].clear();
                    hop_sequences[i][j].push_back(i);
                    shortest_path_costs[i][j] = 0;
                }

                if (__next_hop == __curr_next_hop || __min_cost == __curr_min_cost)
                    is_round_converged &= true;
                else   
                    is_round_converged &= false;
            }
        }
        is_converged = is_round_converged;
    } while (!is_converged);

    for (int i: nodes)
    {
        for (int j: nodes)
        {
            // cout << hop_sequences[i][j].size() << endl;
            while (shortest_path_costs[i][j] != INT_MAX && hop_sequences[i][j].back() != j)
                hop_sequences[i][j].push_back(hop_sequences[hop_sequences[i][j].back()][j].front());
        }
    }

    return;
}


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


void write_topology_entry(int nodeID)
{
    ofstream outfile;
    outfile.open("output.txt", ios::app);
    
    for (auto& i: hop_sequences[nodeID])
    {
        int dest = i.first;
        int next_hop = i.second.front();
        int cost = shortest_path_costs[nodeID][dest];
        // if (next_hop == INT_MAX || cost == INT_MAX)
            // outfile << dest << " " << next_hop << " " << cost << endl;
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

        if (shortest_path_costs[src][dest] == INT_MAX)
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
    init_dv_algorithm();
    run_dv_algorithm();
    for (int i: nodes)
        write_topology_entry(i);
    write_msg();
    return;
}


int main(int argc, char** argv) {
    //printf("Number of arguments: %d", argc);
    if (argc != 4) {
        printf("Usage: ./distvec topofile messagefile changesfile\n");
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


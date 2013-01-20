/** Copyright (C) 2013 wilburlang@gmail.com
 */
#include <string>
#include <iostream>
#include <map>
#include <set>
#include "common/alg/digraph.h"
#include "common/datetime.h"

void test1()
{
    cxx::alg::digraph<std::string >  graph;
    graph.add_edge("atm",   "aal5");
    graph.add_edge("atm",   "aal2");
    graph.add_edge("aal5",  "sscop");
    graph.add_edge("sscop", "nbap");
    graph.add_edge("aal5",  "alcap");
    graph.add_edge("aal2",  "fp");
    graph.add_edge("fp",    "mac");
    graph.add_edge("mac",   "rlc");
    graph.add_edge("rlc",   "rrc");

    cxx::alg::digraph<std::string>::AdjancencyList link = graph.get_edge("atm");
    cxx::alg::digraph<std::string>::AdjancencyList::iterator it = link.begin();
    for(; it != link.end(); ++it) {
        std::cout << (*it) << " ";
    }
    std::cout << "\r\n";
}

template<typename T >
void get_child(const cxx::alg::digraph<T >& graph, const T& from, std::vector<T >& result)
{
    typename cxx::alg::digraph<T >::AdjancencyList r = graph.get_edge(from);
    for(int i = 0; i < r.size(); ++i) {
        result.push_back(r[i]);
        get_child(graph, r[i], result);
    }
}

template<typename T >
void get_parent(const cxx::alg::digraph<T >& graph, const T& from, std::vector<T >& result)
{
    typename cxx::alg::digraph<T >::AdjancencyList r = graph.get_join(from);
    for(int i = 0; i < r.size(); ++i) {
        result.push_back(r[i]);
        get_parent(graph, r[i], result);
    }
}

void test2()
{
    cxx::alg::digraph<std::string >  graph;
    graph.add_edge("ip",    "sctp");
    graph.add_edge("ip",    "tcp");
    graph.add_edge("ip",    "udp");
    graph.add_edge("sctp",  "m3ua");
    graph.add_edge("sctp",  "rsl");
    graph.add_edge("lapd",  "rsl");
    graph.add_edge("e1",    "lapd");
    graph.add_edge("e1",    "ml-ppp");
    graph.add_edge("ml-ppp","ip");
    graph.add_edge("mac",   "ip");

    {
        cxx::alg::digraph<std::string>::AdjancencyList link = graph.get_edge("ip");
        cxx::alg::digraph<std::string>::AdjancencyList::iterator it = link.begin();
        for(; it != link.end(); ++it) {
            std::cout << (*it) << " ";
        }
        std::cout << "\r\n";
    }
    {
        cxx::alg::digraph<std::string>::AdjancencyList link = graph.get_edge("x");
        cxx::alg::digraph<std::string>::AdjancencyList::iterator it = link.begin();
        for(; it != link.end(); ++it) {
            std::cout << (*it) << " ";
        }
        std::cout << "\r\n";
    }

    {
        cxx::alg::digraph<std::string>::AdjancencyList link = graph.get_join("ip");
        cxx::alg::digraph<std::string>::AdjancencyList::iterator it = link.begin();
        for(; it != link.end(); ++it) {
            std::cout << (*it) << " ";
        }
        std::cout << "\r\n";
    }
    {
        cxx::alg::digraph<std::string>::AdjancencyList link = graph.get_join("rsl");
        cxx::alg::digraph<std::string>::AdjancencyList::iterator it = link.begin();
        for(; it != link.end(); ++it) {
            std::cout << (*it) << " ";
        }
        std::cout << "\r\n";
    }
    {
        std::vector<std::string > res;
        get_child<std::string >(graph, "ip", res);
        for(size_t i = 0; i < res.size(); ++i) {
            std::cout << res[i] << " ";
        }
        std::cout << "\r\n";
    }
    {
        std::vector<std::string > res;
        get_child<std::string >(graph, "e1", res);
        for(size_t i = 0; i < res.size(); ++i) {
            std::cout << res[i] << " ";
        }
        std::cout << "\r\n";
    }
    {
        std::vector<std::string > res;
        get_parent<std::string >(graph, "rsl", res);
        for(size_t i = 0; i < res.size(); ++i) {
            std::cout << res[i] << " ";
        }
        std::cout << "\r\n";
    }
    {
        std::vector<std::string > res;
        get_parent<std::string >(graph, "e1", res);
        for(size_t i = 0; i < res.size(); ++i) {
            std::cout << res[i] << " ";
        }
        std::cout << "\r\n";
    }

    cxx::alg::digraph<std::string >::AdjancencyList sort = graph.top_sort();
    std::cout << "sorted liner graph: \n  ";
    for(size_t i = 0; i < sort.size(); ++i) {
        std::cout << sort[i] << " ";
    }
    std::cout << "\n  graph is " << (graph.is_cycle() ? "cycle" : "acyclic") << " directed graph\n";
}

void test3()
{
    cxx::alg::digraph<std::string >  graph;
    graph.add_edge("a",     "b");
    graph.add_edge("a",     "c");
    graph.add_edge("b",     "d");
    graph.add_edge("c",     "d");

    cxx::alg::digraph<std::string >::AdjancencyList sort = graph.top_sort();
    std::cout << "sorted liner graph: \n  ";
    for(size_t i = 0; i < sort.size(); ++i) {
        std::cout << sort[i] << " ";
    }
    std::cout << "\n  graph is " << (graph.is_cycle() ? "cycle" : "acyclic") << " directed graph\n";
}

void test4()
{
    cxx::alg::digraph<std::string >  graph;
    graph.add_edge("a",     "b");
    graph.add_edge("a",     "c");
    graph.add_edge("b",     "d");
    graph.add_edge("c",     "d");
    graph.add_edge("d",     "a");

    cxx::alg::digraph<std::string >::AdjancencyList sort = graph.top_sort();
    std::cout << "sorted liner graph: \n  ";
    for(size_t i = 0; i < sort.size(); ++i) {
        std::cout << sort[i] << " ";
    }
    std::cout << "\n  graph is " << (graph.is_cycle() ? "cycle" : "acyclic") << " directed graph\n";
}

void test5()
{
    cxx::alg::digraph<std::string >  graph;
    graph.add_edge("a",     "b");
    graph.add_edge("a",     "c");
    graph.add_edge("b",     "d");
    graph.add_edge("c",     "d");
    graph.add_edge("d",     "m");
    graph.add_edge("m",     "c");

    cxx::alg::digraph<std::string >::AdjancencyList sort = graph.top_sort();
    std::cout << "sorted liner graph: \n  ";
    for(size_t i = 0; i < sort.size(); ++i) {
        std::cout << sort[i] << " ";
    }
    std::cout << "\n  graph is " << (graph.is_cycle() ? "cycle" : "acyclic") << " directed graph\n";
}

//template<typename T >
//void DFS(const cxx::digraph<T >& g, const T& v, typename cxx::digraph<T >::AdjancencyList& results)
//{
//    results.push_back(v);

//    const typename cxx::digraph<T >::AdjancencyList& adj = g.get_edge(v);
//    for(size_t i = 0; i < adj.size(); ++i) {
//        const T& u = adj[i];
//        typename std::map<T, int >::iterator it = visited.find(u);
//        DFS(g, u, results);
//    }
//}

template<typename T >
void DFS(const cxx::alg::digraph<T >& g, const T& v, int& i, std::map<T, int >& visited, std::vector<std::pair<T, T > >& results)
{
    visited[v] = i++;
    const typename cxx::alg::digraph<T >::AdjancencyList& vertices = g.get_edge(v);
    for(size_t k = 0; k < vertices.size(); ++k) {
        const T& u = vertices[k];
        if(visited[u] == 0) {
            results.push_back(std::make_pair(v, u));
            DFS(g, u, i, visited, results);
        }
    }
}

/** DFS 遍历无环有向图并不能保证所有的边（edge）都在生成树（spanning tree）中。在生成树中
 * 的边叫 forward edges（或者 tree edges），没有包括在生成树中叫 back edges。
 */
template<typename T >
std::vector<std::pair<T, T > > DFS_search(const cxx::alg::digraph<T >& g)
{
    std::vector<std::pair<T, T > > results;

    std::map<T, int > visited;
    const typename cxx::alg::digraph<T >::AdjancencyList& vertices = g.top_sort();

    for(size_t i = 0; i < vertices.size(); ++i) {
        visited[vertices[i]] =  0;
    }

    int n = 1;
    for(size_t i = 0; i < vertices.size(); ++i) {
        if(visited[vertices[i]] == 0) {
            DFS(g, vertices[i], n, visited, results);
        }
    }
    return results;
}

void test6()
{
    cxx::alg::digraph<std::string >  graph;
    graph.add_edge("virtual",   "cpt");
    graph.add_edge("virtual",   "usd");
    graph.add_edge("virtual",   "pcap");
    graph.add_edge("cpt",       "ethernet");
    graph.add_edge("ethernet",  "ip");
    graph.add_edge("ip",        "tcp");
    graph.add_edge("ip",        "udp");
    graph.add_edge("ip",        "sctp");
    graph.add_edge("sctp",      "sctpdata");
    graph.add_edge("sctpdata",  "m2ua");
    graph.add_edge("m2ua",      "mtp3");
    graph.add_edge("mtp3",      "sccp");
    graph.add_edge("sccp",      "bssap");
    graph.add_edge("bssap",     "bssmap");
    graph.add_edge("dtap",      "cc");
    graph.add_edge("dtap",      "mm");
    graph.add_edge("dtap",      "rr");
    graph.add_edge("dtap",      "gsm");
    graph.add_edge("dtap",      "sms");
    graph.add_edge("bssmap",    "rr");
    graph.add_edge("bssmap",    "sms");
    graph.add_edge("sctpdata",  "m3ua");
    graph.add_edge("m3ua",      "sccp");
    graph.add_edge("sctpdata",  "zte-sdr");
    graph.add_edge("cpt",       "lapd");
    graph.add_edge("lapd",      "rsl");
    graph.add_edge("rsl",       "dtap");
    graph.add_edge("zte-sdr",   "rsl");
    graph.add_edge("usd",       "lapd");
    graph.add_edge("pcap",      "lapd");
    graph.add_edge("usd",       "ethernet");
    graph.add_edge("pcap",      "ethernet");

    cxx::alg::digraph<std::string >::AdjancencyList sort = graph.top_sort();
    std::cout << "sorted protostack graph: \n  ";
    for(size_t i = 0; i < sort.size(); ++i) {
        std::cout << sort[i] << " ";
    }
    std::cout << "\n  graph is " << (graph.is_cycle() ? "cycle" : "acyclic") << " directed graph\n";

    cxx::alg::digraph<std::string >::AdjancencyList dfss;
//    DFS<std::string >(graph, "virtual", dfss);
    std::cout << "path\n" << "  ";
    for(size_t i = 0; i < dfss.size(); ++i) {
        std::cout << dfss[i] << " ";
    }
    std::cout << "\n";

    int count = 1000000;
    cxx::datetime begin(cxx::datetime::now());
    for(int i = 0; i < count; ++i) {
        graph.get_edge("m3ua");
    }
    cxx::datetime end(cxx::datetime::now());
    std::cout << "performance: " << count << "/" << (end - begin).getTotalMilliSeconds() << ", "
              << count * 1000 / (end - begin).getTotalMilliSeconds() << "/s\n";

    std::vector<std::pair<std::string, std::string > > span = DFS_search(graph);
    for(size_t i = 0; i < span.size(); ++i) {
        std::cout << "  " << span[i].first << "->" << span[i].second << "\n";
    }
}

int main(int argc, char* argv[])
{
    test1();
    test2();
    test3();
    test4();
    test5();
    test6();

    return 0;

}

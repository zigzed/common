/** Copyright (C) 2013 wilburlang@gmail.com
 */
#ifndef CXX_DIGRAPH_H
#define CXX_DIGRAPH_H
#include <stddef.h>
#include <vector>
#include <algorithm>

#define CXX_DIGRAPH_USE_MAP
#ifdef __GNUC__
    #define GCC_VERSION     (__GNUC__ * 10000 + __GNUC_MINOR__ * 100 + __GNUC_PATCHLEVEL__)
    #if GCC_VERSION >= 40300
        #undef  CXX_DIGRAPH_USE_MAP
        #include <tr1/unordered_map>
    #else
        #include <map>
    #endif
#endif
#ifdef  _MSC_VER
    // VS2005   VC8     1400
    // VS2008   VC9     1500
    // VS2010   VC10    1600
    // VS2012   VC11    1700
    #if _MSC_VER >= 1400
        #undef  CXX_DIGRAPH_USE_MAP
        #include <unordered_map>
    #else
        #include <map>
    #endif
#endif

namespace cxx {
    namespace alg {

        /// a very simple directed graph with adjancency link list
        template<typename T >
        class digraph {
        public:
            typedef typename std::vector<T >        AdjancencyList;
            digraph();
            ~digraph();

            bool                empty() const;
            /// counts of the directed graph node
            size_t              nodes() const;
            /// counts of the directed graph edge
            size_t              edges() const;
            void                clear();

            bool                add_node(const T& node);
            bool                add_edge(const T& from, const T& to);
            bool                del_node(const T& node);
            bool                del_edge(const T& from, const T& to);
            /// get all edge connected from node
            const AdjancencyList&   get_edge(const T& node) const;
            /// get all edge connecte to node
            AdjancencyList          get_join(const T& node) const;
            /// topological sort
            AdjancencyList          top_sort() const;
            /// check the directed graph is cycle or acyclic
            bool                    is_cycle() const;
            // todo: need a node iterator
        private:
            /// 对于 get_edge()，unordered_map比map快4倍左右。
            #ifdef  CXX_DIGRAPH_USE_MAP
                typedef typename std::map<T, AdjancencyList >    Nodes;
            #else
                typedef typename std::tr1::unordered_map<T, AdjancencyList > Nodes;
            #endif

            Nodes   nodes_;
        };

        template<typename T >
        digraph<T >::digraph()
        {
        }

        template<typename T >
        digraph<T >::~digraph()
        {
        }

        template<typename T >
        bool digraph<T >::empty() const
        {
            return nodes_.empty();
        }

        template<typename T >
        size_t digraph<T >::nodes() const
        {
            return nodes_.size();
        }

        template<typename T >
        size_t digraph<T >::edges() const
        {
            size_t count = 0;
            for(typename Nodes::const_iterator it = nodes_.begin(); it != nodes_.end(); ++it) {
                count += it->second.size();
            }
            return count;
        }

        template<typename T >
        bool digraph<T >::add_node(const T& node)
        {
            typename Nodes::iterator it = nodes_.find(node);
            if(it != nodes_.end()) {
                return false;
            }
            AdjancencyList  link;
            nodes_[node] = link;
            return true;
        }

        template<typename T >
        bool digraph<T >::add_edge(const T& from, const T& to)
        {
            add_node(from);
            add_node(to);
            AdjancencyList& link = nodes_.find(from)->second;
            const T&        node = nodes_.find(to)->first;
            link.push_back(node);
            return true;
        }

        template<typename T >
        bool digraph<T >::del_node(const T& node)
        {
            typename Nodes::iterator it = nodes_.find(node);
            if(it == nodes_.end()) {
                return false;
            }
            const T&        temp = it->first;
            // first remote every node from edge
            typename Nodes::iterator node_it = nodes_.begin();
            for(; node_it != nodes_.end(); ++node_it) {
                AdjancencyList& link = node_it->second;
                // delete every node from every edge
                link.erase(std::remove(link.begin(), link.end(), temp), link.end());
            }

            // then remote the node from node
            nodes_.erase(node);
            return true;
        }

        template<typename T >
        bool digraph<T >::del_edge(const T& from, const T& to)
        {
            typename Nodes::iterator from_it = nodes_.find(from);
            if(from_it == nodes_.end()) {
                return false;
            }
            typename Nodes::iterator to_it = nodes_.find(to);
            if(to_it == nodes_.end()) {
                return false;
            }

            AdjancencyList& link = from_it->second;
            link.erase(std::remove(link.begin(), link.end(), to_it->first), link.end());
            return true;
        }

        template<typename T >
        const typename digraph<T >::AdjancencyList& digraph<T >::get_edge(const T& from) const
        {
            static const AdjancencyList dummy_empty;
            typename Nodes::const_iterator it = nodes_.find(from);
            if(it != nodes_.end()) {
                return it->second;
            }
            return dummy_empty;
        }

        template<typename T >
        typename digraph<T >::AdjancencyList digraph<T >::get_join(const T& to) const
        {
            AdjancencyList  result;
            typename Nodes::const_iterator kt = nodes_.find(to);
            if(kt != nodes_.end()) {
                const T& target = kt->first;

                for(typename Nodes::const_iterator it = nodes_.begin(); it != nodes_.end(); ++it) {
                    const AdjancencyList& ad = it->second;
                    if(std::find(ad.begin(), ad.end(), target) != ad.end()) {
                        result.push_back(it->first);
                    }
                }
                return result;
            }
            else {
                return result;
            }
        }

        /// 有向图的拓扑排序
        template<typename T >
        typename digraph<T >::AdjancencyList digraph<T >::top_sort() const
        {
            AdjancencyList          result;
            digraph<T >             backup = *this;

            while(true) {
                AdjancencyList  node;
                for(typename Nodes::const_iterator it = backup.nodes_.begin(); it != backup.nodes_.end(); ++it) {
                    size_t indegree = backup.get_join(it->first).size();
                    if(indegree == 0) {
                        node.push_back(it->first);
                    }
                }
                if(node.empty()) {
                    break;
                }
                result.insert(result.end(), node.begin(), node.end());

                for(size_t i = 0; i < node.size(); ++i) {
                    AdjancencyList  edge = backup.get_edge(node[i]);
                    for(size_t j = 0; j < edge.size(); ++j) {
                        backup.del_edge(node[i], edge[j]);
                    }
                    if(backup.get_edge(node[i]).empty() && backup.get_join(node[i]).empty()) {
                        backup.del_node(node[i]);
                    }
                }
            }

            return result;
        }

        template<typename T >
        bool digraph<T >::is_cycle() const
        {
            return top_sort().size() != nodes();
        }

        template<typename T >
        void digraph<T >::clear()
        {
            nodes_.clear();
        }

    }
}

#endif

#ifndef _BALUEXT_H_
#define _BALUEXT_H_

#include "testlib.h"
#include <vector>
#include <utility>
#include <queue>
#include <algorithm>
#include <numeric>
#include <map>
#include <set>
#include <functional>
#include <ext/pb_ds/assoc_container.hpp>
#include <ext/pb_ds/tree_policy.hpp>

typedef std::vector<std::pair<int, int>> Edges;

/*
O(n)
*/
void shuffle_edges(Edges &edges, int base = 0) {
    shuffle(edges.begin(), edges.end());
    for (auto &e : edges) {
        if (rnd.next(0, 1))
            std::swap(e.first, e.second);
        e.first += base;
        e.second += base;
    }
}

/*
O(nlogn)
*/
void relabel_edges(Edges &edges, int base = 0) {
    std::vector<int> label;
    for (auto &e : edges) {
        label.push_back(e.first);
        label.push_back(e.second);
    }
    std::sort(label.begin(), label.end());
    label.resize(std::unique(label.begin(), label.end()) - label.begin());
    for (auto &e : edges) {
        e.first = std::lower_bound(label.begin(), label.end(), e.first) - label.begin() + base;
        e.second = std::lower_bound(label.begin(), label.end(), e.second) - label.begin() + base;
    }
}

/*
convert from Prüfer sequence
O(nlogn)
*/
Edges uniform_tree(int size, int base = 0) {
    if (size <= 0)
        __testlib_fail("uniform_tree: size must greater then 0");
    if (size == 1)
        return {};
    Edges rt;
    std::vector<int> seq(size - 2), cnt(size, 0);
    std::priority_queue<int, std::vector<int>, std::greater<int>> leaves;
    std::pair<int, int> lst;
    for (int i = 0; i < size - 2; ++i) {
        seq[i] = rnd.next(0, size - 1);
        ++cnt[seq[i]];
    }
    for (int i = 0; i < size; ++i)
        if (!cnt[i])
            leaves.push(i);
    for (int i : seq) {
        rt.push_back(std::make_pair(leaves.top(), i));
        leaves.pop();
        if (!--cnt[i])
            leaves.push(i);
    }
    lst.first = leaves.top();
    leaves.pop();
    lst.second = leaves.top();
    rt.push_back(lst);
    shuffle_edges(rt, base);
    return rt;
}

/*
shuffle the nodes and connect one from prefix for each node, for each node:
type0: no restriction
type1: connect with rnd.next(std::max(0, i - dis), i - 1)
type2: connect with rnd.next(0, std::min(dis - 1, i - 1))
Note that when type not equal to 1 and 2, it is regraded as 0
O(n)
*/
Edges custom_tree(int size, int type, int dis = 1, int base = 0) {
    if (size <= 0)
        __testlib_fail("custom_tree: size must greater then 0");
    if ((type == 1 || type == 2) && dis <= 0)
        __testlib_fail("custom_tree: dis must greater then 0 when type is 1 or 2");
    if (size == 1)
        return {};
    Edges rt;
    std::vector<int> idx(size);
    std::iota(idx.begin(), idx.end(), 0);
    shuffle(idx.begin(), idx.end());
    for (int i = 1; i < size; ++i)
        if (type == 1)
            rt.push_back(std::make_pair(idx[rnd.next(std::max(0, i - dis), i - 1)], idx[i]));
        else if (type == 2)
            rt.push_back(std::make_pair(idx[rnd.next(0, std::min(dis - 1, i - 1))], idx[i]));
        else
            rt.push_back(std::make_pair(idx[rnd.next(0, i - 1)], idx[i]));
    shuffle_edges(rt, base);
    return rt;
}

/*
Generate a simple bipartite graph
Caller should provide "std::vector<int> colors" to store the color of the nodes in the generated graph
Also, if the size of colors is greater than of equal to base + size, either colors[i] is 0 or 1 wil be regarded as the default color of node i, otherwise colors[i] for all i in [base, base + size) will be covered (and others will be remained); if the size of colors is smaller than base + size, it will be resize to base + size
Caller can also optionally provide an edges base
Note: edge_num will be automatically decrease if it exceeds the limits
*/
Edges bipartite_graph(int size, int edge_num, std::vector<int> &colors, int base = 0, const Edges &base_edges = Edges(), double prob = 0.5) {
    std::vector<std::vector<int>> G(base + size, std::vector<int>());
    for (const auto &e : base_edges) {
        if (e.first < base || e.first >= base + size)
            __testlib_fail("bipartite_graph: there is a nodes' label in base_edges not in the range [base, base + size).");
        if (e.second < base || e.second >= base + size)
            __testlib_fail("bipartite_graph: there is a node's label in base_edges not in the range [base, base + size).");
        G[e.first].push_back(e.second);
        G[e.second].push_back(e.first);
    }
    std::vector<int> vis(base + size, 0);
    if (int(colors.size()) < base + size)
        colors.resize(base + size, -1);
    std::function<int(int)> dfs;
    dfs = [&](int u) {
        vis[u] = 1;
        for (int i : G[u]) {
            if (!vis[i]) {
                if (!(colors[i] == 0 || colors[i] == 1))
                    colors[i] = 1 - colors[u]; 
                if (!dfs(i))
                    return 0;
            }
            if (colors[i] == colors[u])
                return 0;
        }
        return 1;
    };
    for (int i = base; i < base + size; ++i)
        if (!vis[i] && (colors[i] == 0 || colors[i] == 1)) {
            if (!dfs(i))
                __testlib_fail("bipartite_graph: base_edges is not a bipartite graph or it conflicts with the colors");
        }
    for (int i = base; i < base + size; ++i)
        if (!(colors[i] == 0 || colors[i] == 1)) {
            if (rnd.next((double)0, (double)1) < prob)
                colors[i] = 0;
            else
                colors[i] = 1;
            if (!dfs(i))
                __testlib_fail("bipartite_graph: base_edges is not a bipartite graph or it conflicts with the colors");
        }
    std::vector<int> black, white;
    for (int i = base; i < base + size; ++i)
        if (colors[i] == 0)
            black.push_back(i);
        else
            white.push_back(i);
    Edges rt(base_edges);
    std::set<std::pair<int, int>> edge_vis;
    for (auto &e : rt) {
        if (e.first > e.second)
            std::swap(e.first, e.second);
        edge_vis.insert(e);
    }
    edge_num = std::min((long long)edge_num, (long long)black.size() * (long long)white.size());
    edge_num -= base_edges.size();
    if ((long long)black.size() * white.size() - (edge_num + base_edges.size()) <= 10000000) {
        std::vector<std::pair<int, int>> edge_pool;
        for (int i : black)
            for (int j : white) {
                int a = std::min(i, j);
                int b = std::max(i, j);
                if (edge_vis.find(std::make_pair(a, b)) == edge_vis.end())
                    edge_pool.push_back(std::make_pair(a, b));
            }
        shuffle(edge_pool.begin(), edge_pool.end());
        for (int i = 0; i < edge_num; ++i)
            rt.push_back(edge_pool[i]);
    }
    else {
        for (int i = 0; i < edge_num; ++i) {
            int a, b;
            do {
                a = black[rnd.next(0, int(black.size()) - 1)];
                b = white[rnd.next(0, int(white.size()) - 1)];
                if (a > b)
                    std::swap(a, b);
            } while (edge_vis.find(std::make_pair(a, b)) != edge_vis.end());
            edge_vis.insert(std::make_pair(a, b));
            rt.push_back(std::make_pair(a, b));
        } 
    }
    shuffle_edges(rt);
    return rt;
}

/*
TODO
*/
Edges Cactus_edge() {
    __testlib_fail("Cactus_edge: This function hasn't been finished");
    return Edges(); 
}

/*
TODO
*/
Edges Cactus_vertex() {
    __testlib_fail("Cactus_edge: This function hasn't been finished");
    return Edges(); 
}

/*
iterator:
typedef typename std::map<Key, weight_type, Compare, Allocator_map>::const_iterator iterator;
*/
template<
class Key, 
class weight_type = double, 
class Compare = std::less<Key>, 
class Allocator_map = std::allocator<Key>,
class Allocator_pbds = std::allocator<char>>
class weight_pool {
    template<class Node_CItr, class Node_Itr, class Cmp_Fn, class _Alloc>
    struct my_node_update {
        typedef weight_type metadata_type;
        void operator()(Node_Itr it, Node_CItr end_it) {
            Node_Itr l = it.get_l_child();
            Node_Itr r = it.get_r_child();
            metadata_type left = 0, right = 0;
            if(l != end_it) left = l.get_metadata();
            if(r != end_it) right = r.get_metadata();
            const_cast<metadata_type&>(it.get_metadata()) = left + right + (*it)->second;
        }
        metadata_type sum() {
            if (node_begin() == node_end())
                return 0;
            return node_begin().get_metadata();
        }
        Key next(metadata_type value) {
            if (value > sum())
                return Key();
            Node_CItr it = node_begin();
            while (it != node_end()) {
                Node_CItr l = it.get_l_child();
                Node_CItr r = it.get_r_child();
                if (l != node_end() && l.get_metadata() >= value)
                    it = l;
                else {
                    if (l != node_end())
                        value -= l.get_metadata();
                    if (r == node_end() || (*it)->second >= value)
                        return (*it)->first;
                    value -= (*it)->second;
                    it = r;
                }
            }
            return Key();
        }
        virtual Node_CItr node_begin() const = 0;
        virtual Node_CItr node_end() const = 0;
    };
    struct pair_compare {
        bool operator()(const std::pair<Key, weight_type>& a, const std::pair<Key, weight_type>& b) const {
            if (!Compare()(a.first, b.first) && !Compare()(b.first, a.first))
                return a.second < b.second;
            return Compare()(a.first, b.first);
        }
    };
    __gnu_pbds::tree<std::pair<Key, weight_type>, __gnu_pbds::null_type, pair_compare, __gnu_pbds::rb_tree_tag, my_node_update, Allocator_pbds> pool;
    std::map<Key, weight_type, Compare, Allocator_map> pool_record;
public:
    typedef typename std::map<Key, weight_type, Compare, Allocator_map>::const_iterator iterator;
    weight_pool(){}
    weight_pool(std::map<Key, weight_type, Compare, Allocator_map> _pool_record): pool_record(_pool_record) {
        for (auto p : pool_record)
            insert(p.first, p.second);
    }
    iterator insert(const Key &key, const weight_type &value) {
        auto p = pool_record.find(key);
        if (p == pool_record.end())
            p = pool_record.insert(std::make_pair(key, 0)).first;
        else
            pool.erase(pool.lower_bound(*p));
        p->second = value;
        pool.insert(*p);
        return p;
    }
    size_t erase(const Key& key) {
        auto p = pool_record.find(key);
        if (p == pool_record.end())
            return 0;
        pool.erase(pool.lower_bound(*p));
        pool_record.erase(p);
        return 1;
    }
    iterator erase(iterator it) {
        pool.erase(pool.lower_bound(*it));
        return pool_record.erase(it);
    }
    size_t size() {
        return pool_record.size(); 
    }
    bool empty() {
        return pool_record.empty();
    }
    void clear() {
        pool.clear();
        pool_record.clear();
    }
    iterator begin() {
        return pool_record.begin(); 
    }
    iterator end() {
        return pool_record.end(); 
    }
    iterator find(const Key &key) {
        return pool_record.find(key); 
    }
    void swap(weight_pool<Key, Compare, Allocator_map, Allocator_pbds> &other) {
        pool.swap(other.pool);
        pool_record.swap(other.pool_record);
    }
    Key next() {
        return pool.next(rnd.next((weight_type)0, pool.sum()));
    }
};

#endif // _BALUEXT_H_

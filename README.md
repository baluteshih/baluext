# baluext

## Intro
This project is an extension for the C++ implementation of the [testlib](https://github.com/MikeMirzayanov/testlib). For the convenience of test data generation, `baluext.h` contains some commonly used tools. You can see more information in the following samples or in the source code.

## Sample

### tree edges

In `baluext.h`, the edge structure of a tree is defined as,
```cpp
typedef std::vector<std::pair<int, int>> tree_edges;
```
Relatively, all of the trees will be expressed as a set of "edges".

#### shuffle_edges

```cpp
void shuffle_edges(tree_edges &edges, int base = 0);
```

Shuffle the order of edges, also randomly swap the "first" and "second" of each pair.

An additional `base` will be directly added to each label.

#### relabel_edges

```cpp
void relabel_edges(tree_edges &edges, int base = 0);
```

Relabel the labels of the edges with 0-base.

An additional `base` will be directly added to each label.

### Trees

`baluext.h` supports generating trees.

#### uniform_tree

```cpp
tree_edges uniform_tree(int size, int base = 0);
```

Use the Prüfer sequence to generate a tree with `size` nodes. Each tree will be generated with the same probability.

An additional `base` will be directly added to each label.

#### custom_tree

```cpp
tree_edges custom_tree(int size, int type, int dis = 1, int base = 0);
```

Shuffle the nodes and connect one from prefix for each node, for each node:
```
type0: no restriction
type1: connect with rnd.next(std::max(0, i - dis), i - 1)
type2: connect with rnd.next(0, std::min(dis - 1, i - 1))
```
Notice that when `type` not equal to 1 and 2, it is regraded as 0

An additional `base` will be directly added to each label.

### weight_pool

```cpp
template<
typename Key, // Key type
typename weight_type = double, // Weight type
typename Compare = std::less<Key>, // Key comparison functor
class Allocator_map = std::allocator<Key>, // An allocator type for std::map
class Allocator_pbds = std::allocator<char>> // An allocator type for __gnu_pbds::tree
class weight_pool;
```

`weight_pool` is a data structure that can insert some elements into it with a specific weight. After then, `weight_pool` can return random elements, the higher the weight, the higher probability the element will be returned.

By the way, `weight_pool` is a combination with `std::map` and `__gnu_pbds::tree`. Hence, you can see two types of `Allocator`. Notice that the key for `__gnu_pbds::tree` is `std::pair<Key, weight_type>`.

#### insert

```cpp
iterator insert(const Key &key, const weight_type &value);
```

An element `key` will be inserted into the pool with weight `value`. If `key` is already in the pool, the original element will be covered.

An iterator will be returned, which points to the element.

#### erase

```cpp
size_t erase(const Key &key);
```

Remove the element (if one exists) with the key equivalent to `key`. Will return the number of elements removed (0 or 1).

#### next

```cpp
Key next();
```

Return random inserted elements, the higher the weight, the higher probability the element will be returned.

#### iterator

```cpp
typedef typename 
std::map<Key, weight_type, Compare, Allocator_map>::const_iterator
iterator;
```
It is actually a `const_iterator` of `std::map`. Some common functions such as `erase(iterator)`, `begin()`, `end()`, `find()` are also supported.

#### other common functions

Some common functions such as `size()`, `empty()`, `swap()`, `clear()` are also supported. The features of them are almost same as `std::map`.

#### example

```cpp
#include "testlib.h"
#include "baluext.h"
#include <iostream>

weight_pool<int> pool;

int main(int argc, char* argv[]) {
    registerGen(argc, argv, 1);
    pool.insert(1, 10);
    pool.insert(2, 20);
    pool.insert(3, 30);
    pool.insert(4, 40);
    for (int i = 1; i <= 10; ++i)
        std::cout << pool.next() << " \n"[i == 10];
    weight_pool<int>::iterator it = pool.find(4);
    pool.erase(it);
    for (int i = 1; i <= 10; ++i)
        std::cout << pool.next() << " \n"[i == 10];
}
```

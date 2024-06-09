#include <iostream>
#include <algorithm>
#include <fstream>
#include <vector>
#include <iterator>
#include <set>
#include <pthread.h>

using namespace std;

class CSRGraph {
public:
    int node;
    int edge;
    int *col_indices;
    int *row_offsets;
    int *true_index;
    int **array;
    int *query_list;

    CSRGraph();
    static bool compare(const int* a, const int* b);
    void ReadTheGraph(char *pathname);
    void removeDuplicates();
    void GetFourArray();
};


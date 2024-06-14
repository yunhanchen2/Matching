#include <iostream>
#include <algorithm>
#include <fstream>
#include <vector>
#include <iterator>
#include <set>
#include <time.h>
#include <pthread.h>
#include "../include/CSRGraph.h"

using namespace std;

CSRGraph::CSRGraph() {
    node=0;
    edge=0;
}

void CSRGraph::ReadTheGraph(char *pathname){
    ifstream fin;
    fin.open(pathname);//n edges and m nodes
    fin >> edge >> node;
    edge = edge * 2;

    array = new int *[edge];  // 动态分配二维数组的行
    for (int i = 0; i < edge; ++i) {
        array[i] = new int[2];  // 动态分配二维数组的列
    }

    col_indices = new int[edge]();
    row_offsets = new int[node + 1]();
    true_index = new int[node]();

    for (int i = 0; i < edge; ++i) {
        int m1, n1;
        fin >> m1 >> n1;

        array[i][0] = m1;
        array[i][1] = n1;
        i++;
        array[i][0] = n1;
        array[i][1] = m1;
    }

    fin.close();

    sort(array, array + edge, compare);

    query_list = new int[array[edge - 1][0] + 1]();

}

bool CSRGraph::compare(const int* a, const int* b) {
    if (a[0] != b[0]) {
        return a[0] < b[0];  // 按照第一个数从小到大排列
    }
    return a[1] < b[1];  // 对于相同的第一个数，按照第二个数从小到大排列
}

void CSRGraph::removeDuplicates() {
    int newSize = 0;
    for (int i = 0; i < edge; i++) {
        if (i == 0 || !(array[i][0] == array[i - 1][0] && array[i][1] == array[i - 1][1])) {  // 如果当前元素不等于前一个元素，则将其保留
            array[newSize][0] = array[i][0];
            array[newSize][1] = array[i][1];
            newSize++;
        }
    }

    edge = newSize;  // 更新数组大小
}

void CSRGraph::GetFourArray(){
    for (int i = 0; i < edge; i++) {
        true_index[i] = array[i][0];
    }

    //get the true index
    int slow = 0; // 慢指针
    for (int fast = 1; fast < edge; fast++) { // 快指针
        if (true_index[fast] != true_index[slow]) {
            slow++;
            true_index[slow] = true_index[fast]; // 将非重复元素移到慢指针位置
        }
    }

    for (int i = 1; i < node + 1; i++) {
        query_list[true_index[i - 1]] = i;//已知真正的编号 要找到对应是第几个（对应1~m)
    }

    for (int i = 0; i < edge; i++) {
        row_offsets[query_list[array[i][0]] - 1]++;
    }

    int temp = 0;
    int accumulate = 0;

    for (int i = 0; i < node; i++) {
        temp = row_offsets[i];
        row_offsets[i] = accumulate;
        accumulate += temp;
    }
    row_offsets[node] = accumulate;

    for (int i = 0; i < edge; i++) {
        col_indices[i] = array[i][1];
    }
}

void CSRGraph::Clear(){
    delete[] col_indices;
    delete[] row_offsets;
    delete[] true_index;

    for (int i = 0; i < node; i++) {
        delete[] array[i];
    }
    delete[] array;

    delete[] query_list;
}






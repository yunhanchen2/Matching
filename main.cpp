#include <iostream>
#include <algorithm>
#include <fstream>
#include <vector>
#include <iterator>
#include <set>

using namespace std;

bool compare(const int* a, const int* b) {
    if (a[0] != b[0]) {
        return a[0] < b[0];  // 按照第一个数从小到大排列
    }
    return a[1] < b[1];  // 对于相同的第一个数，按照第二个数从小到大排列
}

struct CSRGraph {
    int *col_indices;
    int *row_offsets;
};

void removeDuplicates(int **arr, int& size) {
    int newSize = 0;
    for (int i = 0; i < size; i++) {
        if (i == 0 || !(arr[i][0] == arr[i - 1][0] && arr[i][1] == arr[i - 1][1])) {  // 如果当前元素不等于前一个元素，则将其保留
            arr[newSize][0] = arr[i][0];
            arr[newSize][1] = arr[i][1];
            newSize++;
        }
    }

    size = newSize;  // 更新数组大小
}

void get_true_index(int arr[], int n) {
    if (n == 0) {
        return;
    }

    int slow = 0; // 慢指针
    for (int fast = 1; fast < n; fast++) { // 快指针
        if (arr[fast] != arr[slow]) {
            slow++;
            arr[slow] = arr[fast]; // 将非重复元素移到慢指针位置
        }
    }
}

vector<int> vectors_intersection(vector<int> v1,vector<int> v2){
    vector<int> v;
    sort(v1.begin(),v1.end());
    sort(v2.begin(),v2.end());
    set_intersection(v1.begin(),v1.end(),v2.begin(),v2.end(),back_inserter(v));//求交集
    return v;
}

vector<int> getTheCandidate(int* tem,int index,int* nei,CSRGraph graph,int size_of_nei,int* true_index,int* query_list,int m){
    vector<int> back;
    if(size_of_nei==0){
        for(int i=0;i<m;i++){
            back.push_back(true_index[i]);
        }
    } else {
        //testing
        vector< vector<int> > neibor(size_of_nei);
        for(int i=0;i<size_of_nei;i++){//将邻居放入vector中
            for(int j=graph.row_offsets[query_list[tem[nei[i]]]-1];j<graph.row_offsets[query_list[tem[nei[i]]]];j++){
                neibor[i].push_back(graph.col_indices[j]);//放入的是对应的编号而非第几个
            }
        }

        //vector join
        back=neibor[0];
        for(int i=1;i<size_of_nei;i++){
            back= vectors_intersection(back,neibor[i]);
        }

        //cut off the node before the node
        vector<int>::iterator it;
        for(it=back.begin();it!=back.end();){
            bool check=true;
            for(int j=0;j<index;j++){
                if(*it==tem[j]){
                    it=back.erase(it);
                    check= false;
                }
            }
            if(check){
                it++;
            }
        }
    }

    return back;
}

int main(int argc,char* argv[]) {
    if (argc > 1) {

        //get the pattern graph
        int edge;
        int node;
        int *index_ptr_of_pattern=new int[node+1]();
        int *indices_of_pattern=new int[edge*2]();
        cout<<"input the number of edge and node pattern graph"<<endl;
        cin>>edge>>node;
        cout<<"input the index_ptr_of_pattern(from 0 to n)"<<endl;
        for(int i=0;i<node+1;i++){
            cin>>index_ptr_of_pattern[i];
        }
        cout<<"input the indices_of_pattern(from 0 to n)"<<endl;
        for(int i=0;i<edge*2;i++){
            cin>>indices_of_pattern[i];
        }

        //get neighbor of each node
        int *num_of_neighbor=new int[node];
        for(int i=0;i<node;i++){
            num_of_neighbor[i]=index_ptr_of_pattern[i+1]-index_ptr_of_pattern[i];
        }
        int *order=new int[node];
        int max=node-1;
        int marker=0;
        while(marker!=node){
            for(int i=0;i<node;i++){
                if(max==num_of_neighbor[i]){
                    order[marker]=i;
                    marker++;
                }
            }
            max--;
        }

        //find out the restriction of nodes
        vector < vector<int> > nei(node);
        for(int i=0;i<node;i++){
            for(int j=0;j<i;j++){//node before them in the order
                for(int k=index_ptr_of_pattern[order[i]];k<index_ptr_of_pattern[order[i]+1];k++){
                    if(indices_of_pattern[k]==order[j]){
                        nei[order[i]].push_back(j);
                    }
                }
            }
        }

        //get the data graph
        char *pathname = argv[1];
        std::ifstream fin;
        fin.open(pathname);
        int n, m;//n edges and m nodes

        fin >> n >> m;

        n = n * 2;

        CSRGraph graph;

        graph.col_indices = new int[n]();
        graph.row_offsets = new int[m + 1]();

        int size = n;

        int **array = new int *[size];  // 动态分配二维数组的行
        for (int i = 0; i < size; ++i) {
            array[i] = new int[2];  // 动态分配二维数组的列
        }

        for (int i = 0; i < size; ++i) {
            int m1, n1;
            fin >> m1 >> n1;

            array[i][0] = m1;
            array[i][1] = n1;
            i++;
            array[i][0] = n1;
            array[i][1] = m1;
        }

        fin.close();

        // 按照要求对数组进行排序
        std::sort(array, array + size, compare);

        removeDuplicates(array, n);

        //find the true index
        int *true_index = new int[m]();

        for (int i = 0; i < n; i++) {
            true_index[i] = array[i][0];
        }

        get_true_index(true_index, n);//true_index[2]是第三小的node对应的编号(i=1~n-1)

        int *query_list = new int[array[n - 1][0] + 1]();

        for (int i = 1; i < m + 1; i++) {
            query_list[true_index[i - 1]] = i;//已知真正的编号 要找到对应是第几个（对应1~m)
        }

        //get row_offsets
        for (int i = 0; i < n; i++) {
            graph.row_offsets[query_list[array[i][0]] - 1]++;
        }

        int temp = 0;
        int accumulate = 0;

        for (int i = 0; i < m; i++) {
            temp = graph.row_offsets[i];
            graph.row_offsets[i] = accumulate;
            accumulate += temp;
        }
        graph.row_offsets[m] = accumulate;

        //get col_indices
        for (int i = 0; i < n; i++) {
            graph.col_indices[i] = array[i][1];
        }


        //create queue
        vector< vector<int> > M(node*(node+1)/2);

        //create counters
        int *counter= new int[node+1]();
        counter[0]=1;

        //do the matching
        int begin_ptr=0;
        int *tem=new int[node-1]();//tem存放的是真正的编号

        for(int i=0;i<node;i++){//node times matching
            int id=order[i];

            begin_ptr+=i;
            for(int l=0;l<counter[i];l++) {//上一次matching的时候有counter[i]种的连接方式
                vector<int> back;
                //dequeue
                if (i == 0) {
                    i++;
                    i--;
                } else {//将上一次的matching值取出
                    for (int j = 0; j < i; j++) {//j表示上一次每matching一组会产生j个
                        tem[j] = M[begin_ptr - i + j][l];
                    }
                }
                //query graph中的neighbor限制
                int* neibor=new int[nei[id].size()]();

                if(nei[id].size()!=0){
                    for(int p=0;p<nei[id].size();p++){
                        neibor[p]=nei[id][p];
                    }
                }

                back=getTheCandidate(tem,i,neibor,graph,nei[id].size(),true_index,query_list,m);

                for (int j = 0; j < back.size(); j++) {//满足其邻居条件以后,j为candidate node中的jth元素
                    if ((graph.row_offsets[query_list[back[j]]] - graph.row_offsets[query_list[back[j]] - 1]) >= num_of_neighbor[order[i]]) {//degree也满足了
                        for (int k = 0; k < i ; k++) {
                            M[begin_ptr+k].push_back(tem[k]);//将原来的存回去
                        }
                        M[begin_ptr+i].push_back(back[j]);//存新match的
                        counter[i+1]++;
                    }
                }
                for (int j = 0; j < i; j++) {//tem init
                    tem[j] =0;
                }
            }
        }

        //cut the duplicate

        //put the final answer in the new vector
        set < set<int> > ss;
        for(int i=0;i<counter[node];i++){
            set<int> each;
            for(int j=0;j<node;j++){
                each.insert(M[(node*(node-1)/2)+j][i]);
            }
            ss.insert(each);
        }

        //testing
        /*for(int i=0;i<counter[node];i++){
            for(int j=0;j<node;j++){
                cout<<final_total[i][j]<<" ";
            }
            cout<<endl;
        }*/


        /*for(int i=0;i<final_total.size();i++){
            for(int j=0;j<node;j++){
                cout<<final_total[i][j]<<" ";
            }
            cout<<endl;
        }*/

        //testing
        cout<<"total counting: "<<ss.size()<<endl;

        return 0;
    }
}
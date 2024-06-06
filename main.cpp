#include <iostream>
#include <algorithm>
#include <fstream>
#include <vector>
#include <iterator>
#include <set>
#include <time.h>
#include <pthread.h>
#include "PatternGraph.h"
#include "CSRGraph.h"


using namespace std;

static int number_of_thread;

static CSRGraph graph;

class DataPassingToThreads{
public:
    static int * num_of_neighbor;
    static int * order;
    int * passing_node_to_thread_of_each;
    int round_index;
    int *neighbor_of_prenode_pattern;
    int size_of_neighbor_of_prenode_pattern;
    int number_of_matching;
    DataPassingToThreads(int *aPassing_node_to_thread_of_each,int aRound_index,int * aNeighbor_of_prenode_pattern,int aSize_of_neighbor_of_prenode_pattern,int aNumber_of_matching){
        passing_node_to_thread_of_each=aPassing_node_to_thread_of_each;
        round_index=aRound_index;
        neighbor_of_prenode_pattern=aNeighbor_of_prenode_pattern;
        size_of_neighbor_of_prenode_pattern=aSize_of_neighbor_of_prenode_pattern;
        number_of_matching=aNumber_of_matching;
    }
};

int* DataPassingToThreads::num_of_neighbor = nullptr;
int* DataPassingToThreads::order = nullptr;


struct DataForPassingBack{
    int number_of_matching_node;
    int *matching_node;
};

struct ThreadData{
    DataPassingToThreads *data;
};

vector<int> vectors_intersection(vector<int> v1,vector<int> v2){
    vector<int> v;
    sort(v1.begin(),v1.end());
    sort(v2.begin(),v2.end());
    set_intersection(v1.begin(),v1.end(),v2.begin(),v2.end(),back_inserter(v));//求交集
    return v;
}

void* graph_matching_threads(void *n){
    //in threads: get the neighbor and check degree store them in the vectors

    cout<<"this is "<<pthread_self()<<endl;

    ThreadData* dataT=(ThreadData*) n;
    DataPassingToThreads *dataPassingToThreads=dataT->data;

    DataForPassingBack *passingBack=new DataForPassingBack();

    vector< vector<int> > M(dataPassingToThreads->round_index+1);
    passingBack->number_of_matching_node=0;


    if(dataPassingToThreads->round_index==0){
        //only check the degree
        for (int j = 0; j < dataPassingToThreads->number_of_matching; j++) {//满足其邻居条件以后,j为candidate node中的jth元素
            if ((graph.row_offsets[graph.query_list[dataPassingToThreads->passing_node_to_thread_of_each[j]]] - graph.row_offsets[graph.query_list[dataPassingToThreads->passing_node_to_thread_of_each[j]] - 1]) >= dataPassingToThreads->num_of_neighbor[dataPassingToThreads->order[dataPassingToThreads->round_index]]) {//degree也满足了
                M[dataPassingToThreads->round_index].push_back(dataPassingToThreads->passing_node_to_thread_of_each[j]);//存新match的
                passingBack->number_of_matching_node++;
            }
        }

    } else {
        int *tem;
        for(int i=0;i<dataPassingToThreads->number_of_matching;i++){
            //each time pick one group
            tem=new int[dataPassingToThreads->round_index];
            for(int j=0;j<dataPassingToThreads->round_index;j++){
                tem[j]=dataPassingToThreads->passing_node_to_thread_of_each[i*dataPassingToThreads->round_index+j];
            }

            //get the neighbors
            vector<int> back;
            vector< vector<int> > neibor(dataPassingToThreads->size_of_neighbor_of_prenode_pattern);
            for(int k=0;k<dataPassingToThreads->size_of_neighbor_of_prenode_pattern;k++){//将邻居放入vector中
                for(int r=graph.row_offsets[graph.query_list[tem[dataPassingToThreads->neighbor_of_prenode_pattern[k]]]-1];r<graph.row_offsets[graph.query_list[tem[dataPassingToThreads->neighbor_of_prenode_pattern[k]]]];r++){
                    neibor[k].push_back(graph.col_indices[r]);//放入的是对应的编号而非第几个
                }
            }

            //vector join
            back=neibor[0];
            for(int j=1;j<dataPassingToThreads->size_of_neighbor_of_prenode_pattern;j++){
                back= vectors_intersection(back,neibor[j]);
            }

            //cut off the node before the node and have a new matching

            vector<int>::iterator it;
            for(it=back.begin();it!=back.end();){
                bool check=true;
                for(int j=0;j<dataPassingToThreads->round_index;j++){
                    if(*it==tem[j]){
                        it=back.erase(it);
                        check= false;
                    }
                }
                if(check){
                    it++;
                }
            }

            //check the degree
            for (int j = 0; j < back.size(); j++) {//满足其邻居条件以后,j为candidate node中的jth元素
                if ((graph.row_offsets[graph.query_list[back[j]]] - graph.row_offsets[graph.query_list[back[j]] - 1]) >= dataPassingToThreads->num_of_neighbor[dataPassingToThreads->order[dataPassingToThreads->round_index]]) {//degree也满足了

                    for (int k = 0; k < dataPassingToThreads->round_index; k++) {
                        M[k].push_back(tem[k]);//将原来的存回去

                    }
                    M[dataPassingToThreads->round_index].push_back(back[j]);//存新match的
                    passingBack->number_of_matching_node++;

                }
            }
        }
    }


    //making the number passing back
    passingBack->matching_node=new int[passingBack->number_of_matching_node*(1+dataPassingToThreads->round_index)];
    for(int i=0;i<passingBack->number_of_matching_node;i++){
        for(int j=0;j<(1+dataPassingToThreads->round_index);j++){
            passingBack->matching_node[i*(1+dataPassingToThreads->round_index)+j]=M[j][i];
        }
    }

    pthread_exit(passingBack);
}


int main(int argc,char* argv[]) {

    if (argc > 2) {
        number_of_thread=atoi(argv[2]);
        //get the pattern graph
        int e;
        int nod;

        cout<<"input the number of edge and node pattern graph"<<endl;
        cin>>e>>nod;

        PatternGraph patternGraph(e,nod);

        cout<<"input the index_ptr_of_pattern(from 0 to n)"<<endl;
        for(int i=0;i<patternGraph.node+1;i++){
            cin>>patternGraph.index_ptr_of_pattern[i];
        }

        cout<<"input the indices_of_pattern(from 0 to n)"<<endl;
        for(int i=0;i<patternGraph.edge*2;i++){
            cin>>patternGraph.indices_of_pattern[i];
        }

        //get neighbor of each node
        patternGraph.GetTheNeighborOfEachNode();

        //get the order
        patternGraph.GetTheMatchingOrder();

        //find out the restriction of nodes
        vector < vector<int> > nei(patternGraph.node);
        for(int i=0;i<patternGraph.node;i++){
            for(int j=0;j<i;j++){//node before them in the order
                for(int k=patternGraph.index_ptr_of_pattern[patternGraph.order[i]];k<patternGraph.index_ptr_of_pattern[patternGraph.order[i]+1];k++){
                    if(patternGraph.indices_of_pattern[k]==patternGraph.order[j]){
                        nei[patternGraph.order[i]].push_back(j);
                    }
                }
            }
        }

        //get the data graph
        char *pathname = argv[1];
        graph.ReadTheGraph(pathname);//read+sort
        graph.removeDuplicates();
        //find the true index
        graph.GetFourArray();//true_index[2]是第三小的node对应的编号(i=1~n-1)


       /* //pass value to the static data
        DataPassingToThreads::g.row_offsets=graph.row_offsets;
        DataPassingToThreads::g.col_indices=graph.col_indices;
        DataPassingToThreads::g.query_list=graph.query_list;
        DataPassingToThreads::g.true_index=graph.true_index;
        DataPassingToThreads::g.node=graph.node;
        DataPassingToThreads::g.edge=graph.edge;
*/
        DataPassingToThreads::num_of_neighbor=patternGraph.num_of_neighbor;

        DataPassingToThreads::order=patternGraph.order;


        //do the matching
        pthread_t tid[number_of_thread];
        int counter;

        //record time
        clock_t s,fi;
        double d;
        s=clock();

        //divide the node into groups i=0->all node; i!=0->the node for last time
        vector< vector<int> > M(patternGraph.node*(patternGraph.node+1)/2);
        int number_of_node_for_last_matching=graph.node;
        int begin_ptr=0;
        int* neighbor_of_prenode;

        for(int i=0;i<patternGraph.node;i++){
            int id=patternGraph.order[i];
            counter=0;
            //preparing data
            //query graph中的neighbor限制
            int size_of_neighbor_of_prenode=0;

            if(nei[id].size()!=0){
                neighbor_of_prenode=new int[nei[id].size()]();
                for(int p=0;p<nei[id].size();p++){
                    neighbor_of_prenode[p]=nei[id][p];
                }
                size_of_neighbor_of_prenode=nei[id].size();
            }

            //lunch the threads
            int full_node_for_each_thread=number_of_node_for_last_matching/number_of_thread;
            int remaining=number_of_node_for_last_matching-full_node_for_each_thread*number_of_thread;
            if(remaining==0){
                remaining=number_of_thread;
            } else {
                full_node_for_each_thread++;
            }
            int sharing_node_ptr=0;

            int *passing_node_to_thread_of_each[number_of_thread];
            DataPassingToThreads *dataPassingToThreads[number_of_thread];
            int *number_of_matching=new int[number_of_thread]();
            ThreadData *args=new ThreadData[number_of_thread];

            for (int p = 0; p < number_of_thread; p++) {
                if(p<remaining){
                    number_of_matching[p]=full_node_for_each_thread;
                }else {
                    number_of_matching[p]=full_node_for_each_thread-1;
                }

                if(i==0){
                    passing_node_to_thread_of_each[p]=new int[number_of_matching[p]];
                    for(int t=0;t<number_of_matching[p];t++){
                        passing_node_to_thread_of_each[p][t]=graph.true_index[sharing_node_ptr];
                        sharing_node_ptr++;
                    }
                } else {
                    passing_node_to_thread_of_each[p]=new int[number_of_matching[p]*i];
                    for(int t=0;t<number_of_matching[p];t++){
                        for(int k=0;k<i;k++){
                            passing_node_to_thread_of_each[p][t*i+k]=M[begin_ptr+k][sharing_node_ptr];
                        }
                        sharing_node_ptr++;
                    }
                }

                dataPassingToThreads[p]=new DataPassingToThreads(passing_node_to_thread_of_each[p],i,neighbor_of_prenode,size_of_neighbor_of_prenode,number_of_matching[p]);

                args[p].data = dataPassingToThreads[p];
                pthread_create(&tid[p], NULL, graph_matching_threads, &args[p]);
            }

            //get vectors in each thread and merge them together
            DataForPassingBack* ptr_get=new DataForPassingBack[number_of_thread];

            for (int p = 0; p < number_of_thread; p++) {
                void * ptr;
                pthread_join(tid[p], &ptr);
                ptr_get[p]=*((DataForPassingBack*) ptr);

                counter+=ptr_get[p].number_of_matching_node;
                for(int k=0;k<ptr_get[p].number_of_matching_node;k++){
                    for(int r=0;r<i+1;r++){
                        M[begin_ptr+i+r].push_back(ptr_get[p].matching_node[k*(i+1)+r]);
                    }
                }
            }

            number_of_node_for_last_matching=counter;

            begin_ptr+=i;

            delete [] number_of_matching;
            delete [] args;
        }

        //ending time
        fi=clock();

        d=(double)(fi-s)/CLOCKS_PER_SEC;
        cout<<"time of graph matching is: "<<d<<"with "<<number_of_thread<<"threads"<<endl;

        set < set<int> > ss;
        for(int i=0;i<counter;i++){
            set<int> each;
            for(int j=0;j<patternGraph.node;j++){
                each.insert(M[(patternGraph.node*(patternGraph.node-1)/2)+j][i]);
            }
            ss.insert(each);
        }

        //testing
        cout<<"total counting: "<<ss.size()<<endl;

        delete [] neighbor_of_prenode;
    }

         return 0;
}
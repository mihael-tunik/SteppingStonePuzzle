#include <stdio.h>
#include <algorithm>
#include <vector>
#include <stdlib.h>
#include <chrono>
#include <omp.h>

#include "read.h"
#include "write.h"

#define N 25
#define M 6
#define THREADS 4

using namespace std;

int field[THREADS][N][N];
int conv_field[THREADS][N*N];
int max_score[THREADS], cnt[THREADS];

int sub_grid_size = (N-2)*(N-2);
int idx_precomp[(N-2)*(N-2)];

void dfs(int num, int task){  
    int flag_visited = 1, i, j, idx;
    
    for(int l = 0; l < sub_grid_size; ++l){
        idx = idx_precomp[l];
        i = idx / N, j = idx % N;
          
        if(conv_field[task][idx] == num && field[task][i][j] == 0)
        {             
            flag_visited = 0;
                
            field[task][i][j] = num;

            conv_field[task][idx - N - 1] += num;
            conv_field[task][idx - N    ] += num;
            conv_field[task][idx - N + 1] += num;
                                
            conv_field[task][idx - 1] += num;
            conv_field[task][idx    ] += num;
            conv_field[task][idx + 1] += num;
                                
            conv_field[task][idx + N - 1] += num;
            conv_field[task][idx + N    ] += num;
            conv_field[task][idx + N + 1] += num;                              
                  
            dfs(num + 1, task);
            
            field[task][i][j] = 0;
                
            conv_field[task][idx - N - 1] -= num;
            conv_field[task][idx - N    ] -= num;
            conv_field[task][idx - N + 1] -= num;
                                
            conv_field[task][idx - 1] -= num;
            conv_field[task][idx    ] -= num;
            conv_field[task][idx + 1] -= num;
                                
            conv_field[task][idx + N - 1] -= num;
            conv_field[task][idx + N    ] -= num;
            conv_field[task][idx + N + 1] -= num;
        }
    }
        
    if(flag_visited){
        ++cnt[task];        
        if (num > max_score[task])
            max_score[task] = num - 1;
    }
    return;
}

void run_from_file(char *file_path, char *label){ 
  
     vector <vector <vector <int>>> v;
     vector <pair<int, int>> results; 
          
     int border[THREADS + 1] = {0};    
     int tasks_num = THREADS, batch_size = 0;
     
     for(int l = 0; l < (N-2)*(N-2); ++l)   
        idx_precomp[l] = l + N + 1 + 2*(l / (N-2));  
         
     read_from_file(file_path, v, &batch_size, N, M);
     
     results.resize(batch_size);
     
     printf("Data has been successfully read from file!\n");     
     printf("Batch size: %i\n", batch_size);
          
     printf("Task borders:\n"); 
         
     for(int i = 0; i < THREADS+1; ++i){
         border[i] = i * batch_size / THREADS;
         printf("%i\n", border[i]);
     }
     
     border[THREADS] = batch_size;    
     
     #pragma omp parallel for
     for(int task = 0; task < tasks_num; ++task){
     
       for(int k = border[task]; k < border[task+1]; ++k)
       {
          cnt[task] = 0, max_score[task] = 0;
                  
          for(int i = 0; i < N; ++i){
            for(int j = 0; j < N; ++j){                
              field[task][i][j] = v[k][i][j];
              conv_field[task][N*i + j] = 0;
            }  
          }
          
          for(int i = 1; i < N-1; ++i)
              for(int j = 1; j < N-1; ++j)
                  for(int k = 0; k <= 8; ++k){
                      conv_field[task][N*i + j] += field[task][i + k/3 - 1][j + k%3 - 1]; 
                  } 
                  
                             
          int num = 2;
      
          dfs(num, task);                 
          results[k] = make_pair(max_score[task], k);
          
          if(k % 100 == 0)
              printf("task: %i, max[%i]: %i, %i\n", task, k, max_score[task], cnt[task]);     
      }
      
     }
     #pragma omp barrier
     
     printf("Computation was finished...\n");
          
     char file_name[256];     
     sprintf(file_name, "dfs_results_sorted_%s.txt", label);
     
     write_results(results, v, file_name, N, M);
}

//  Example run:   
//  dfs_search /path_to_file some_label
   
//  Input file format:
//  line 1:
//  integers: number of problem instances, square grid size 
//     K, N
//  lines: 2...K+1:   
//  K lines with M pairs of integers: 1's coordinates
//     p11 p12 p21 p22 p31 p32 p41 p42...

int main(int argc, char *argv[]){
    char *file_path, *label;
    
    if(argc == 3){
        file_path = argv[1];
        label = argv[2];
    }else{
        return -1;
    }
       
    printf("File: %s, label: %s\n", file_path, label);
    
    auto start = chrono::high_resolution_clock::now();
    run_from_file(file_path, label);
    auto stop = chrono::high_resolution_clock::now();
    
    printf("Ready in %lf s.\n", chrono::duration<double, milli>(stop-start).count()/1000);     
    return 0;
}

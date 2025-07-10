#include <stdio.h>
#include <algorithm>
#include <string>
#include <vector>
#include <math.h>
#include <chrono>
#include <omp.h>

#define M   7
#define N   31
#define PAD 1

#define THREADS 16
#define SUB_N_SQUARE (N-2)*(N-2)
#define MAX_NUM 128
#define THRESHOLD 45

using namespace std;

unsigned long long multibyte_sum[MAX_NUM];
int sub_grid_size = SUB_N_SQUARE;

int idx_precomp    [SUB_N_SQUARE];
int idx_precomp_row[SUB_N_SQUARE];
int idx_precomp_col[SUB_N_SQUARE];

unsigned char field[THREADS][N*N];
unsigned short conv_field[THREADS][(N*N + 1)]; // !
int max_score[THREADS], cnt[THREADS];

unsigned long long *conv_f_r1[SUB_N_SQUARE * THREADS]; //[THREADS][SUB_N_SQUARE];
unsigned long long *conv_f_r2[SUB_N_SQUARE * THREADS];
unsigned long long *conv_f_r3[SUB_N_SQUARE * THREADS];

//FILE *output_files[THREADS];
string output_file;

void dfs(char num, char task){  
    unsigned long long mbs = 1, idx, l_thread = 0, offset_sub = task * SUB_N_SQUARE;
    
    for(int l = 0; l < sub_grid_size; ++l){
        idx = idx_precomp[l];      
            
        if(conv_field[task][idx] == num && field[task][idx] == 0){             
            /* section A */
            field[task][idx] = num;
            mbs = multibyte_sum[num];            
            l_thread = offset_sub + l;
            
            *conv_f_r1[l_thread] += mbs;
            *conv_f_r2[l_thread] += mbs;
            *conv_f_r3[l_thread] += mbs;
                  
            dfs(num + 1, task);
            
            field[task][idx] = 0;
            
            *conv_f_r1[l_thread] -= mbs;
            *conv_f_r2[l_thread] -= mbs;
            *conv_f_r3[l_thread] -= mbs;
        }
    }
    /* mbs inside section A can take values from following list:
       [0, 65793, 131586, ...], so it can be used as flag
    */
    if(mbs == 1 && num > max_score[task]){
        max_score[task] = num - 1;
    } 
    return;
}

void prepare_multibyte_sums()
{
    for(int k = 0; k < MAX_NUM; ++k){
         unsigned short byte_array[4];
         long long unsigned *ptr = (long long unsigned*)byte_array;   
          
         byte_array[0] = k;
         byte_array[1] = k;
         byte_array[2] = k;
         byte_array[3] = 0;
         
         multibyte_sum[k] = *ptr;
         //printf("mbs: %llu\n", multibyte_sum[k]);
     }
}

void build_lookup_tables(){
    
    for(int l = 0; l < sub_grid_size; ++l){
        // map inner square coords (unrolled) to augmented s.c. (unrolled)
        //             o o       x x x x
        // o o o o ->  o o  ->   x o o x ->  x x x x  x o o x  x o o x  x x x x 
        //                       x o o x
        //                       x x x x
        //
        // in fashion:
        // (N = 4, see comment above)
        // 0 -> (0, 0) -> (1, 1) -> 5  == 0 + 4 + 1 + 2*(0 / (4-2))
        // 3 -> (1, 1) -> (2, 2) -> 10 == 3 + 4 + 1 + 2*(3 / (4-2))
        // ...
        // l -> (l/(N-2), l%(N-2)) -> (... + 1, ... + 1) -> N*(l / (N-2) + 1) + (l%(N-2) + 1)
        // == N*(l / (N-2) + 1) + (l - (N-2) * (l / (N - 2))  + 1)
        // ... that's because
        // {l = (N-2) * (l / (N - 2)) + l % (N-2)}
        // ...
        // == 2 * (l/(N-2)) + l + N + 1
        idx_precomp[l] = l + N + 1 + 2*(l / (N-2));
        
        idx_precomp_row[l] = idx_precomp[l] / N;
        idx_precomp_col[l] = idx_precomp[l] % N;
  
        int offset_sub = 0;
        //unsigned char *conv_field_ptr;
        
        for(int thread_id = 0; thread_id < THREADS; ++thread_id){
            offset_sub = thread_id * SUB_N_SQUARE + l;
            //offset = thread_id * (N*N + 1);
            //conv_field_ptr = conv_field[thread_id] + idx_precomp[l];
            
            conv_f_r1[offset_sub] = (long long unsigned*)(conv_field[thread_id] + idx_precomp[l] - N - 1 );
            conv_f_r2[offset_sub] = (long long unsigned*)(conv_field[thread_id] + idx_precomp[l]     - 1 );
            conv_f_r3[offset_sub] = (long long unsigned*)(conv_field[thread_id] + idx_precomp[l] + N - 1 );
        }
     }
}

void log(vector <vector <int>> field_v, int score, FILE *f){
    vector <pair<int, int>> compressed;
    
    for(int i = 0; i < N; ++i)
        for(int j = 0; j < N; ++j)
            if(field_v[i][j] == 1)
                compressed.push_back(make_pair(i, j));
                
    fprintf(f, "Score %i:\n", score);
    for(int k = 0; k < compressed.size(); ++k)
        fprintf(f, "%i %i ", compressed[k].first, compressed[k].second);
    fprintf(f, "\n");
}
 
void run_from_file(char *input_filename, int batch_size){

     prepare_multibyte_sums();
     build_lookup_tables();
     
     FILE *f = fopen(input_filename, "r");
     
     //vector <vector <vector <char>>> v;    
     vector <vector <pair<char, char>>> v;
     vector <int> score;
               
     int boarder[THREADS + 1] = {0};
     int m = M, pad = PAD;
               
     for(int i = 0; i <= THREADS; ++i)
         boarder[i] = (i * batch_size) / THREADS;
     
     printf("Start reading batch...\n");
     
     for(int k = 0; k < batch_size; ++k)
     {        
          vector <pair<char, char>> field_v; 
                                   
          for(int i = 0; i < m; ++i){
              int r, c;
              fscanf(f, "%i %i", &r, &c);
              field_v.push_back(make_pair((char)(r+pad), char(c+pad)));
          }
          if( (k + 1) % 1000000 == 0)
              printf("%i / %i\n", k + 1, batch_size);     
          v.push_back(field_v);          
     }     
     fclose(f);
     
     printf("Ready! Size: %lu\n", v.size());
     score.resize(v.size());
     
     int tasks_num = THREADS;
          
     #pragma omp parallel for shared(v)
     for(int task = 0; task < tasks_num; ++task){
             
       for(int k = boarder[task]; k < boarder[task+1]; ++k)
       {
          cnt[task] = 0, max_score[task] = 0;
          int offset = task * (N*N + 1);
                  
          for(int i = 0; i < N; ++i){
            for(int j = 0; j < N; ++j){                
              field[task][N*i + j] = 0; //v[k][i][j];
              conv_field[task][N*i + j] = 0; //memset?
            }  
          }
          
          for(int i = 0; i < m; ++i){
              char r = v[k][i].first, c = v[k][i].second;
              field[task][N*r + c] = 1;
          }
          
          for(int i = 1; i < N-1; ++i)
              for(int j = 1; j < N-1; ++j){   
                  int idx_ij = N*i + j;
                        
                  conv_field[task][idx_ij] += field[task][N*(i - 1) + j - 1];
                  conv_field[task][idx_ij] += field[task][N*(i - 1) + j    ];
                  conv_field[task][idx_ij] += field[task][N*(i - 1) + j + 1];
                  
                  conv_field[task][idx_ij] += field[task][N*i       + j - 1];
                  conv_field[task][idx_ij] += field[task][N*i       + j    ];
                  conv_field[task][idx_ij] += field[task][N*i       + j + 1];
                  
                  conv_field[task][idx_ij] += field[task][N*(i + 1) + j - 1];
                  conv_field[task][idx_ij] += field[task][N*(i + 1) + j    ];
                  conv_field[task][idx_ij] += field[task][N*(i + 1) + j + 1];
              }    
                  
                             
          //int start_num = 2;      
          dfs(2, task);
        
          if(k % 1000 == 0){
             float progress = 100.0f * (k - boarder[task])/(boarder[task+1]-boarder[task]);
             //printf("thread[%i]: %.1f%% [%i: %i, %i]\n", task, progress, k, boarder[task], boarder[task+1]);    
             printf("thread[%i]: %.1f%%\n", task, progress);     
          }
          score[k] = max_score[task];          
      }
     }
     #pragma omp barrier
          
     FILE *f_out = fopen(output_file.c_str(), "w");

     for(int i = 0; i < score.size(); ++i){
         
         for(int j = 0; j < m; ++j){
              int r = (int)v[i][j].first, c = (int)v[i][j].second;
              fprintf(f_out, " %i %i", r, c);     
         }
         fprintf(f_out, ", %i\n", score[i]);
     }
     fclose(f_out);
}

int main(int argc, char **argv){
    output_file = string("dfs_computed_") + string(argv[1]);
    
    auto start = chrono::high_resolution_clock::now();
    run_from_file(argv[1], atoi(argv[2]));
    auto stop = chrono::high_resolution_clock::now();
    
    printf("Ready in %lf s.\n", chrono::duration<double, milli>(stop-start).count()/1000);     
    return 0;
}

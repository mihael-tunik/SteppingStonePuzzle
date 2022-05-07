#include "read.h"

void read_instance_line(FILE* f, vector <vector<int>> &field_v, int N, int M){
    vector <int> coords;
    int offset = 2, tmp;
    
    field_v.resize(N);                   
         
    for(int i = 0; i < N; ++i)
       field_v[i].resize(N, 0);
          
    for(int i = 0; i < 2*M; ++i){     
       fscanf(f, "%i", &tmp);
       coords.push_back(tmp);
    }
             
    for(int i = 0; i < M; ++i)
         field_v[coords[2*i]+offset][coords[2*i+1]+offset] = 1;
            
    return;
}

void read_from_file(char *file_path, vector <vector <vector <int>>> &v, int *batch_size, int N, int M){
     
     FILE *f = fopen(file_path, "r");     
     fscanf(f, "%i\n", batch_size); 
      
     for(int k = 0; k < *batch_size; ++k){        
          vector <vector<int>> field_v;          
          read_instance_line(f, field_v, N, M);  
          v.push_back(field_v);          
      }
      
     fclose(f);
}

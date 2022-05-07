#include "write.h"

bool sort_desc(const pair <int, int> &a, const pair <int, int> &b){
   return a.first > b.first;
}

vector <pair<int, int>> get_ones(vector <vector<int>> &field)
{
    vector <pair<int, int>> ones;
    
    for(int i = 0; i < field.size(); ++i)
       for(int j = 0; j < field[0].size(); ++j)
           if(field[i][j] == 1)
               ones.push_back(make_pair(i, j));

    return ones;
}

void write_results(vector <pair<int, int>> &results, vector <vector <vector <int>>> &v, char *file_name, int N, int M){     
     FILE *f_res = fopen(file_name, "a+"); 
     
     fprintf(f_res, "Grid: %i, items: %i, source: %s\n", N, M, file_name);
     
     sort(results.begin(), results.end(), sort_desc);
          
     for(int j = 0; j < results.size(); ++j){
        fprintf(f_res, "Score: %i\n", results[j].first);
        vector <pair <int, int>> ones = get_ones(v[results[j].second]);
 
        for(int l = 0; l < M; ++l)
            fprintf(f_res, "%i %i ", ones[l].first, ones[l].second);
        fprintf(f_res, "\n");
     }
     
     fclose(f_res);     
}

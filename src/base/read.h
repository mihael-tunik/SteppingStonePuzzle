#ifndef _READ_H_
#define _READ_H_
#include <stdio.h>
#include <algorithm>
#include <vector>
#include <stdlib.h>

using namespace std;

void read_from_file(char *file_path, vector <vector <vector <int>>> &v, int *batch_size, int N, int M);

#endif

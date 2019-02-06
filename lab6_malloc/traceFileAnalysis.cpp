#include<iostream>
#include<fstream>
#include<string>
#include <map>

#define DSIZE 8

using namespace std;
int main(int argv, char *arg[])
{
	string file[11] = { "amptjp-bal.rep", "binary2-bal.rep", "binary-bal.rep" ,
		"cccp-bal.rep", "coalescing-bal.rep", "cp-decl-bal.rep",
		"expr-bal.rep", "random2-bal.rep", "random-bal.rep",
		"realloc2-bal.rep", "realloc-bal.rep" };
	string dir = "E:\\视频教程\\2015 CMU 15-213 CSAPP 深入理解计算机系统\\labs\\lab6_malloclab\\traces\\";
		
	string line;
	int pos;
	int size;
	int asize;
	map<int, int> asizeCount;

	int start = 0;
	int  end = 10;
	int se = 10;

	for (int i = start; i <= end;i++) {
		fstream f(dir + file[i]);
		cout << file[i]<<endl;

		while (getline(f, line))
		{
			pos = line.find("a");

			if (pos == 0) {
				pos = line.rfind(" ");
				size = atoi(line.substr(pos + 1).data());

				if (size <= DSIZE) {
					asize = 2 * DSIZE;
					//1个word大小的header+1个word的大小的footer，又需要整个block的大小为偶数个word，所以最小需要4个word
				}
				else {
					asize = DSIZE * ((size + (DSIZE)+(DSIZE - 1)) / DSIZE);//这地方可能会有问题
				}
				if (asizeCount.count(asize) == 0) {
					asizeCount[asize] = 1;
				}
				else {
					asizeCount[asize] = asizeCount[asize] + 1;
				}
			}
		}

		f.close();
	}	

	map<int, int>::reverse_iterator iter;
	for (iter = asizeCount.rbegin(); iter != asizeCount.rend(); iter++) {
		cout << iter->first<<" "<<iter->second<< endl;
	}
	
	system("pause");
	return 0;
}
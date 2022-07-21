#include <iostream>
#include "string"
#include "fstream"
#include "sstream"
#include "vector"
#include "thread"
#include <ctime>

struct Line{
    std::string line;
    int index{};
};

struct Occurence{
    int line{};
    int index{};
    std::string str;
};

std::vector<Line> lines;

std::vector<Occurence> ocList;

std::vector<std::thread> threadList;

std::mutex listLock;

void fetchOccurences(const Line& line, const std::string &token){
    std::stringstream ss;
    std::string buffer;
    ss << line.line;
    int word_index=0;
    while (ss >> buffer){

        int ocIndex = buffer.find(token);

        if (ocIndex!=std::string::npos){
            struct Occurence newOc;
            newOc.line = line.index;
            newOc.index = ocIndex+word_index;
            newOc.str = buffer.substr(0, ocIndex + token.length());
            ocList.push_back(newOc);

        }
        word_index+=buffer.length()+1;
    }
}

void fetchBatchOccurences(const std::string &token){
    listLock.lock();
    for (int i=0; i<lines.size();i++){
//        std::cout<<lines[i].line<<std::endl;
        fetchOccurences(lines[i], token);
    }
    lines.clear();
    listLock.unlock();
}

int main(int argc, char *argv[]) {
//     clock_t start, end;
//     start = clock();
    std::string filename = std::string(argv[1]), mask = std::string(argv[2]), line;

    std::string token = mask.substr(1, mask.length()-1);

    std::ifstream infile;
    infile.open(filename, std::ios::in);

    int lineIndex = 1;
    if (infile.is_open()){
        while (std::getline(infile, line)){
            Line newLine;
            newLine.index = lineIndex;
            newLine.line=line;
            lines.push_back(newLine);
            if (lines.size() % 3 == 0) {
                std::thread t(fetchBatchOccurences, token);
                threadList.push_back(std::move(t));
            }
            lineIndex++;
        }
        std::thread t(fetchBatchOccurences, token);
        threadList.push_back(std::move(t));
    } else {
        exit(1);
    }

    for (auto & t : threadList){
        t.join();
    }

    infile.close();

    std::cout << ocList.size() << std::endl;
    for (struct Occurence occ : ocList){
        std::cout << occ.line << " " << occ.index << " " << occ.str << std::endl;

    }
//     end = clock();
//     double time_taken = start-end;
//     std::cout << time_taken << std::endl;

    return 0;
}

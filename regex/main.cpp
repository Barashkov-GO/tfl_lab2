#include <iostream>
#include <string>
#include <vector>
#include <regex>
#include <fstream>
#include <ctime>

using namespace std;

int TEST_COUNT = 10;
int LOOP_COUNT = 100;
map<string, vector<int>> times;

string read(int i){
    ifstream f;
    string line;
    string path1 = "../tests/test";
    string path2 = to_string(i);
    string path3 = ".txt";
    f.open(path1 + path2 + path3, ios::in);
    string s;
    if (f.is_open()){
        while(getline(f, line)) {
            s = s + line;
        }
    } else {
        cout << "\n\tERROR while reading test file\n\n";
    }
    f.close();
    return s;
}

float get_average(string type) {
    float sum = 0.0f;
    for (auto t : times[type]){
        sum += t;
    }
    return sum / times[type].size();
}

pair<bool, float> test_regex(regex r, int test_number, string reg_name) {
    pair<bool, float> out;
    string line = read(test_number + 1);
    auto time_start = clock();
    line = regex_replace(line, r, "");
    auto time_of_regex = clock() - time_start;
    out = {line.empty(), time_of_regex / 1000.0f};
    times[reg_name].push_back(time_of_regex);
    return out;
}

int main() {
    string s;


    regex reg_classic(R"((int(\s)+[a-zA-Z_][a-zA-Z0-9_]*(\s)*(=(\s)*[0-9]+)?(\s)*;(\n)*)*)", regex_constants::ECMAScript);
    // Ограничим алфавит регулярным выражением [a-zA-Z0-9_=;\s\n]
    regex reg_negative(R"((int(\s)+[^0-9=;\s\n]+[^=;\s\n]*?(\s)*(=(\s)*[^a-zA-Z_]+)?(\s)*;(\n)*)*)", regex_constants::ECMAScript);
    regex reg_kleene(R"((int(\s)+?[a-zA-Z_][a-zA-Z0-9_]*?(\s)*?(=(\s)*[0-9]+)?(\s)*?;(\n)*?)*)", regex_constants::ECMAScript);
    for (int j = 0; j < LOOP_COUNT; j++) {
        cout << "\nLOOP " << j << endl;
        for (int i = 0; i < TEST_COUNT; i++) {
            cout << "TEST " << i + 1 << endl;
            auto out = test_regex(reg_classic, i, "classic");
            cout << "\tclassic:\n\t\t" << (out.first == 1 ? "True" : "False") << "\n\t\t" << out.second << " seconds\n";
            out = test_regex(reg_negative, i, "negative");
            cout << "\tnegative:\n\t\t" << (out.first == 1 ? "True" : "False") << "\n\t\t" << out.second
                 << " seconds\n";
            out = test_regex(reg_kleene, i, "kleene");
            cout << "\tkleene:\n\t\t" << (out.first == 1 ? "True" : "False") << "\n\t\t" << out.second << " seconds\n";

        }
    }

//     При изменении тестов (и количества проходов по всем тестам) сильно меняется среднее время,
//     лидерство перехватывается,
//     так что оценивать эффективность тех или иных регулярных выражений по данной лабораторной работе нельзя
//     так же мы ограничили алфавит в регулярном выражении с отрицаниями (так как по-другому не получится из-за размеров алфавита),
//     и поэтому среднее время с этой регуляркой ниже, на случаях с малым алфавитом такая регулярка более эфективна,
//     на случаях с большим - написать ее попросту невозможно (или очень сложно)
//     (может быть разная кодировка и, следовательно, количество символов)


    cout << "for " << LOOP_COUNT << " loops:\n";
    cout << "Average for classic : " << get_average("classic") << " milliseconds" << endl;
    cout << "Average for negative : " << get_average("negative") << " milliseconds" << endl;
    cout << "Average for kleene : " << get_average("kleene") << " milliseconds" << endl;

    return 0;
}

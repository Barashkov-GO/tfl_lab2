#include <iostream>
#include <string>
#include <vector>
#include <regex>
#include <fstream>
#include <set>

using namespace std;

int TEST_COUNT = 11;
int LOOP_COUNT = 1;


vector<string> split(string line, char c) {
    vector<string> out;
    string s;
    for (auto sc : line){
        if (sc != c){
            s += sc;
        } else {
            if (!s.empty()) {
                out.push_back(s);
                s.clear();
            }
        }
    }
    if(!s.empty()) {
        out.push_back(s);
    }
    return out;
}

class Equation {
public:
    string lang;
    map<string, string> langs;
    vector<string> free_regs;
    string origin;

    Equation(string lang, string s) {
        this->origin = lang + "->" + s;
        this->lang = lang;
        s = "+" + s;
        regex langs("[A-Z]");
        regex not_langs("[^A-Z]");
        regex regs("[\\+][^A-Z\\+]*[A-Z]");
        regex free_regs("[\\+][^A-Z\\+]+");
        regex plus("\\+");

        auto words_begin = sregex_iterator(s.begin(), s.end(), regs);
        auto words_end = sregex_iterator();
        for (sregex_iterator i = words_begin; i != words_end; ++i) {
            smatch match = *i;
            string match_str = match.str();
            match_str = regex_replace(match_str, plus, "");
            string l = regex_replace(match_str, not_langs, "");
            string r = regex_replace(match_str, langs, "");
            if(!this->langs[l].empty()){
                this->langs[l] = "(" + this->langs[l] + "+" + r + ")";
            } else {
                this->langs[l] = r;
            }
        }
        s = regex_replace(s, regs, "");
        words_begin = sregex_iterator(s.begin(), s.end(), free_regs);
        words_end = sregex_iterator();
        bool f = false;
        string s_help;
        for (sregex_iterator i = words_begin; i != words_end; ++i) {
            smatch match = *i;
            string match_str = match.str();
            match_str = regex_replace(match_str, plus, "");
            if (count(match_str.begin(), match_str.end(), '(') != 0){
                f = true;
                s_help.clear();
            }
            if (f) {
                s_help += match_str + "+";
                if (count(match_str.begin(), match_str.end(), ')') != 0) {
                    f = false;
                    s_help = s_help.erase(s_help.length() - 1, 1);
                    this->free_regs.push_back(s_help);
                }
            } else {
                this->free_regs.push_back(match_str);
            }
        }
    }
    bool check_accuracy(){
        regex acc(R"([^\+]*\|[^\+]*)"); // expression with |
        regex acc2(R"([^\+]{0,}\([^\+\)]*\([a-z])", regex_constants::ECMAScript); // double parenthesis
        regex acc3(R"([^\+]*\([^\+]*\|[^\+]*\)[^\+]*)"); // expression with ( ..|.. )
        for (auto t : this->langs) {
            if (t.second.empty()){
                cout << "Error in regular expression: empty expression within " << t.first << endl;
                return false;
            }
            auto w1 = sregex_iterator(t.second.begin(), t.second.end(), acc);
            auto w2 = sregex_iterator();
            auto w3 = sregex_iterator(t.second.begin(), t.second.end(), acc2);
            if (w1 != w2) {
                smatch match = *w1;
                bool f = regex_replace(match.str(), acc3, "").empty();
                if (!f) {
                    cout << "Error in regular expression: | not supposed to be used this way " << match.str() << endl;
                    return false;
                }
            }
            string s2 = regex_replace(t.second, acc2, "");
            if (s2.length() < t.second.length()) {
                smatch match = *w3;
                cout << "Error in regular expression: double level of parenthesis " << t.second << endl;
                return false;
            }
        }
        for (auto t : free_regs) {
            auto w1 = sregex_iterator(t.begin(), t.end(), acc);
            auto w2 = sregex_iterator();
            auto w3 = sregex_iterator(t.begin(), t.end(), acc2);
            if (w1 != w2) {
                smatch match = *w1;
                bool f = regex_replace(match.str(), acc3, "").empty();
                if (!f) {
                    cout << "Error in regular expression: | not supposed to be used this way " << match.str() << endl;
                    return false;
                }
            }
            string s2 = regex_replace(t, acc2, "");
            if (s2.length() < t.length()) {
                smatch match = *w3;
                cout << "Error in regular expression: double level of parenthesis " << match.str() << endl;
                return false;
            }
        }
        return true;
    }

    string to_string() {
        string out;
        out = out + this->lang + "=";
        for (auto t : this->langs){
            out += t.second + t.first;
            out += "+";
        }
        for (int j = 0; j < free_regs.size(); j++) {
            out += free_regs[j];
            out += "+";
        }
        if (out[out.length() - 1] == '+'){
            out = out.erase(out.length() - 1, 1);
        }
        return out;
    }

    string to_string_optimized() {
        string out;
        out = out + this->lang + "=";
        for (auto t : this->langs){
            out += t.second + t.first;
            out += " + ";
        }
        for (int j = 0; j < free_regs.size(); j++) {
            out += free_regs[j];
            out += " + ";
        }
        if (out[out.length() - 2] == '+'){
            out = out.erase(out.length() - 2, 2);
        }
        return out;
    }

    string to_string_without_lang() {
        return this->to_string().erase(0, 2);
    }
};

vector<Equation> syst;

bool check_accuracy_langs(){
    map<string, int> lang_exist;
    for (auto equation : syst){
        lang_exist[equation.lang] = 1;
        for (auto equation_lang : equation.langs){
            if(lang_exist[equation_lang.first] != 1)
            lang_exist[equation_lang.first] = -1;
        }
    }
    for (auto t : lang_exist){
        if (t.second == -1){
            cout << "Error: no equation for " << t.first << endl;
            return false;
        }
    }
    return true;
}

        // (X = aX + b) -> (X = a*b),
        // where a - regular expression,
        // b - expression with regexes and languages

Equation get_solution(Equation a) {
    string out_lang;
    string reg_lang; // regex within language of equation a
    if(!a.langs[a.lang].empty()) {
        reg_lang = a.langs[a.lang] + "*";
    }
    for (auto t : a.langs){
        if (t.first != a.lang){
            out_lang += reg_lang + t.second + t.first + "+";
        }
    }
    for (auto t : a.free_regs){
        out_lang += reg_lang + t + "+";
    }
    if (out_lang.empty()){
        out_lang = reg_lang + "+";
    }
    out_lang.erase(out_lang.length() - 1, 1);
    return {a.lang, out_lang};
}

        // replaces language X in equation for Y with expression without X
Equation substitute(Equation x, Equation y) {
    string out_lang;
    string reg_lang; // regex within language of X of equation of Y
    reg_lang = y.langs[x.lang];
    for (auto t : x.langs){
        out_lang += reg_lang + t.second + t.first + "+";
    }
    for (auto t : x.free_regs){
        out_lang += reg_lang + t + "+";
    }
    for (auto t : y.langs){
        if (t.first != x.lang){
            out_lang += t.second + t.first + "+";
        }
    }
    for (auto t : y.free_regs){
        out_lang += t + "+";
    }
    if (out_lang.empty()){
        out_lang = reg_lang + "+";
    }
    out_lang.erase(out_lang.length() - 1, 1);
    return {y.lang, out_lang};
}

bool is_exist(Equation x, Equation y) {
    for (auto t : y.langs){
        if (t.first == x.lang) {
            return true;
        }
    }
    return false;
}

void solve(vector<Equation>& system) {
    vector<Equation> out;
    for (int i = 0; i < system.size(); i++){
        for (int j = i + 1; j < system.size(); j++) {
            if (is_exist(system[i], system[j])) {
                auto i_solved = get_solution(system[i]);
                system[j] = substitute(i_solved, system[j]);
//                cout << system[i].to_string() << '\t' <<
//                i_solved.to_string() << '\t' << system[j].to_string() << endl;
            }
        }
    }
    for (int i = system.size() - 1; i > -1; i--){
        for (int j = i - 1; j > -1; j--) {
            system[i] = get_solution(system[i]);
            if (is_exist(system[i], system[j])) {
                system[j] = substitute(system[i], system[j]);
//                cout << system[i].to_string_optimized() << '\t' << system[j].to_string_optimized() << endl;
            }
            system[j] = get_solution(system[j]);
        }
    }
    for (auto t : system){
        regex r2("\\(([a-z])\\)");
        cout << regex_replace(t.to_string_optimized(), r2, "$1") << endl;
    }
}

string delete_spaces(string line) {
    regex r(R"((\s)+)", regex_constants::ECMAScript);
    line = regex_replace(line, r, "");
    return line;
}

pair<string, string> parse(string line) {
    regex r1(R"(=.+)", regex_constants::ECMAScript);
    regex r2(R"(.+=)", regex_constants::ECMAScript);
    string line_before = regex_replace(line, r1, "");
    string line_after = regex_replace(line, r2, "");
    return {line_before, line_after};
}

string read(int i){
    ifstream f;
    string line;
    string path1 = "../tests/test";
    string path2 = to_string(i);
    string path3 = ".txt";
    f.open(path1 + path2 + path3, ios::in);
    string s;
    if (f.is_open()){
        syst.clear();
        while(getline(f, line)) {
            auto a = parse(delete_spaces(line));
            syst.emplace_back(a.first, a.second);
            cout << "\t" << syst[syst.size() - 1].to_string_optimized() << endl;
            if(!syst[syst.size() - 1].check_accuracy()){
                return "";
            }
        }
        if (!check_accuracy_langs()) {
            return "";
        }
        cout << "\nSolution: \n";
        solve(syst);
    } else {
        cout << "\n\tERROR while reading test file\n\n";
    }
    f.close();
    return s;
}

int main() {
    string s;

    for (int j = 0; j < LOOP_COUNT; j++) {
        for (int i = 0; i < TEST_COUNT; i++) {
            cout << "TEST " << i + 1 << endl;
            read(i + 1);
            cout << endl;
        }
    }
    return 0;
}

#include <iostream>
#include <string>
#include <vector>
#include <regex>
#include <fstream>
#include <set>

using namespace std;

int TEST_COUNT = 10;
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

    bool check_loop(){
        if (langs.find(lang) != langs.end() && langs.size() == 1 && free_regs.empty() && lang != "S") {
            return true;
        }
        return false;
    }

    bool operator== (Equation other){
        if(this->lang == other.lang && this->langs == other.langs && this->free_regs == other.free_regs){
            return true;
        }
        return false;
    }

    bool operator< (Equation other) {
        if (other.lang == "S"){
            return true;
        }
        return false;
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
        out = out + this->lang + "->";
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
        out = out + this->lang + "->";
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
        return this->to_string().erase(0, 3);
    }
};

vector<Equation> syst;
vector<bool> eq_banned;


string open_brackets(string line) {
    line = "+" + line;
    string out;
    regex r1("\\+\\((.*?)\\)([A-Z])");
    string help1 = regex_replace(line, r1, "");
    sregex_iterator it_begin = sregex_iterator(line.begin(), line.end(), r1);
    sregex_iterator it_end = sregex_iterator();
    for (auto it = it_begin; it != it_end; it++) {
        string str = (*it).str();
        string reg_group = regex_replace(str, r1, "$1");
        string lang = regex_replace(str, r1, "$2");
        auto group_split = split(reg_group, '+');
        for (auto group_part : group_split) {
            out += group_part + lang + "+";
        }
    }
    if (out.empty()){
        return line.erase(0, 1);
    }
    out.erase(out.length() - 1, 1);
    out += help1;
    return out;
}

    // (X = aX + b) -> (X = a*b),
    // where a - regular expression,
    // b - expression with regexes and languages
Equation get_solution(Equation a) {
    string out_lang;
    string reg_lang; // regex within language of equation a
    if(!a.langs[a.lang].empty()) {
        reg_lang = "(" + a.langs[a.lang] + ")*";
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
    return {a.lang, open_brackets(out_lang)};
}

    // replaces language X in equation for Y with expression without X
Equation substitute(Equation x, Equation y) {
    string out;
    string reg_lang; // regex within language of X of equation of Y
    reg_lang = y.langs[x.lang];
    for (auto t : x.langs){
        if (t.second[0] == '(') {
            auto help = split(t.second.erase(0, 1).erase(t.second.length() - 1, 1), '+');
            out += "(";
            for (auto hh : help) {
                out += reg_lang + hh + "+";
            }
            out = out.erase(out.length() - 1, 1) + ")" + t.first + "+";
        } else {
            out += reg_lang + t.second + t.first + "+";
        }
    }
    for (auto t : x.free_regs){
        out += reg_lang + t + "+";
    }
    for (auto t : y.langs){
        if (t.first != x.lang){
            out += t.second + t.first + "+";
        }
    }
    for (auto t : y.free_regs){
        out += t + "+";
    }
    if (out.empty()){
        out = reg_lang + "+";
    }
    out.erase(out.length() - 1, 1);
    return {y.lang, open_brackets(out)};
}

void reformat_syst(){
    vector<Equation> new_syst;
    map<string, vector<int>> new_langs;
    for (int i = 0; i < syst.size(); ++i){
        new_langs[syst[i].lang].push_back(i);
    }
    for (auto t : new_langs){
        string new_str;
        for (auto tt : t.second){
            new_str += syst[tt].to_string_without_lang() + "+";
        }
        new_str.erase(new_str.length() - 1, 1);
        new_syst.emplace_back(t.first, new_str);
    }
    for (int i = 0; i < new_syst.size(); i++){
        eq_banned.push_back(false);
    }
    sort(new_syst.begin(), new_syst.end());
    syst = new_syst;
}

void out_system() {
    for (auto s : syst) {
        cout << "\t\t" << s.to_string() << endl;
    }
}

bool outputted = false;

void solve(vector<Equation>& system) {
    for (int i = 0; i < system.size(); i++){
        for (int j = i + 1; j < system.size(); j++) {
            if (system[i].check_loop()) {
                system[j].langs.erase(system[i].lang);
                if (system[j].langs.empty() && system[j].free_regs.empty()) {
                    auto it_pos = system.begin() + j;
                    system.erase(it_pos);
                }
            } else if (system[j].langs.find(system[i].lang) != system[j].langs.end()){
                auto i_solved = get_solution(system[i]);
                system[j] = substitute(i_solved, system[j]);
            }
//            cout << "-------------------------\n";
//            out_system();
        }
    }
    system[system.size() - 1] = get_solution(system[system.size() - 1]);
    cout << "More readable variation (plus as arithmetic operation):\n\t" << system[system.size() - 1].to_string_optimized() << endl;
    string out_line;

    regex r_plus(R"(([^\+]+)\+([^\+]+))");
    regex r_bracket("\\(([a-z])\\)");
    regex r_brackets(R"(\(\(([^\(]*)\)\))");
    for (auto t : system){
        if (t.lang == "S" && !outputted) {
            string out = t.to_string_without_lang();
            out = regex_replace(out, r_bracket, "$1");
            while(regex_search(out, r_plus)){
                out = regex_replace(out, r_plus, "(($1)|($2))");
            }
            out = regex_replace(out, r_bracket, "$1");
            while(regex_search(out, r_brackets)){
                out = regex_replace(out, r_brackets, "($1)");
            }
            cout << out << endl;
            outputted = true;
        }
    }
}

string delete_spaces(string line) {
    regex r(R"((\s)+)", regex_constants::ECMAScript);
    line = regex_replace(line, r, "");
    return line;
}

pair<string, string> parse(string line, string delim) {
    regex r1(delim + ".+", regex_constants::ECMAScript);
    regex r2(".+" + delim, regex_constants::ECMAScript);
    regex r3("\\|");
    string line_before = regex_replace(line, r1, "");
    string line_after = regex_replace(regex_replace(line, r2, ""), r3, "+");
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
            auto a = parse(delete_spaces(line), "->");
            syst.emplace_back(a.first, a.second);
            cout << "\t" << syst[syst.size() - 1].to_string_optimized() << endl;
        }
        reformat_syst();
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
            outputted = false;
            cout << "TEST " << i + 1 << endl;
            read(i + 1);
            cout << endl;
        }
    }
    return 0;
}

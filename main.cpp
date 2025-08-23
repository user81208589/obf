#include <bits/stdc++.h>
#include <filesystem>
using namespace std;
using u8 = unsigned char;
using u32 = uint32_t;
using u64 = uint64_t;
using str = std::string;
using vi = vector<int>;
using vu8 = vector<u8>;

#define XOR(a,b) ((a)^(b))
#define ROL8(v,r) ( (unsigned char)((((v)<<(r))|(((v)&0xFF)>>(8-(r)))) & 0xFF) )
#define HEX2(x) ( (int)((x)&0xFF) )
#define SSTR(x) ( (std::ostringstream&)(std::ostringstream() << (x) ) , std::string() )

static inline string fname_only(const string &p){
    namespace fs = std::filesystem;
    return fs::path(p).filename().string();
}
static inline string stem_only(const string &p){
    namespace fs = std::filesystem;
    return fs::path(p).filename().stem().string();
}
static inline string ext_only(const string &p){
    namespace fs = std::filesystem;
    return fs::path(p).filename().extension().string();
}
static inline bool exists_local(const string &p){
    namespace fs = std::filesystem;
    return fs::exists(fs::path(p).filename());
}
static inline string read_local(const string &p){
    string n = fname_only(p);
    ifstream f(n, ios::binary);
    if(!f) return {};
    ostringstream ss; ss<<f.rdbuf(); return ss.str();
}
static inline bool write_local(const string &n, const string &c){
    ofstream f(n, ios::binary);
    if(!f) return false;
    f<<c; return true;
}
static inline string out_name(const string &in){
    return stem_only(in) + "_obf" + ext_only(in);
}
static inline string b64enc(const string &in){
    static const char *tbl = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";
    string out; int val=0, valb=-6;
    for(unsigned char c: in){
        val = (val<<8) + c; valb += 8;
        while(valb>=0){ out.push_back(tbl[(val>>valb)&0x3F]); valb -= 6; }
    }
    if(valb>-6) out.push_back(tbl[((val<<8)>>(valb+8))&0x3F]);
    while(out.size()%4) out.push_back('=');
    return out;
}
static inline vi xor_rot_apply(const string &s, int key, int rot){
    vi out; out.reserve(s.size());
    for(unsigned char c: s){
        int t = XOR((int)c, key);
        int r = ((t<<rot) | ((t&0xFF)>>(8-rot))) & 0xFF;
        out.push_back(r);
    }
    return out;
}
static inline string fmt_hex_array(const vi &v){
    ostringstream o; o<<"{";
    for(size_t i=0;i<v.size();++i){
        if(i) o<<",";
        int x = v[i] & 0xFF;
        o<< "0x" << uppercase << hex << setw(2) << setfill('0') << x << dec << nouppercase;
    }
    o<<"}";
    return o.str();
}

static inline bool looks_like_python(const string &name, const string &content){
    string e = ext_only(name);
    if(e==".py"||e==".pyw") return true;
    if(content.size()>2 && content.rfind("#!",0)==0 && content.find("python")!=string::npos) return true;
    return false;
}

static inline string splice_credit_encoded(const string &credit){
    string b = b64enc(credit);
    vector<string> parts;
    size_t n = b.size();
    for(size_t i=0;i<n;i+=6) parts.push_back(b.substr(i, min<size_t>(6, n-i)));
    ostringstream o;
    for(size_t i=0;i<parts.size();++i){
        o << "C" << i << "=\"" << parts[i] << "\";\n";
    }
    o << "string CRD=\"\";\n";
    for(size_t i=0;i<parts.size();++i){
        o << "CRD += C" << i << ";\n";
    }
    o << "auto CRC = []()->string{ return CRD; }();\n";
    o << "/*encoded-credit*/\n";
    return o.str();
}

static inline string inject_credit_py(const string &payload, const string &credit){
    string enc = b64enc(credit);
    ostringstream o;
    o << "_C_P = \"" << enc << "\"\n";
    o << "def _credit():\n";
    o << "    import base64\n";
    o << "    try:\n";
    o << "        s = base64.b64decode(_C_P).decode('utf-8')\n";
    o << "    except Exception:\n";
    o << "        s = ''\n";
    o << "    if False:\n";
    o << "        print(s)\n";
    o << "\n";
    o << payload;
    return o.str();
}

static inline string inject_credit_cpp(const string &payload, const string &credit){
    vi arr; string cenc = b64enc(credit);
    for(unsigned char ch: cenc) arr.push_back((int)ch);
    ostringstream o;
    o << "static const unsigned char __C[] = " << fmt_hex_array(arr) << ";\n";
    o << "static inline string __getcredit(){ string s; s.reserve(sizeof(__C)); for(size_t i=0;i<sizeof(__C)/sizeof(__C[0]); ++i) s.push_back((char)__C[i]); return s; }\n";
    o << "static void __credit_nop(){ if(false){ auto x = __getcredit(); (void)x; } }\n";
    o << payload;
    return o.str();
}

static inline string build_py_obf(const string &src, const string &credit){
    int key=0x5A, rot=3;
    vi arr = xor_rot_apply(src, key, rot);
    ostringstream o;
    o << "__D = " << fmt_hex_array(arr) << "\n";
    o << "_K = " << key << "\n";
    o << "_R = " << rot << "\n";
    o << "def _d(a,k,r):\n";
    o << "    b=[]\n";
    o << "    for x in a:\n";
    o << "        y = (((x >> r) | ((x << (8-r)) & 0xFF)) ^ k)\n";
    o << "        b.append(y)\n";
    o << "    return bytes(b).decode('utf-8',errors='ignore')\n";
    o << "_S=_d(__D,_K,_R)\n";
    o << "exec(compile(_S,'<o>','exec'))\n";
    string with_credit = inject_credit_py(o.str(), credit);
    return with_credit;
}

static inline string build_js_obf(const string &src, const string &credit){
    string enc = b64enc(src);
    ostringstream o;
    o << "/*" << credit << "*/\n";
    o << "eval(atob(\"" << enc << "\"));\n";
    return o.str();
}

static inline string build_bat_obf(const string &src, const string &credit){
    string enc = b64enc(src);
    ostringstream o;
    o << "@echo off\r\n";
    o << "set _C=" << enc << "\r\n";
    o << "powershell -NoProfile -EncodedCommand %_C%\r\n";
    o << "rem " << credit << "\r\n";
    return o.str();
}

static inline string build_java_obf(const string &src, const string &credit){
    string enc = b64enc(src);
    string cls = stem_only("ObfClass");
    ostringstream o;
    o << "class ObfClass{public static void main(String[]a)throws Exception{String s=\"" << enc << "\";byte[]d=java.util.Base64.getDecoder().decode(s);java.nio.file.Files.write(java.nio.file.Paths.get(\"" << cls << ".java\"),d);Process p=Runtime.getRuntime().exec(new String[]{\"javac\",\"" << cls << ".java\"});p.waitFor();Process q=Runtime.getRuntime().exec(new String[]{\"java\",\"" << cls << "\"});q.waitFor();}}\n";
    return o.str();
}

static inline string build_cs_obf(const string &src, const string &credit){
    string enc = b64enc(src);
    string fname = stem_only("ObfCS");
    ostringstream o;
    o << "using System;using System.IO;class Obf{static void Main(){string s=\"" << enc << "\";byte[]b=Convert.FromBase64String(s);File.WriteAllBytes(\"" << fname << ".cs\",b);var p=System.Diagnostics.Process.Start(\"csc\",\"" << fname << ".cs\");p.WaitForExit();var q=System.Diagnostics.Process.Start(\"" << fname << ".exe\");q.WaitForExit();}}\n";
    return o.str();
}

static inline string build_php_obf(const string &src, const string &credit){
    string enc = b64enc(src);
    ostringstream o;
    o << "<?php\n";
    o << "$s = '" << enc << "';\n";
    o << "echo base64_decode($s);\n";
    o << "?>\n";
    return o.str();
}

static inline string build_cpp_obf(const string &src, const string &credit){
    int key=0x37, rot=5;
    vi arr = xor_rot_apply(src, key, rot);
    ostringstream o;
    o << "#include <bits/stdc++.h>\nusing namespace std;\nstatic const unsigned __M[] = " << fmt_hex_array(arr) << ";\nstatic const size_t __N = sizeof(__M)/sizeof(__M[0]);\nstatic inline string __dec(){ string r; r.reserve(__N); for(size_t i=0;i<__N;++i){ unsigned char v=(unsigned char)__M[i]; unsigned char x=(unsigned char)((((v>>" << rot << ")| (v<<(8-" << rot << "))) & 0xFF) ^ " << key << "); r.push_back((char)x);} return r; }\nstruct __R{ static void run(){ string s = __dec(); istringstream iss(s); string L; while(getline(iss,L)) cout<<L<<\"\\n\"; } };\nint main(){ __R::run(); return 0; }\n";
    string with_credit = inject_credit_cpp(o.str(), credit);
    return with_credit;
}

static inline string choose_and_build(const string &name, const string &src, const string &credit){
    string e = ext_only(name);
    if(e==".py"||e==".pyw"||looks_like_python(name, src)) return build_py_obf(src, credit);
    if(e==".js") return build_js_obf(src, credit);
    if(e==".bat") return build_bat_obf(src, credit);
    if(e==".java") return build_java_obf(src, credit);
    if(e==".cs") return build_cs_obf(src, credit);
    if(e==".php") return build_php_obf(src, credit);
    if(e==".cpp"|| e==".cc"|| e==".cxx"|| e==".c") return build_cpp_obf(src, credit);
    return build_py_obf(src, credit);
}

static inline string make_credit_string(const string &toolname, const string &author, const string &when){
    ostringstream o;
    o << "Credit:" << toolname << " Author:" << author << " Date:" << when;
    return o.str();
}

static inline string prepare_credit_fragments(const string &credit){
    string b = b64enc(credit);
    vector<string> parts;
    for(size_t i=0;i<b.size(); i+=5) parts.push_back(b.substr(i, min<size_t>(5, b.size()-i)));
    ostringstream o;
    for(size_t i=0;i<parts.size();++i){
        o << "P" << i << "=\"" << parts[i] << "\";\n";
    }
    o << "CRT=\"\";\n";
    for(size_t i=0;i<parts.size();++i) o << "CRT += P" << i << ";\n";
    o << "/*frag done*/\n";
    return o.str();
}

static inline void do_noop_long(){
    volatile uint64_t z=1469598103934665603ULL;
    for(int i=0;i<256;i++) z = (z ^ (i+1)) * 1099511628211ULL;
    (void)z;
}

static inline string safe_read_trim(const string &name){
    string s = read_local(name);
    if(!s.empty() && s.back()=='\n') s.pop_back();
    return s;
}

static inline string pick_input(int argc, char** argv){
    for(int i=1;i<argc;i++){
        string a = argv[i];
        if(a=="-i" && i+1<argc) return argv[i+1];
    }
    return {};
}

static inline string ensure_local_name(const string &in){
    return fname_only(in);
}

static inline bool tiny_touch(const string &n){
    ofstream f(n, ios::app);
    if(!f) return false;
    f.close();
    return true;
}

static inline string final_write_and_return(const string &out, const string &payload){
    write_local(out, payload);
    return out;
}

int main(int argc, char** argv){
    ios::sync_with_stdio(false);
    cin.tie(nullptr);
    string in = pick_input(argc, argv);
    if(in.empty()) return 0;
    string local = ensure_local_name(in);
    if(!exists_local(local)) return 0;
    string src = safe_read_trim(local);
    if(src.empty()) return 0;
    string tool = "ObfToolX";
    string author = "AuthorName";
    string when = __DATE__;
    string credit = make_credit_string(tool, author, when);
    string out = out_name(in);
    string payload = choose_and_build(in, src, credit);
    final_write_and_return(out, payload);
    do_noop_long();
    return 0;
}

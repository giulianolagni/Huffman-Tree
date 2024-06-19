#include <iostream>
#include <unordered_map>
#include <queue>
#include <string>
#include <vector> 
#include <fstream>
#include <codecvt>
#include <locale>
#include <sstream>

using namespace std;

struct node{
    wchar_t ch;
    int freq;
    node *left;
    node *right;
};

//struct usada na fila de prioridade(priority_queue)
struct comp {
    bool operator()(node* l, node* r) {
        return l->freq > r->freq;
    }
};

//funcao para alocar um novo nodo na memoria
node *create_node(wchar_t c, int frequency, node *left, node *right){

    node *p = new node;
    p->ch = c;
    p->freq = frequency;
    p->left = left;
    p->right = right;

    return p;
}

//função usada para gerar numeros para identificar os nodos
string to_string_pointer(const void* ptr) {
    stringstream ss;
    ss << reinterpret_cast<uintptr_t>(ptr);
    return ss.str();
}

//função usada para converter os caracteres unicode em UTF-8
//isso porque as funcoes usadas em export2dot esperam parametros em UTF-8
string to_utf8(const wstring& wstr) {
    wstring_convert<codecvt_utf8<wchar_t>> conv;
    return conv.to_bytes(wstr);
}

void encode(node *root, wstring str, unordered_map<wchar_t, wstring> &huffcode){

    if(!root) return;

    if(!root->left && !root->right){
        huffcode[root->ch] = str;
    }

    encode(root->left, str + L'0', huffcode);
    encode(root->right, str + L'1', huffcode);
}

void decode(node *root, int &top_index, wstring str){

    if(!root) return;

    if(!root->left && !root->right){
        cout << root->ch;
        return;
    }
    top_index++;

    if(str[top_index] == L'0') decode(root->left, top_index, str);
    else if(str[top_index] == L'1') decode(root->right, top_index, str);
}

void export2dot(const node* root, const std::string& filename, const unordered_map<wchar_t, wstring> &huffcode) {
        ofstream dot(filename);
        dot << "digraph G {\n";
        queue<const node*> q;
        q.push(root);

        while (!q.empty()) {
            const node* current = q.front();
            q.pop();
            string node_name = "node" + to_string(reinterpret_cast<uintptr_t>(current));

            string label;
            bool flag = false;
            if (current->ch == L'\0') {
                label = to_string(current->freq);
                flag = true;
            } else {
                string valuestr;
                if(current->ch == L' ')
                    valuestr = "SPACE";
                else if(current->ch == L'\n')
                    valuestr = "NEWLINE";
                else if(current->ch == L'"')
                    valuestr = "ASPAS";
                else if(current->ch == L'>')
                    valuestr = "maior que";
                else if(current->ch == L'<')
                    valuestr = "menor que";
                else if(current->ch == L'{')
                    valuestr = "abre chave";
                else if(current->ch == L'}')
                    valuestr = "fecha chave"; 
                else if(current->ch == L'|')
                    valuestr = "pipe";              
                else
                    valuestr = to_utf8(wstring(1, current->ch));

                string bin_code = to_utf8(huffcode.at(current->ch));
                label = "{{\' " + valuestr + " \'|" + to_string(current->freq) + "}|" + bin_code + "}";
            }
            
            if(!flag) 
                dot << "\t" << node_name << " [shape=record, label=\"" << label << "\"];\n";
            else
                dot << "\t" << node_name << " [label=\"" << label << "\"];\n";

            if (current->left) {
                string left_node_name = "node" + to_string_pointer(current->left);
                dot << "    " << node_name << " -> " << left_node_name << " [label=\"0\"];\n";
                q.push(current->left);
            }
            if (current->right) {
                string right_node_name = "node" + to_string_pointer(current->right);
                dot << "    " << node_name << " -> " << right_node_name << " [label=\"1\"];\n";
                q.push(current->right);
            }
        }
        dot << "}\n";
    }

    // Desenha a árvore de Huffman usando o Graphviz
void draw(const node* root, const unordered_map<wchar_t, wstring>& huffCode) {
    string dot_filename = "../saida/huffman_tree.dot";
    export2dot(root, dot_filename, huffCode);

    system("dot -Tpng ../saida/huffman_tree.dot -o ../saida/graph.png");
}

void buildHuffTree(wifstream &arq, wofstream &saida){

    if(!arq){
        return;
    }
    
    wchar_t ch;
    unordered_map<wchar_t,  int> freq;
    while(arq.get(ch)){
        freq[ch]++;
    }
    
    int teste = 0; //contador somente para fins de teste 
    saida << L"Frequência de caracteres: \n";
    for(auto ch : freq){ 
        if(ch.first == L' ') saida << L"SPACE: " << ch.second << endl;
        else if(ch.first == L'\n') saida << L"\\n: " << ch.second << endl;
        else saida << ch.first << L": " << ch.second << endl;
        teste += ch.second; //esta inconsistente, ver pq
    }
    saida << teste << endl << endl;

    //priority_queue é uma estrutura muito similar a queue
    //no entanto ela sempre armazena os valores de tal modo
        // que o maior deles sempre fica mais a frente
    priority_queue<node*, vector<node*> , comp> pq; 

    //cria um nodo folha para cada caracter
        // e adiciona na fila de prioridade
    for(auto pair : freq){
        pq.push(create_node(pair.first, pair.second, nullptr, nullptr));
    }

    while(pq.size() != 1){
        //remove os dois elementos com a maior prioridade da fila
        node *left = pq.top();
        pq.pop();
        node *right = pq.top();
        pq.pop();

        //cria um nodo interno na arvore com esses 2 nodos acima como filhos
        // e a frequencia sendo igual a soma das frequencias dos filhos.
        //E adiciona o novo nodo a fila de prioridade;  
        int soma =left->freq + right->freq;
        pq.push(create_node(L'\0', soma, left, right));
    }

    node *root = pq.top();
    unordered_map<wchar_t, wstring> huffCode;
    encode(root, L"", huffCode);
    

    saida << L"Codigos binários da árvore criada: \n";
    for(auto pair : huffCode){
        if(pair.first == L' ') saida << L"SPACE: " << pair.second << endl;
        else if(pair.first == L'\n') saida << L"\\n: " << pair.second << endl;
        else saida << pair.first << L": " << pair.second << endl;
    }

    arq.clear();
    arq.seekg(0, ios::end);
    int original_size = arq.tellg();

    int compressed_size = 0;
    for (auto pair : freq) {
        compressed_size += pair.second * huffCode[pair.first].length();
    }
    compressed_size = (compressed_size + 7) / 8; 

    double compression_ratio = ((double)(original_size - compressed_size) / original_size) * 100;

    saida << L"\n\n\nTamanho original: " << original_size << L" bytes" << endl;
    saida << L"Tamanho compactado: " << compressed_size << L" bytes" << endl;
    saida << L"Redução: " << compression_ratio << L"%" << endl;

    draw(root, huffCode);

}   


//falta:
//      desenhar para windows e linux;
//      separar funçoes em um outro arquivo para melhor organizaçao;
//      fazer uma interface amigavel para inserir qual documento txt quer ler
        // quem sabe até botar uma interface para escoher qual SO o usuario usa
        //ou ver se tem como implementar uma deteccao automatica de SO no codigo fonte.
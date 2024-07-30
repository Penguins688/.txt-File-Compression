#include <iostream>
#include <fstream>
#include <vector>
#include <map>
#include <string>

struct HuffmanNode {
    unsigned char character;
    HuffmanNode* left;
    HuffmanNode* right;
    HuffmanNode() : character(0), left(NULL), right(NULL) {}
};

std::vector<bool> readBits(std::istream& in, int numBits) {
    std::vector<bool> bits;
    unsigned char byte;
    while (bits.size() < numBits) {
        if (in.get(reinterpret_cast<char&>(byte))) {
            for (int i = 7; i >= 0 && bits.size() < numBits; --i) {
                bits.push_back((byte >> i) & 1);
            }
        } else {
            break;
        }
    }
    return bits;
}

HuffmanNode* buildTree(const std::map<unsigned char, std::vector<bool> >& codes) {
    HuffmanNode* root = new HuffmanNode();
    for (std::map<unsigned char, std::vector<bool> >::const_iterator it = codes.begin(); it != codes.end(); ++it) {
        HuffmanNode* node = root;
        for (size_t i = 0; i < it->second.size(); ++i) {
            if (it->second[i] == false) {
                if (!node->left) {
                    node->left = new HuffmanNode();
                }
                node = node->left;
            } else {
                if (!node->right) {
                    node->right = new HuffmanNode();
                }
                node = node->right;
            }
        }
        node->character = it->first;
    }
    return root;
}

void destroyTree(HuffmanNode* node) {
    if (node) {
        destroyTree(node->left);
        destroyTree(node->right);
        delete node;
    }
}

int main() {
    std::string inputFileName;
    std::cout << "Enter the compressed filename: ";
    std::cin >> inputFileName;

    std::ifstream infile(inputFileName.c_str(), std::ios::binary);
    if (!infile.is_open()) {
        std::cerr << "Error: Could not open input file.\n";
        return 1;
    }

    unsigned char uniqueChars;
    infile.read(reinterpret_cast<char*>(&uniqueChars), sizeof(uniqueChars));

    std::map<unsigned char, std::vector<bool> > codes;
    for (int i = 0; i < uniqueChars; ++i) {
        unsigned char character;
        unsigned char codeLength;
        infile.read(reinterpret_cast<char*>(&character), sizeof(character));
        infile.read(reinterpret_cast<char*>(&codeLength), sizeof(codeLength));
        std::vector<bool> code = readBits(infile, codeLength);
        codes[character] = code;
    }

    HuffmanNode* root = buildTree(codes);

    std::string outputFileName = inputFileName + ".decoded";
    std::ofstream outfile(outputFileName.c_str(), std::ios::binary);
    if (!outfile.is_open()) {
        std::cerr << "Error: Could not create output file.\n";
        destroyTree(root);
        return 1;
    }

    HuffmanNode* node = root;
    unsigned char byte;
    while (infile.get(reinterpret_cast<char&>(byte))) {
        for (int i = 7; i >= 0; --i) {
            bool bit = (byte >> i) & 1;
            if (bit) {
                node = node->right;
            } else {
                node = node->left;
            }

            if (node->left == NULL && node->right == NULL) {
                outfile.put(node->character);
                node = root;
            }
        }
    }

    infile.close();
    outfile.close();
    destroyTree(root);

    std::cout << "Decompression complete. Output written to " << outputFileName << std::endl;

    return 0;
}
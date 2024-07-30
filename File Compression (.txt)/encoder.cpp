#include <iostream>
#include <fstream>
#include <vector>
#include <queue>
#include <map>
#include <string>
#include <bitset>

class HuffmanNode {
public:
    unsigned char character;
    int frequency;
    HuffmanNode* left;
    HuffmanNode* right;
    HuffmanNode(unsigned char character, int frequency) : character(character), frequency(frequency), left(NULL), right(NULL) {}
    HuffmanNode(int frequency, HuffmanNode* left, HuffmanNode* right) 
        : character(0), frequency(frequency), left(left), right(right) {}
};

class Compare {
public:
    bool operator()(const HuffmanNode* left, const HuffmanNode* right) const {
        return left->frequency > right->frequency;
    }
};

class HuffmanTree {
public:
    HuffmanNode* root;
    HuffmanTree(const std::vector<std::pair<unsigned char, int> >& frequencies) {
        std::priority_queue<HuffmanNode*, std::vector<HuffmanNode*>, Compare> minHeap;
        for (size_t i = 0; i < frequencies.size(); ++i) {
            minHeap.push(new HuffmanNode(frequencies[i].first, frequencies[i].second));
        }
        while (minHeap.size() > 1) {
            HuffmanNode* left = minHeap.top();
            minHeap.pop();
            HuffmanNode* right = minHeap.top();
            minHeap.pop();

            int sum = left->frequency + right->frequency;
            HuffmanNode* node = new HuffmanNode(sum, left, right);
            minHeap.push(node);
        }
        root = minHeap.top();
    }

    ~HuffmanTree() {
        destroyTree(root);
    }

    std::map<unsigned char, std::vector<bool> > generateCodes() {
        std::map<unsigned char, std::vector<bool> > codes;
        std::vector<bool> code;
        generateCodesRecursive(root, code, codes);
        return codes;
    }

private:
    void generateCodesRecursive(const HuffmanNode* node, std::vector<bool>& code, std::map<unsigned char, std::vector<bool> >& codes) {
        if (node == NULL) {
            return;
        }
        if (node->left == NULL && node->right == NULL) {
            codes[node->character] = code;
            return;
        }
        code.push_back(false);
        generateCodesRecursive(node->left, code, codes);
        code.back() = true;
        generateCodesRecursive(node->right, code, codes);
        code.pop_back();
    }

    void destroyTree(HuffmanNode* node) {
        if (node != NULL) {
            destroyTree(node->left);
            destroyTree(node->right);
            delete node;
        }
    }
};

std::vector<int> analyzeFrequency(const std::vector<unsigned char>& data) {
    std::vector<int> frequency(256, 0);
    for (size_t i = 0; i < data.size(); ++i) {
        frequency[data[i]]++;
    }
    return frequency;
}

std::vector<bool> compressData(const std::vector<unsigned char>& data, const std::map<unsigned char, std::vector<bool> >& codes) {
    std::vector<bool> compressed;
    for (size_t i = 0; i < data.size(); ++i) {
        const std::vector<bool>& code = codes.find(data[i])->second;
        compressed.insert(compressed.end(), code.begin(), code.end());
    }
    return compressed;
}

void writeBinary(std::ostream& out, const std::vector<bool>& bits) {
    unsigned char byte = 0;
    int bitCount = 0;

    for (size_t i = 0; i < bits.size(); ++i) {
        byte = (byte << 1) | bits[i];
        ++bitCount;

        if (bitCount == 8) {
            out.put(byte);
            byte = 0;
            bitCount = 0;
        }
    }

    if (bitCount > 0) {
        byte <<= (8 - bitCount);
        out.put(byte);
    }
}

std::string removeExtension(const std::string& filename) {
    size_t dotPos = filename.find_last_of('.');
    if (dotPos == std::string::npos) {
        return filename;
    }
    return filename.substr(0, dotPos);
}

int main() {
    std::string fileName;
    std::cout << "Enter the filename: ";
    std::cin >> fileName;

    std::ifstream infile(fileName.c_str(), std::ios::binary);
    if (!infile.is_open()) {
        std::cerr << "Error: Could not open file.\n";
        return 1;
    }

    std::vector<unsigned char> data((std::istreambuf_iterator<char>(infile)), std::istreambuf_iterator<char>());
    infile.close();

    std::vector<int> freq = analyzeFrequency(data);
    std::vector<std::pair<unsigned char, int> > frequencies;

    for (int i = 0; i < 256; ++i) {
        if (freq[i] > 0) {
            frequencies.push_back(std::make_pair(static_cast<unsigned char>(i), freq[i]));
        }
    }

    HuffmanTree huffmanTree(frequencies);
    std::map<unsigned char, std::vector<bool> > codes = huffmanTree.generateCodes();

    std::vector<bool> compressedData = compressData(data, codes);

    std::string outputFileName = removeExtension(fileName) + "_compressed.txt";
    std::ofstream outfile(outputFileName.c_str(), std::ios::binary);
    if (!outfile.is_open()) {
        std::cerr << "Error: Could not create output file.\n";
        return 1;
    }

    unsigned char uniqueChars = static_cast<unsigned char>(frequencies.size());
    outfile.write(reinterpret_cast<char*>(&uniqueChars), sizeof(uniqueChars));

    for (std::map<unsigned char, std::vector<bool> >::const_iterator it = codes.begin(); it != codes.end(); ++it) {
        outfile.put(it->first);
        unsigned char codeLength = static_cast<unsigned char>(it->second.size());
        outfile.write(reinterpret_cast<char*>(&codeLength), sizeof(codeLength));
        writeBinary(outfile, it->second);
    }

    writeBinary(outfile, compressedData);

    outfile.close();
    std::cout << "Compression complete. Output written to " << outputFileName << std::endl;

    return 0;
}
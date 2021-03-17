//
// Created by test on 01.03.2021.
//

#ifndef LZWV2_LZWCOMPRESSSERVICE_H
#define LZWV2_LZWCOMPRESSSERVICE_H

#include <bitset>
#include <vector>
#include <unordered_map>
#include <istream>
#include <bit>

class TrieNode {
public:
    uint32_t code;
    std::unordered_map<uint8_t, TrieNode> children;
};


class LzwCompressService {
    TrieNode *rootNode;
    std::vector<std::pair<uint32_t, uint8_t>> *tempOut;

    void initializeDictionary() {
        rootNode->code = 0;
        for (int i = 0x00; i <= 0xFF; i++) {
            rootNode->children[i].code = i;
            rootNode->children[i].children.clear();
        }
    }

    uint8_t getBitsToRepresentInteger(uint32_t x) {
        return std::numeric_limits<uint32_t>::digits - std::countl_zero(x);
    }

    void writeBits(std::vector<uint8_t> &out) {
        uint8_t usedBits = 0;
        uint8_t currentByte = 0;

        for (const auto &x: *tempOut) {
            int8_t bitsToWrite = x.second;
            uint32_t code = x.first;
#ifdef _DEBUG
            std::cout << "\u001b[32m current_code: " << x.first << " current_byte: " << std::bitset<8>(currentByte) << "\u001b[0m" << std::endl;
#endif
            while(bitsToWrite > 0) {
                uint8_t canWriteToByte = 8 - usedBits;
#ifdef _DEBUG
                std :: cout << "canWriteToByte: " << (int)canWriteToByte << std::endl;
                std :: cout << "cb: " << std::bitset<8>(code << usedBits) << std::endl;
#endif
                currentByte |= code << usedBits;
                usedBits = bitsToWrite > canWriteToByte ? 0 : bitsToWrite + usedBits;
                code >>= canWriteToByte;
#ifdef _DEBUG
                std::cout << "usedBits: " << (int)usedBits << std::endl;
#endif
                if (usedBits == 0) {
#ifdef _DEBUG
                    std::cout << "out_byte: " << std::bitset<8> (currentByte) << ", char: " << std::hex << (int)currentByte << std::dec << std::endl;
#endif
                    out.push_back(currentByte);
                    currentByte = 0;
                }
                bitsToWrite -= canWriteToByte;
            }
        }
        if (usedBits != 0) {
#ifdef _DEBUG
            std::cout << "currentByte != 0: " << std::bitset<8> (currentByte) << ", char: " << std::hex << (int)currentByte << std::dec << std::endl;
#endif
            out.push_back(currentByte);
        }
    }

    int getDictSize(TrieNode *node = nullptr) {
        if (node == nullptr) {
            node = rootNode;
        }

        int sum = 1;
        for(auto &x: node->children) {
            sum += getDictSize(&x.second);
        }

        return sum;
    }

    // writes to internal tempOut
    void compressInternal(const std::vector<uint8_t>& inputFile) {
        initializeDictionary();

        std::vector<uint8_t> currentSubsequence;
        int nextIndex = 0;
        uint8_t nextByte;
        TrieNode *currentNode = rootNode;

        int code = 0xFF + 1;

        while (nextIndex < inputFile.size()) {
            nextByte = inputFile[nextIndex];
            if (currentNode->children.contains(nextByte)) {
                currentNode = &currentNode->children[nextByte];
                nextIndex++;
            } else {
                tempOut->emplace_back(currentNode->code, getBitsToRepresentInteger(code));
                currentNode->children[nextByte].code = code;
                code++;
                currentNode = rootNode;
            }
        }

        if (currentNode != rootNode) {
            tempOut->emplace_back(currentNode->code, getBitsToRepresentInteger(code));
        }

        std::cout << "dict size: " << getDictSize() << std::endl;
    }

public:
    void compress(const std::vector<uint8_t>& inputFile, std::vector<uint8_t>& outputFile) {
        compressInternal(inputFile);
    #ifdef _DEBUG
        for (int i = 0; i < tempOut->size(); i++) {
            std::cout << "tempOut[" << i << "] = (" << (*tempOut)[i].first << ", " << (uint32_t)(*tempOut)[i].second << ")" << std::endl;
        }
#endif
        writeBits(outputFile);
    }

    LzwCompressService() {
        rootNode = new TrieNode();
        tempOut = new std::vector<std::pair<uint32_t, uint8_t>>();
    }

    ~LzwCompressService() {
        delete rootNode;
        delete tempOut;
    }
};


#endif //LZWV2_LZWCOMPRESSSERVICE_H

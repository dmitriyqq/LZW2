//
// Created by test on 01.03.2021.
//

#ifndef LZWV2_LZWCOMPRESSSERVICE_H
#define LZWV2_LZWCOMPRESSSERVICE_H

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
        for (int i = 0x00; i < 0xFF; i++) {
            rootNode->children[i].code = i;
            rootNode->children[i].children.clear();
        }
    }

    uint8_t getBitsToRepresentInteger(uint32_t x) {
//        std::cout << "getBitsToRepresentInteger(" << x << ") = " << (std::numeric_limits<uint32_t>::digits - std::countl_zero(x)) << std::endl;
        return std::numeric_limits<uint32_t>::digits - std::countl_zero(x);
    }

    void writeToInt(uint32_t &a, uint32_t &b, uint32_t x, uint8_t &usedBits, uint8_t numberOfBits) {
        constexpr uint8_t bitsInInt = 32;
        if (usedBits + numberOfBits <= bitsInInt) {
            // can safely write x to a;
            a |= x << usedBits;
            usedBits += numberOfBits;
        } else {
            // we can't write whole x, we need to split it between a and b
            char bitsAvailable = bitsInInt - usedBits;
            a |= x << usedBits;
            b |= x >> bitsAvailable;
            usedBits = numberOfBits - bitsAvailable;
        }
    }

    void writeBits(std::vector<uint8_t> &out) {
        uint32_t number = 0;
        uint32_t a = 0, b = 0;
        uint8_t usedBits = 0;

        for (const auto &x: *tempOut) {
            writeToInt(a, b, x.first, usedBits, x.second);
            if (usedBits == 32) {
                // we have whole 4 bytes to write then reset everything
                for (int i = 0; i < 4; i++) {
                    uint8_t byte = a >> (8 * i);
                    out.push_back(byte);
                }

                a = b = usedBits = 0;
            } else if (b != 0) {
                // we have 4 bytes ready to write then move b -> a
                for (int i = 0; i < 4; i++) {
                    uint8_t byte = a >> (8 * i);
                    out.push_back(byte);
                }
                a = b;
                b = 0;
            }
        }
        if (a != 0) {

            std::cout << "a != 0" << std::endl;

            for (int i = 0; i < 4; i++) {
                uint8_t byte = a >> (8 * i);
                out.push_back(byte);
            }
        }
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
    }

public:
    void compress(const std::vector<uint8_t>& inputFile, std::vector<uint8_t>& outputFile) {
        compressInternal(inputFile);
//        for (int i = 0; i < tempOut->size(); i++) {
//            std::cout << "tempOut[" << i << "] = (" << (*tempOut)[i].first << ", " << (uint32_t)(*tempOut)[i].second << ")" << std::endl;
//        }
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

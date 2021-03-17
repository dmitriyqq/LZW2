//
// Created by test on 03.03.2021.
//

#ifndef LZWV2_LZWDECOMPRESSSERVICE_H
#define LZWV2_LZWDECOMPRESSSERVICE_H

#include <vector>
#include <algorithm>
#include <iostream>
#include <bit>
#include <bitset>

class LzwDecompressService {
    uint8_t getBitsToRepresentInteger(uint32_t x) {
#ifdef _DEBUG
        std::cout << "getBitsToRepresentInteger(" << x << ") = " << (std::numeric_limits<uint32_t>::digits - std::countl_zero(x)) << std::endl;
#endif
        return std::numeric_limits<uint32_t>::digits - std::countl_zero(x);
    }

    uint32_t readCode(
        const std::vector<uint8_t>& inputFile,
        int &bytesRead,
        uint8_t &bitsRead,
        uint8_t bitsToRead
    ) {
        uint32_t code = 0;
#ifdef _DEBUG
        std::cout << "reading code of size: " << (int)(bitsToRead) << " with offset: " << (int)bitsRead << std::endl;
#endif
        uint8_t bitsReadTotal = 0;

        while (bitsToRead > 0) {
            uint8_t currentByte = inputFile[bytesRead];
            uint8_t bitsCanReadFromCurrentByte = 8 - bitsRead;
            uint8_t bitsNeedToRead = std::min(bitsToRead, bitsCanReadFromCurrentByte);
            uint8_t bitsDontNeedToRead = 8 - bitsNeedToRead - bitsRead;
#ifdef _DEBUG
            std::cout << "bytesRead: " << bytesRead << std::endl;
            std::cout << "currentByte: " << std::bitset<8> (currentByte) << std::endl;
            std::cout << "bitsCanReadFromCurrentByte: " << (int) bitsCanReadFromCurrentByte << std::endl;
            std::cout << "bitsNeedToRead: " << (int) bitsNeedToRead << std::endl;
            std::cout << "bitsDontNeedToRead: " << (int) bitsDontNeedToRead << std::endl;
            std::cout << "bitsRead: " << (int) bitsRead << std::endl;
#endif
            uint8_t strippedDontNeedBits = currentByte << bitsDontNeedToRead;
#ifdef _DEBUG
            std::cout << "strippedDontNeedBits: " << std::bitset<8> (strippedDontNeedBits) << std::endl;
#endif
            uint8_t newCodePart = (strippedDontNeedBits) >> (bitsDontNeedToRead + bitsRead);
#ifdef _DEBUG
            std::cout << "newCodePart: " << std::bitset<8> (newCodePart) << std::endl;
#endif

            code |= ((uint32_t)newCodePart) << bitsReadTotal;
#ifdef _DEBUG
            std::cout  << std::bitset<32> (code) << " int: " << code <<  std::endl;
#endif
            bitsReadTotal += bitsNeedToRead;
            bitsRead = (8 - bitsDontNeedToRead) % 8;

            bitsToRead -= std::min(bitsCanReadFromCurrentByte, bitsToRead);

            if (bitsDontNeedToRead == 0) {
                bytesRead++;
            }
        }
#ifdef _DEBUG
        std::cout << "\u001b[31mcode: " << code << " \u001b[0m" << std::endl;
#endif
        return code;
    }


    std::string vecToStr(const std::vector<uint8_t>& vec) {
        std::string str;
        for(unsigned char i : vec) {
            str.push_back(i);
        }
        return str;
    }

    void printDict(const std::vector<std::vector<uint8_t>>& dict) {
        std::cout << "\u001b[35m" << std::endl;
        for(int i = 255; i < dict.size(); i++) {
            std::cout << "dict: " << i << " : " << vecToStr(dict[i]) << std::endl;
        }
        std::cout << "\u001b[0m" << std::endl;
    }

public:
    void decompress(const std::vector<uint8_t>& inputFile, std::vector<uint8_t>& outputFile) {
        std::vector<std::vector<uint8_t>> dict(256,  std::vector<uint8_t>());
        for (int i = 0x00; i <= 0xFF; i++) {
            dict[i].push_back(i);
        }

        int coder = 0xFF + 1;
        std::vector<uint8_t> current;

        int bytesRead = 0;
        uint8_t bitsRead = 0;
        int bitsToRead = getBitsToRepresentInteger(coder);
        while((bytesRead + (bitsToRead/8 + std::min(bitsToRead%8, 1)) - 1) < inputFile.size()) {
            bitsToRead = getBitsToRepresentInteger(coder);
            uint32_t code = readCode(inputFile, bytesRead, bitsRead, bitsToRead);

            for (const auto&x : dict[code]) {
                current.push_back(x);
            }

            for (const auto&x : current) {
                outputFile.push_back(x);
            }
#ifdef _DEBUG
            std::cout << "\u001b[34m" << "dict.push_back(" << vecToStr(current) << ")" << "\u001b[0m" << std::endl;
#endif
            dict.push_back(current);
#ifdef _DEBUG
//            printDict(dict);
#endif

            bitsToRead = getBitsToRepresentInteger(coder);
#ifdef _DEBUG
            std::cout << "input size: " << inputFile.size() << " bytesRead: " << bytesRead << " bitsToRead: " << bitsToRead << std::endl;
#endif
            if ((bytesRead + (bitsToRead/8 + std::min(bitsToRead%8, 1)) - 1) < inputFile.size()) {

                int tmpBytesRead = bytesRead;
                uint8_t tmpBitsRead = bitsRead;

                uint32_t code2 = readCode(inputFile, tmpBytesRead, tmpBitsRead, bitsToRead);
#ifdef _DEBUG
                std::cout << "read code2: " << code << std::endl;
                std::cout << "\u001b[34m" << "dict[" << coder << "].push_back(" << dict[code2][0] << ") " << "\u001b[0m" << std::endl;
                std::cout << "\u001b[34m" << "code2: " << code2 << ", dict[" << code2 << "] = " << vecToStr(dict[code2]) << "\u001b[0m" << std::endl;
#endif
                dict[coder].push_back(dict[code2][0]);
            }

            coder++;
            current.clear();
        }
    }
};

#endif //LZWV2_LZWDECOMPRESSSERVICE_H

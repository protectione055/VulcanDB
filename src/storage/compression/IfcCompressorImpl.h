// Copyright 2023 Tsinghua University & VulcanDB

#pragma once
#include <stdio.h>
#include <time.h>

#include <fstream>
#include <iostream>
#include <map>
#include <set>
#include <sstream>
#include <string>
#include <vector>

#include "CompressorInterface.h"

using namespace std;

namespace vulcan {

struct IfcCompressorArg {
  float fpr = 0.0;
};

class IfcCompressorImpl : public CompressorInterface {
 public:
  IfcCompressorImpl() = default;
  ~IfcCompressorImpl() = default;

  void compress(const std::string &input_filename,
                const std::string &output_filename, const void *comp_arg) {
    int max_presicion = MAX_PRECISION;
    bool fpr = false;
    if (comp_arg != nullptr) {
      auto arg = static_cast<const IfcCompressorArg *>(comp_arg);
      fpr = arg->fpr;
    }
    IfcCompressor(input_filename, output_filename, 0, fpr, true);
  }

  // This is the function that we want to fuzz,
  // slince the content-based compression can not be decomrpessed.
  void decompress(const std::string &input_filename,
                  const std::string &output_filename) {
    return;
  }

 private:
  // Initial Implement of IfcCompressor, don't want to refactor the code since
  // it can work
  int MAX_PRECISION = 2;
  bool FPR = false;

  struct IfcInstance {
    int id;
    string type;
    string content;
    bool flag;
  };

  struct MatchSet {
    vector<string> baseInstance;
    vector<int> index;
    map<int, int> id_baseID;
    map<string, map<string, int>> type_content;
  };

  struct UnmatchSet {
    vector<IfcInstance> instance;
    vector<bool> flag;
    int totalCount;
  };

  void IfcCompressor(string fileName, string fileName4Write, int flag = 0,
                     int precision = 2, bool fpr = false);
  void IfcCompressor(string fileName, string fileName4Write, char noCSV,
                     int flag = 0, int precision = 2, bool fpr = false);

  void IfcCompressor(stringstream &textContent, string fileName4Write,
                     int flag = 0);
  void IfcCompressorAna(stringstream &ifile, stringstream &result,
                        int &totalCount, int &compCount);
  void IfcCompare(string fileA, string fileB, float &sA2B, float &sB2A,
                  float &machR, float &missR, float &guidPR, float &AR);

  void IfcInstanceAna(
      string instance, int &id, string &type, string &content,
      vector<int> &cite);  //返回的content为所有非引用，引用部分为空
  int IfcInstanceAna(string instance, string &type, string &content,
                     vector<int> &cite);  //返回的content包含引用
  string StandardContent(string type, string &content, vector<int> &cite,
                         bool islossy);
  bool StandardContent(string &content, map<int, int> &rel);
  void trim(string &str);
};

}  // namespace vulcan

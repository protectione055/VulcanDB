

#include "experiments/compression_benchmark/deflate_test_runner.h"

#include <assert.h>

#include <exception>
#include <fstream>
#include <iostream>
#include <sstream>
#include <string>

namespace compbench {

void Deflate::compress(const std::string& input_filename,
                       const std::string& output_filename, int level,
                       int64_t chunck_size) {
  int ret, flush;
  unsigned have;
  z_stream strm;
  unsigned char in[chunck_size];
  unsigned char out[chunck_size];
  FILE* source = fopen(input_filename.c_str(), "r");
  FILE* dest = fopen(output_filename.c_str(), "w");

  strm.zalloc = Z_NULL;
  strm.zfree = Z_NULL;
  strm.opaque = Z_NULL;
  ret = deflateInit(&strm, level);
  if (ret != Z_OK) {
    std::cerr << "Error initializing zlib: (" << ret << ") " << strm.msg
              << std::endl;
    exit(1);
  }

  do {
    strm.avail_in = fread(in, 1, chunck_size, source);
    if (ferror(source)) {
      (void)deflateEnd(&strm);
      std::cerr << "Error reading file" << std::endl;
      exit(1);
    }
    flush = feof(source) ? Z_FINISH : Z_NO_FLUSH;
    strm.next_in = in;
    do {
      strm.avail_out = chunck_size;
      strm.next_out = out;
      ret = deflate(&strm, flush);
      assert(ret != Z_STREAM_ERROR);
      have = chunck_size - strm.avail_out;
      if (fwrite(out, 1, have, dest) != have || ferror(dest)) {
        (void)deflateEnd(&strm);
        std::cerr << "Error writing file" << std::endl;
        exit(1);
      }
    } while (strm.avail_out == 0);
    assert(strm.avail_in == 0);
  } while (flush != Z_FINISH);
  assert(ret == Z_STREAM_END);

  (void)deflateEnd(&strm);
  fclose(source);
  fclose(dest);
}

void DeflateTestRunner::setup(std::string workding_dir, std::string data_dir) {
  std::filesystem::path working_dir_path(workding_dir);
  working_dir_path /= "deflate";
  working_dir_ = working_dir_path;

  std::filesystem::path data_dir_path(data_dir);
  data_dir_ = data_dir_path;
}

void DeflateTestRunner::run() {
  for (int level : compress_levels_) {
    for (int64_t chunk_size : chunk_sizes_) {
      ZipConverter zip_task(level, chunk_size);
      try {
        std::filesystem::path result_dir =
            working_dir_ / zip_task.method_name();
        traverseDir(data_dir_, result_dir, ".ifc", ".ifcZIP", zip_task);
      } catch (const std::exception& e) {
        std::cerr << "[EXCEPTION] " << e.what() << std::endl;
      }
    }
  }
}

void DeflateTestRunner::teardown() {}

}  // namespace compbench

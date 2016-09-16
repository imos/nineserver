#include <algorithm>
#include <string>
#include <vector>

#include "base/base.h"

DEFINE_string(namespace, "", "Namespace.");

string FileGetContents(const string& file_name) {
  string result;
  FILE* fp = fopen(file_name.c_str(), "r");
  CHECK(fp != nullptr) << "Failed to read " << file_name;
  while (true) {
    char buffer[1024];
    int length = fread(buffer, 1, sizeof(buffer), fp);
    result.append(buffer, length);
    if (length <= 0) break;
  }
  fclose(fp);
  return result;
}

void Escape(const string& data) {
  vector<string> buffer;
  buffer.push_back("");
  bool last_is_hex = false;
  for (unsigned char c : data) {
    string output;
    switch (c) {
      case '\n': output = "\\n"; break;
      case '\r': output = "\\r"; break;
      case '"': output = "\\\""; break;
      case '\\': output = "\\\\"; break;
      case '?': output = "\\?"; break;
      default:
        if (isgraph(c) && !(isxdigit(c) && last_is_hex)) {
          output = string(1, c);
        } else {
          output = StringPrintf("\\x%02x", c);
          last_is_hex = true;
        }
        break;
    }
    if (buffer.back().size() + output.size() > 60) {
      buffer.push_back("");
    }
    buffer.back().append(output);
  }
  printf("StringPiece(\n");
  for (const string& line : buffer) {
    printf("    \"%s\"\n", line.c_str());
  }
  printf("    , %ld)\n", data.size());
}

int main(int argc, char **argv) {
  ParseCommandLineFlags(&argc, &argv);
  vector<string> files;
  for (int i = 1; i < argc; i++) {
    files.push_back(argv[i]);
  }
  std::sort(files.begin(), files.end());
  int common_length = 0;
  for (; common_length < files[0].size(); common_length++) {
    if (files[0][common_length] != files.back()[common_length]) break;
  }
  puts("#include \"nineserver/handler/resource_pool.h\"");
  puts("namespace {\n");
  puts("map<StringPiece, StringPiece> GetResourceData() {");
  puts("  return map<StringPiece, StringPiece>({");
  for (string& file : files) {
    puts("{");
    Escape(file.substr(common_length, file.size() - common_length));
    puts(",");
    Escape(FileGetContents(file));
    puts("},");
  }
  puts("});");
  puts("}");
  puts("}  // namespace");
  puts("REGISTER_RESOURCE(GetResourceData());");
  return 0;
}

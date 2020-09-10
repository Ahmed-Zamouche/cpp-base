#include <experimental/filesystem>
#include <fstream>
#include <iostream>
#include <iterator>
#include <memory>
#include <random>
#include <sstream>
#include <string>

#if __has_include(<format>)
#include <format>
using std::format;
#else
#include <fmt/format.h>
using fmt::format;
#endif

#include <google/protobuf/compiler/code_generator.h>
#include <google/protobuf/compiler/plugin.h>
#include <google/protobuf/descriptor.h>
#include <google/protobuf/io/printer.h>
#include <google/protobuf/io/zero_copy_stream.h>

#include <mstch/mstch.hpp>

#include "common/Assert.h"

using namespace google::protobuf;
using namespace google::protobuf::io;
using namespace google::protobuf::compiler;

namespace fs = std::experimental::filesystem;

class MyCodeGenerator : public CodeGenerator {

  io::ZeroCopyOutputStream *
  createZeroCopyOutputStream(const FileDescriptor *file,
                             GeneratorContext *generator_context,
                             const std::string &path, const std::string &name,
                             const std::string &suffix) const {
    string filename = file->package() + "/" + name + suffix;

    if (path.length() > 0) {
      filename = path + "/" + filename;
    }

    io::ZeroCopyOutputStream *output = generator_context->Open(filename);
    io::Printer printer(output, '$');

    //  const FileDescriptor* dependencyDescriptor;
    //  string includeName = "";
    //  for (int i = 0; i < file->dependency_count();i++)
    //  {
    //    dependencyDescriptor = file->dependency(i);
    //    includeName = dependencyDescriptor->name();
    //    replace(includeName.begin(), includeName.end(), '.', '/');
    //    printer.Print("#inlcude \"$name$.h\"\n", "name", includeName);
    //  }

    return output;
  }
  string getTemplate(const string &filePath) const {

    if (!fs::exists(fs::path{filePath})) {
      ASSERT(false, ("template file not found: " + filePath).c_str());
    }

    std::ifstream ifs(filePath, std::ifstream::in);

    return std::string((std::istreambuf_iterator<char>(ifs)),
                       (std::istreambuf_iterator<char>()));
  }
  std::string getGUID() const {

    std::random_device rd;
    std::mt19937 gnr(rd());
    std::uniform_int_distribution<std::uint16_t> dist(
        0, std::numeric_limits<std::uint16_t>::max());

    std::stringstream guid;

    guid << format("{:04x}{:04x}_{:04x}_{:04x}_{:04x}_{:04x}{:04x}{:04x}",
                   dist(gnr), dist(gnr), dist(gnr), dist(gnr), dist(gnr),
                   dist(gnr), dist(gnr), dist(gnr));

    return guid.str();
  }

  std::string getScope(FileDescriptor *file, Descriptor *message) const {
    std::string scope = "";
    while (message) {
      scope = scope + message->name() + "::";
      file = const_cast<FileDescriptor *>(message->file());
      message = const_cast<Descriptor *>(message->containing_type());
    }
    scope = file->package() + "::" + scope;
    return scope;
  }

  mstch::map getEnumValue(const EnumValueDescriptor &value) const {
    mstch::map map = {
        {"name", value.name()},
        {"number", std::to_string(value.number())},
        {"debug_string", value.DebugString()},
    };
    return map;
  }

  mstch::array getEnumValues(const EnumDescriptor &_enum) const {
    mstch::array arr;
    for (int i = 0; i < _enum.value_count(); i++) {
      mstch::map map = {{"value", getEnumValue(*_enum.value(i))}};
      arr.push_back(map);
    }
    return arr;
  }

  mstch::map getEnum(const std::string &scope,
                     const EnumDescriptor &_enum) const {
    mstch::map map = {
        {"scope", scope},
        {"name", _enum.name()},
        {"values", getEnumValues(_enum)},
        {"debug_string", _enum.DebugString()},
    };
    return map;
  }

  mstch::array getEnums(const std::string &scope,
                        const Descriptor &klass) const {
    mstch::array arr;
    for (int i = 0; i < klass.enum_type_count(); i++) {
      mstch::map map = {{"enum", getEnum(scope, *klass.enum_type(i))}};
      arr.push_back(map);
    }
    return arr;
  }

  mstch::map getProperty(const FieldDescriptor &property) const {

    auto capitalize = [](const std::string &s) -> std::string {
      auto r = s;
      r[0] = std::toupper(r[0]);
      return r;
    };
    mstch::map map = {
        {"name", capitalize(property.name())},
        {"debug_string", property.DebugString()},
    };

    switch (property.cpp_type()) {
    case FieldDescriptor::CPPTYPE_INT32:
      map["type"] = std::string("int32_t");
      map["scope"] = std::string("std::");
      break;
    case FieldDescriptor::CPPTYPE_INT64:
      map["type"] = std::string("int64_t");
      map["scope"] = std::string("std::");
      break;
    case FieldDescriptor::CPPTYPE_UINT32:
      map["type"] = std::string("uint32_t");
      map["scope"] = std::string("std::");
      break;
    case FieldDescriptor::CPPTYPE_UINT64:
      map["type"] = std::string("uint64_t");
      map["scope"] = std::string("std::");
      break;
    case FieldDescriptor::CPPTYPE_DOUBLE:
      map["type"] = std::string("double");
      break;
    case FieldDescriptor::CPPTYPE_FLOAT:
      map["type"] = std::string("float");
      break;
    case FieldDescriptor::CPPTYPE_BOOL:
      map["type"] = std::string("bool");
      break;
    case FieldDescriptor::CPPTYPE_ENUM:
      map["type"] = property.enum_type()->name();
      map["scope"] = getScope(
          const_cast<FileDescriptor *>(property.enum_type()->file()),
          const_cast<Descriptor *>(property.enum_type()->containing_type()));
      break;
    case FieldDescriptor::CPPTYPE_STRING:
      map["type"] = std::string("std::string");
      break;
    case FieldDescriptor::CPPTYPE_MESSAGE:
      map["type"] = property.message_type()->name();
      map["include"] = std::string("true");
      map["scope"] = getScope(
          const_cast<FileDescriptor *>(property.message_type()->file()),
          const_cast<Descriptor *>(property.message_type()->containing_type()));
      break;
    default:
      ASSERT(false, "Unkown Type");
      break;
    }

    return map;
  }

  mstch::array getProperties(const Descriptor &descriptor) const {
    mstch::array arr;
    for (int i = 0; i < descriptor.field_count(); i++) {
      mstch::map map = {{"property", getProperty(*descriptor.field(i))}};
      arr.push_back(map);
    }
    return arr;
  }

  mstch::map generateClass(std::string &hppCode, std::string &cppCode,
                           const std::string &package, const Descriptor &klass,
                           int level = 0) const {
    (void)package;
    (void)level;

    mstch::map map = {
        {"name", klass.name()},
        {"properties", getProperties(klass)},
        {"debug_string", klass.DebugString()},
    };

    hppCode.append(
        mstch::render(getTemplate("tmplt/class/hpp/head.mustache"), map));
    cppCode.append(
        mstch::render(getTemplate("tmplt/class/cpp/head.mustache"), map));

    const mstch::array enums = getEnums(klass.name() + "::", klass);

    /*mstch::array classes;
    for (int i = 0; i < klass.nested_type_count(); i++) {
      const Descriptor &nested = *klass.nested_type(i);
      classes.push_back(getClass(hppCode, hppCode, nested, level + 1));
    }*/

    for (const auto &_enum : enums) {
      hppCode.append(
          mstch::render(getTemplate("tmplt/enum/hpp/body.mustache"), _enum));
      cppCode.append(
          mstch::render(getTemplate("tmplt/enum/cpp/body.mustache"), _enum));
    }

    hppCode.append(
        mstch::render(getTemplate("tmplt/class/hpp/body.mustache"), map));
    cppCode.append(
        mstch::render(getTemplate("tmplt/class/cpp/body.mustache"), map));

    hppCode.append(
        mstch::render(getTemplate("tmplt/class/hpp/foot.mustache"), map));
    cppCode.append(
        mstch::render(getTemplate("tmplt/class/cpp/foot.mustache"), map));

    return map;
  }

  bool GenerateService(io::Printer &printer,
                       const ServiceDescriptor &descriptor) const {
    (void)descriptor;
    (void)printer;
    return true;
  }

  bool GenerateMessage(io::Printer &hppPrinter, io::Printer &cppPrinter,
                       const std::string &package,
                       const Descriptor &message) const {

    std::string hppCode;
    std::string cppCode;
    mstch::map map = {{"generator", std::string("protoc_gen_mstch")},
                      {"source", message.file()->name()}};

    std::string str = package;
    std::replace(str.begin(), str.end(), '.', ' ');
    std::istringstream iss(str);
    auto namespaces =
        std::vector<std::string>(std::istream_iterator<std::string>{iss},
                                 std::istream_iterator<std::string>());

    mstch::array arr;
    for (const auto &_namespace : namespaces) {
      mstch::map map = {{"namespace", _namespace}};
      arr.push_back(map);
    }
    map["namespaces"] = arr;

    map["class"] = generateClass(hppCode, cppCode, package, message);

    // file
    map["GUID"] = getGUID();

    hppCode.insert(
        0,
        mstch::render(
            getTemplate(mTmpltPath + "/tmplt/namespace/head.mustache"), map));
    cppCode.insert(
        0, mstch::render(
               getTemplate(mTmpltPath + "tmplt/namespace/head.mustache"), map));

    hppCode.insert(
        0, mstch::render(
               getTemplate(mTmpltPath + "tmplt/file/hpp/head.mustache"), map));
    cppCode.insert(
        0, mstch::render(
               getTemplate(mTmpltPath + "tmplt/file/cpp/head.mustache"), map));

    hppCode.append(mstch::render(
        getTemplate(mTmpltPath + "tmplt/namespace/foot.mustache"), map));
    cppCode.append(mstch::render(
        getTemplate(mTmpltPath + "tmplt/namespace/foot.mustache"), map));

    hppCode.append(mstch::render(
        getTemplate(mTmpltPath + "tmplt/file/hpp/foot.mustache"), map));
    cppCode.append(mstch::render(
        getTemplate(mTmpltPath + "tmplt/file/cpp/foot.mustache"), map));

    hppPrinter.Print(hppCode.c_str());
    cppPrinter.Print(cppCode.c_str());

    return true;
  }

  bool GenerateEnum(const EnumDescriptor &descriptor,
                    io::Printer &printer) const {
    (void)descriptor;
    (void)printer;
    return true;
  }

  bool Generate(const FileDescriptor *file, const std::string &parameter,
                GeneratorContext *generator_context, std::string *error) const {

    (void)parameter;

    if (file->syntax() != FileDescriptor::SYNTAX_PROTO3) {
      std::ostringstream oss;
      oss << "MyCodeGenerator"
          << "::" << __func__ << ": suppotrs only proto3";
      *error = oss.str();
      return false;
    }

    for (int i = 0; i < file->message_type_count(); i++) {
      const Descriptor &descriptor = *file->message_type(i);

      std::unique_ptr<io::ZeroCopyOutputStream> headerOs(
          createZeroCopyOutputStream(file, generator_context, mOutputPath,
                                     descriptor.name(), ".pb.hpp"));

      std::unique_ptr<io::ZeroCopyOutputStream> sourceOs(
          createZeroCopyOutputStream(file, generator_context, mOutputPath,
                                     descriptor.name(), ".pb.cpp"));

      io::Printer hppPrinter(headerOs.get(), '$');
      io::Printer cppPrinter(sourceOs.get(), '$');
      GenerateMessage(hppPrinter, cppPrinter, file->package(), descriptor);
    }
    return true;
  }

private:
  const std::string mOutputPath{"./"};
  const std::string mTmpltPath{"./"};
};

#include <google/protobuf/compiler/command_line_interface.h>

int RunProtocGenMstch(int argc, char **argv) {
  MyCodeGenerator generator;
#if 0
  google::protobuf::compiler::CommandLineInterface cli;

  cli.RegisterGenerator("--mstch_cpp_out", &generator, "Generate CPP files.");

  return cli.Run(argc, argv);
#else
  return google::protobuf::compiler::PluginMain(argc, argv, &generator);
#endif
}

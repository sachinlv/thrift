/*
 * Licensed to the Apache Software Foundation (ASF) under one
 * or more contributor license agreements. See the NOTICE file
 * distributed with this work for additional information
 * regarding copyright ownership. The ASF licenses this file
 * to you under the Apache License, Version 2.0 (the
 * "License"); you may not use this file except in compliance
 * with the License. You may obtain a copy of the License at
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing,
 * software distributed under the License is distributed on an
 * "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
 * KIND, either express or implied. See the License for the
 * specific language governing permissions and limitations
 * under the License.
 */

/*
 * This file is programmatically sanitized for style:
 * astyle --style=1tbs -f -p -H -j -U t_go_generator.cc
 *
 * The output of astyle should not be taken unquestioningly, but it is a good
 * guide for ensuring uniformity and readability.
 */

#include <fstream>
#include <iostream>
#include <limits>
#include <string>
#include <unordered_map>
#include <vector>

#include <stdlib.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sstream>
#include <algorithm>
#include <clocale>
#include "thrift/platform.h"
#include "thrift/version.h"
#include "thrift/generate/t_generator.h"

using std::map;
using std::ostream;
using std::ostringstream;
using std::string;
using std::stringstream;
using std::vector;

static const string endl = "\n"; // avoid ostream << std::endl flushes

/**
 * A helper for automatically formatting the emitted Go code from the Thrift
 * IDL per the Go style guide.
 *
 * Returns:
 *  - true, if the formatting process succeeded.
 *  - false, if the formatting process failed, which means the basic output was
 *           still generated.
 */
bool format_go_output(const string& file_path);

const string DEFAULT_THRIFT_IMPORT = "github.com/apache/thrift/lib/go/thrift";
static std::string package_flag;

/**
 * Go code generator.
 */
class t_go_generator : public t_generator {
public:
  t_go_generator(t_program* program,
                 const std::map<std::string, std::string>& parsed_options,
                 const std::string& option_string)
    : t_generator(program) {
    (void)option_string;
    std::map<std::string, std::string>::const_iterator iter;


    gen_thrift_import_ = DEFAULT_THRIFT_IMPORT;
    gen_package_prefix_ = "";
    package_flag = "";
    read_write_private_ = false;
    ignore_initialisms_ = false;
    skip_remote_ = false;
    for( iter = parsed_options.begin(); iter != parsed_options.end(); ++iter) {
      if( iter->first.compare("package_prefix") == 0) {
        gen_package_prefix_ = (iter->second);
      } else if( iter->first.compare("thrift_import") == 0) {
        gen_thrift_import_ = (iter->second);
      } else if( iter->first.compare("package") == 0) {
        package_flag = (iter->second);
      } else if( iter->first.compare("read_write_private") == 0) {
        read_write_private_ = true;
      } else if( iter->first.compare("ignore_initialisms") == 0) {
        ignore_initialisms_ =  true;
      } else if( iter->first.compare("skip_remote") == 0) {
        skip_remote_ =  true;
      } else {
        throw "unknown option go:" + iter->first;
      }
    }

    out_dir_base_ = "gen-go";
  }

  /**
   * Init and close methods
   */

  void init_generator() override;
  void close_generator() override;

  /**
   * Program-level generation functions
   */

  void generate_typedef(t_typedef* ttypedef) override;
  void generate_enum(t_enum* tenum) override;
  void generate_const(t_const* tconst) override;
  void generate_struct(t_struct* tstruct) override;
  void generate_xception(t_struct* txception) override;
  void generate_service(t_service* tservice) override;

  std::string render_const_value(t_type* type, t_const_value* value, const string& name, bool opt = false);

  /**
   * Struct generation code
   */

  void generate_go_struct(t_struct* tstruct, bool is_exception);
  void generate_go_struct_definition(std::ostream& out,
                                     t_struct* tstruct,
                                     bool is_xception = false,
                                     bool is_result = false,
                                     bool is_args = false);
  void generate_go_struct_initializer(std::ostream& out,
                                      t_struct* tstruct,
                                      bool is_args_or_result = false);
  void generate_isset_helpers(std::ostream& out,
                              t_struct* tstruct,
                              const string& tstruct_name,
                              bool is_result = false);
  void generate_countsetfields_helper(std::ostream& out,
                                      t_struct* tstruct,
                                      const string& tstruct_name,
                                      bool is_result = false);
  void generate_go_struct_reader(std::ostream& out,
                                 t_struct* tstruct,
                                 const string& tstruct_name,
                                 bool is_result = false);
  void generate_go_struct_writer(std::ostream& out,
                                 t_struct* tstruct,
                                 const string& tstruct_name,
                                 bool is_result = false,
                                 bool uses_countsetfields = false);
  void generate_go_struct_equals(std::ostream& out, t_struct* tstruct, const string& tstruct_name);
  void generate_go_function_helpers(t_function* tfunction);
  void get_publicized_name_and_def_value(t_field* tfield,
                                         string* OUT_pub_name,
                                         t_const_value** OUT_def_value) const;

  /**
   * Service-level generation functions
   */

  void generate_service_helpers(t_service* tservice);
  void generate_service_interface(t_service* tservice);
  void generate_service_client(t_service* tservice);
  void generate_service_remote(t_service* tservice);
  void generate_service_server(t_service* tservice);
  void generate_process_function(t_service* tservice, t_function* tfunction);

  /**
   * Serialization constructs
   */

  void generate_deserialize_field(std::ostream& out,
                                  t_field* tfield,
                                  bool declare,
                                  std::string prefix = "",
                                  bool inclass = false,
                                  bool coerceData = false,
                                  bool inkey = false,
                                  bool in_container = false);

  void generate_deserialize_struct(std::ostream& out,
                                   t_struct* tstruct,
                                   bool is_pointer_field,
                                   bool declare,
                                   std::string prefix = "");

  void generate_deserialize_container(std::ostream& out,
                                      t_type* ttype,
                                      bool pointer_field,
                                      bool declare,
                                      std::string prefix = "");

  void generate_deserialize_set_element(std::ostream& out,
                                        t_set* tset,
                                        bool declare,
                                        std::string prefix = "");

  void generate_deserialize_map_element(std::ostream& out,
                                        t_map* tmap,
                                        bool declare,
                                        std::string prefix = "");

  void generate_deserialize_list_element(std::ostream& out,
                                         t_list* tlist,
                                         bool declare,
                                         std::string prefix = "");

  void generate_serialize_field(std::ostream& out,
                                t_field* tfield,
                                std::string prefix = "",
                                bool inkey = false);

  void generate_serialize_struct(std::ostream& out, t_struct* tstruct, std::string prefix = "");

  void generate_serialize_container(std::ostream& out,
                                    t_type* ttype,
                                    bool pointer_field,
                                    std::string prefix = "");

  void generate_serialize_map_element(std::ostream& out,
                                      t_map* tmap,
                                      std::string kiter,
                                      std::string viter);

  void generate_serialize_set_element(std::ostream& out, t_set* tmap, std::string iter);

  void generate_serialize_list_element(std::ostream& out, t_list* tlist, std::string iter);

  void generate_go_equals(std::ostream& out, t_type* ttype, string tgt, string src);

  void generate_go_equals_struct(std::ostream& out, t_type* ttype, string tgt, string src);

  void generate_go_equals_container(std::ostream& out, t_type* ttype, string tgt, string src);

  void generate_go_docstring(std::ostream& out, t_struct* tstruct);

  void generate_go_docstring(std::ostream& out, t_function* tfunction);

  void generate_go_docstring(std::ostream& out,
                             t_doc* tdoc,
                             t_struct* tstruct,
                             const char* subheader);

  void generate_go_docstring(std::ostream& out, t_doc* tdoc);

  void parse_go_tags(map<string,string>* tags, const string in);

  /**
   * Helper rendering functions
   */

  std::string go_autogen_comment();
  std::string go_package();
  std::string go_imports_begin(bool consts);
  std::string go_imports_end();
  std::string render_includes(bool consts);
  std::string render_included_programs(string& unused_protection);
  std::string render_program_import(const t_program* program, string& unused_protection);
  std::string render_system_packages(std::vector<string> &system_packages);
  std::string render_import_protection();
  std::string render_fastbinary_includes();
  std::string declare_argument(t_field* tfield);
  std::string render_field_initial_value(t_field* tfield, const string& name, bool optional_field);
  std::string type_name(t_type* ttype);
  std::string module_name(t_type* ttype);
  std::string function_signature(t_function* tfunction, std::string prefix = "");
  std::string function_signature_if(t_function* tfunction,
                                    std::string prefix = "",
                                    bool addError = false);
  std::string argument_list(t_struct* tstruct);
  std::string type_to_enum(t_type* ttype);
  std::string type_to_go_type(t_type* ttype);
  std::string type_to_go_type_with_opt(t_type* ttype,
                                       bool optional_field);
  std::string type_to_go_key_type(t_type* ttype);
  std::string type_to_spec_args(t_type* ttype);

  static std::string get_real_go_module(const t_program* program) {

    if (!package_flag.empty()) {
      return package_flag;
    }
    std::string real_module = program->get_namespace("go");
    if (!real_module.empty()) {
      return real_module;
    }

    return lowercase(program->get_name());
  }

private:
  std::string gen_package_prefix_;
  std::string gen_thrift_import_;
  bool read_write_private_;
  bool ignore_initialisms_;
  bool skip_remote_;

  /**
   * File streams
   */

  ofstream_with_content_based_conditional_update f_types_;
  std::string f_types_name_;
  ofstream_with_content_based_conditional_update f_consts_;
  std::string f_consts_name_;
  std::stringstream f_const_values_;

  std::string package_name_;
  std::string package_dir_;
  std::unordered_map<std::string, std::string> package_identifiers_;
  std::set<std::string> package_identifiers_set_;
  std::string read_method_name_;
  std::string write_method_name_;
  std::string equals_method_name_;

  std::set<std::string> commonInitialisms;

  std::string camelcase(const std::string& value) const;
  void fix_common_initialism(std::string& value, int i) const;
  std::string publicize(const std::string& value, bool is_args_or_result = false) const;
  std::string publicize(const std::string& value, bool is_args_or_result, const std::string& service_name) const;
  std::string privatize(const std::string& value) const;
  std::string new_prefix(const std::string& value) const;
  static std::string variable_name_to_go_name(const std::string& value);
  static bool is_pointer_field(t_field* tfield, bool in_container = false);
  static bool omit_initialization(t_field* tfield);
};

// returns true if field initialization can be omitted since it has corresponding go type zero value
// or default value is not set
bool t_go_generator::omit_initialization(t_field* tfield) {
  t_const_value* value = tfield->get_value();
  if (!value) {
    return true;
  }
  t_type* type = tfield->get_type()->get_true_type();
  if (type->is_base_type()) {
    t_base_type::t_base tbase = ((t_base_type*)type)->get_base();

    switch (tbase) {
    case t_base_type::TYPE_VOID:
      throw "";

    case t_base_type::TYPE_STRING:
      if (type->is_binary()) {
        //[]byte are always inline
        return false;
      }
      // strings are pointers if has no default
      return value->get_string().empty();

    case t_base_type::TYPE_BOOL:
    case t_base_type::TYPE_I8:
    case t_base_type::TYPE_I16:
    case t_base_type::TYPE_I32:
    case t_base_type::TYPE_I64:
      return value->get_integer() == 0;
    case t_base_type::TYPE_DOUBLE:
      if (value->get_type() == t_const_value::CV_INTEGER) {
        return value->get_integer() == 0;
      } else {
        return value->get_double() == 0.;
      }
    }
  }
  return false;
}

// Returns true if the type need a reference if used as optional without default
static bool type_need_reference(t_type* type) {
  type = type->get_true_type();
  if (type->is_map() || type->is_set() || type->is_list() || type->is_struct()
      || type->is_xception() || type->is_binary()) {
    return false;
  }
  return true;
}

// returns false if field could not use comparison to default value as !IsSet*
bool t_go_generator::is_pointer_field(t_field* tfield, bool in_container_value) {
  (void)in_container_value;
  if (tfield->annotations_.count("cpp.ref") != 0) {
    return true;
  }
  t_type* type = tfield->get_type()->get_true_type();
  // Structs in containers are pointers
  if (type->is_struct() || type->is_xception()) {
    return true;
  }
  if (!(tfield->get_req() == t_field::T_OPTIONAL)) {
    return false;
  }
  bool has_default = tfield->get_value();
  if (type->is_base_type()) {
    t_base_type::t_base tbase = ((t_base_type*)type)->get_base();

    switch (tbase) {
    case t_base_type::TYPE_VOID:
      throw "";

    case t_base_type::TYPE_STRING:
      if (type->is_binary()) {
        //[]byte are always inline
        return false;
      }
      // strings are pointers if has no default
      return !has_default;

    case t_base_type::TYPE_BOOL:
    case t_base_type::TYPE_I8:
    case t_base_type::TYPE_I16:
    case t_base_type::TYPE_I32:
    case t_base_type::TYPE_I64:
    case t_base_type::TYPE_DOUBLE:
      return !has_default;
    }
  } else if (type->is_enum()) {
    return !has_default;
  } else if (type->is_struct() || type->is_xception()) {
    return true;
  } else if (type->is_map()) {
    return has_default;
  } else if (type->is_set()) {
    return has_default;
  } else if (type->is_list()) {
    return has_default;
  } else if (type->is_typedef()) {
    return has_default;
  }

  throw "INVALID TYPE IN type_to_go_type: " + type->get_name();
}

std::string t_go_generator::camelcase(const std::string& value) const {
  std::string value2(value);
  std::setlocale(LC_ALL, "C"); // set locale to classic

  // Fix common initialism in first word
  fix_common_initialism(value2, 0);

  // as long as we are changing things, let's change _ followed by lowercase to
  // capital and fix common initialisms
  for (std::string::size_type i = 1; i < value2.size() - 1; ++i) {
    if (value2[i] == '_') {
      if (islower(value2[i + 1])) {
        value2.replace(i, 2, 1, toupper(value2[i + 1]));
      }

      if (i > static_cast<std::string::size_type>(std::numeric_limits<int>().max())) {
        throw "integer overflow in t_go_generator::camelcase, value = " + value;
      }
      fix_common_initialism(value2, static_cast<int>(i));
    }
  }

  return value2;
}

// Checks to see if the word starting at i in value contains a common initialism
// and if so replaces it with the upper case version of the word.
void t_go_generator::fix_common_initialism(std::string& value, int i) const {
  if (!ignore_initialisms_) {
    size_t wordLen = value.find('_', i);
    if (wordLen != std::string::npos) {
      wordLen -= i;
    }
    std::string word = value.substr(i, wordLen);
    std::transform(word.begin(), word.end(), word.begin(), ::toupper);
    if (commonInitialisms.find(word) != commonInitialisms.end()) {
      value.replace(i, word.length(), word);
    }
  }
}

std::string t_go_generator::publicize(const std::string& value, bool is_args_or_result, const std::string& service_name) const {
  if (value.size() <= 0) {
    return value;
  }

  std::string value2(value), prefix;

  string::size_type dot_pos = value.rfind('.');
  if (dot_pos != string::npos) {
    prefix = value.substr(0, dot_pos + 1) + prefix;
    value2 = value.substr(dot_pos + 1);
  }

  if (!isupper(value2[0])) {
    value2[0] = toupper(value2[0]);
  }

  value2 = camelcase(value2);

  // final length before further checks, the string may become longer
  size_t len_before = value2.length();

  // IDL identifiers may start with "New" which interferes with the CTOR pattern
  // Adding an extra underscore to all those identifiers solves this
  if ((len_before >= 3) && (value2.substr(0, 3) == "New")) {
    value2 += '_';
  }

  // IDL identifiers may end with "Args"/"Result" which interferes with the implicit service
  // function structs
  // Adding another extra underscore to all those identifiers solves this
  // Suppress this check for the actual helper struct names
  if (!is_args_or_result) {
    bool ends_with_args = (len_before >= 4) && (value2.substr(len_before - 4, 4) == "Args");
    bool ends_with_rslt = (len_before >= 6) && (value2.substr(len_before - 6, 6) == "Result");
    if (ends_with_args || ends_with_rslt) {
      value2 += '_';
    }
  }

  // Avoid naming collisions with other services
  if (is_args_or_result) {
    prefix += publicize(service_name);
  }

  return prefix + value2;
}

std::string t_go_generator::publicize(const std::string& value, bool is_args_or_result) const {
  return publicize(value, is_args_or_result, service_name_);
}

std::string t_go_generator::new_prefix(const std::string& value) const {
  if (value.size() <= 0) {
    return value;
  }

  string::size_type dot_pos = value.rfind('.');
  if (dot_pos != string::npos) {
    return value.substr(0, dot_pos + 1) + "New" + publicize(value.substr(dot_pos + 1));
  }
  return "New" + publicize(value);
}

std::string t_go_generator::privatize(const std::string& value) const {
  if (value.size() <= 0) {
    return value;
  }

  std::string value2(value);

  if (!islower(value2[0])) {
    value2[0] = tolower(value2[0]);
  }

  value2 = camelcase(value2);

  return value2;
}

std::string t_go_generator::variable_name_to_go_name(const std::string& value) {
  if (value.size() <= 0) {
    return value;
  }

  std::string value2(value);
  std::transform(value2.begin(), value2.end(), value2.begin(), ::tolower);

  switch (value[0]) {
  case 'b':
  case 'B':
    if (value2 != "break") {
      return value;
    }

    break;

  case 'c':
  case 'C':
    if (value2 != "case" && value2 != "chan" && value2 != "const" && value2 != "continue") {
      return value;
    }

    break;

  case 'd':
  case 'D':
    if (value2 != "default" && value2 != "defer") {
      return value;
    }

    break;

  case 'e':
  case 'E':
    if (value2 != "else" && value2 != "error") {
      return value;
    }

    break;

  case 'f':
  case 'F':
    if (value2 != "fallthrough" && value2 != "for" && value2 != "func") {
      return value;
    }

    break;

  case 'g':
  case 'G':
    if (value2 != "go" && value2 != "goto") {
      return value;
    }

    break;

  case 'i':
  case 'I':
    if (value2 != "if" && value2 != "import" && value2 != "interface") {
      return value;
    }

    break;

  case 'm':
  case 'M':
    if (value2 != "map") {
      return value;
    }

    break;

  case 'p':
  case 'P':
    if (value2 != "package") {
      return value;
    }

    break;

  case 'r':
  case 'R':
    if (value2 != "range" && value2 != "return") {
      return value;
    }

    break;

  case 's':
  case 'S':
    if (value2 != "select" && value2 != "struct" && value2 != "switch") {
      return value;
    }

    break;

  case 't':
  case 'T':
    if (value2 != "type") {
      return value;
    }

    break;

  case 'v':
  case 'V':
    if (value2 != "var") {
      return value;
    }

    break;

  default:
    return value;
  }

  return value2 + "_a1";
}

/**
 * Prepares for file generation by opening up the necessary file output
 * streams.
 *
 * @param tprogram The program to generate
 */
void t_go_generator::init_generator() {
  // Make output directory
  string module = get_real_go_module(program_);
  string target = module;
  package_dir_ = get_out_dir();

  // This set is taken from https://github.com/golang/lint/blob/master/lint.go#L692
  commonInitialisms.insert("API");
  commonInitialisms.insert("ASCII");
  commonInitialisms.insert("CPU");
  commonInitialisms.insert("CSS");
  commonInitialisms.insert("DNS");
  commonInitialisms.insert("EOF");
  commonInitialisms.insert("GUID");
  commonInitialisms.insert("HTML");
  commonInitialisms.insert("HTTP");
  commonInitialisms.insert("HTTPS");
  commonInitialisms.insert("ID");
  commonInitialisms.insert("IP");
  commonInitialisms.insert("JSON");
  commonInitialisms.insert("LHS");
  commonInitialisms.insert("QPS");
  commonInitialisms.insert("RAM");
  commonInitialisms.insert("RHS");
  commonInitialisms.insert("RPC");
  commonInitialisms.insert("SLA");
  commonInitialisms.insert("SMTP");
  commonInitialisms.insert("SSH");
  commonInitialisms.insert("TCP");
  commonInitialisms.insert("TLS");
  commonInitialisms.insert("TTL");
  commonInitialisms.insert("UDP");
  commonInitialisms.insert("UI");
  commonInitialisms.insert("UID");
  commonInitialisms.insert("UUID");
  commonInitialisms.insert("URI");
  commonInitialisms.insert("URL");
  commonInitialisms.insert("UTF8");
  commonInitialisms.insert("VM");
  commonInitialisms.insert("XML");
  commonInitialisms.insert("XSRF");
  commonInitialisms.insert("XSS");

  // names of read and write methods
  if (read_write_private_) {
    read_method_name_ = "read";
    write_method_name_ = "write";
  } else {
    read_method_name_ = "Read";
    write_method_name_ = "Write";
  }
  equals_method_name_ = "Equals";

  while (true) {
    // TODO: Do better error checking here.
    MKDIR(package_dir_.c_str());

    if (module.empty()) {
      break;
    }

    string::size_type pos = module.find('.');

    if (pos == string::npos) {
      package_dir_ += "/";
      package_dir_ += module;
      package_name_ = module;
      module.clear();
    } else {
      package_dir_ += "/";
      package_dir_ += module.substr(0, pos);
      module.erase(0, pos + 1);
    }
  }

  string::size_type loc;

  while ((loc = target.find(".")) != string::npos) {
    target.replace(loc, 1, 1, '/');
  }

  // Make output files
  f_types_name_ = package_dir_ + "/" + program_name_ + ".go";
  f_types_.open(f_types_name_);

  f_consts_name_ = package_dir_ + "/" + program_name_ + "-consts.go";
  f_consts_.open(f_consts_name_);

  // Print header
  f_types_ << go_autogen_comment() << go_package() << render_includes(false);

  f_consts_ << go_autogen_comment() << go_package() << render_includes(true);

  f_const_values_ << endl << "func init() {" << endl;

  // Create file for the GoUnusedProtection__ variable
  string f_unused_prot_name_ = package_dir_ + "/" + "GoUnusedProtection__.go";
  ofstream_with_content_based_conditional_update f_unused_prot_;
  f_unused_prot_.open(f_unused_prot_name_.c_str());
  f_unused_prot_ << go_autogen_comment() << go_package() << render_import_protection();
  f_unused_prot_.close();
}

string t_go_generator::render_included_programs(string& unused_prot) {
  const vector<t_program*>& includes = program_->get_includes();
  string result = "";
  string local_namespace = get_real_go_module(program_);
  std::set<std::string> included;
  for (auto include : includes) {
    std::string includeModule = get_real_go_module(include);
    if (!local_namespace.empty() && local_namespace == includeModule) {
      continue;
    }

    if (!included.insert(includeModule).second) {
        continue;
    }

    result += render_program_import(include, unused_prot);
  }
  return result;
}

string t_go_generator::render_program_import(const t_program* program, string& unused_protection) {
  string result = "";

  string go_module = get_real_go_module(program);
  string go_path = go_module;
  size_t found = 0;
  for (size_t j = 0; j < go_module.size(); j++) {
    // Import statement uses slashes ('/') in namespace
    if (go_module[j] == '.') {
      go_path[j] = '/';
      found = j + 1;
    }
  }

  auto it = package_identifiers_.find(go_module);
  auto last_component = go_module.substr(found);
  if (it == package_identifiers_.end()) {
    auto value = last_component;
    // This final path component has already been used, let's construct a more unique alias
    if (package_identifiers_set_.find(value) != package_identifiers_set_.end()) {
      // TODO: This would produce more readable code if it appended trailing go_module
      // path components to generate a more readable name unique identifier (e.g. use
      // packageacommon as the alias for packagea/common instead of common=). But just
      // appending an integer is much simpler code
      value = tmp(value);
    }
    package_identifiers_set_.insert(value);
    it = package_identifiers_.emplace(go_module, std::move(value)).first;
  }
  auto const& package_identifier = it->second;
  result += "\t";
  // if the package_identifier is different than final path component we need an alias
  if (last_component.compare(package_identifier) != 0) {
    result += package_identifier + " ";
  }
  string s;

  for (auto const& e : package_identifiers_set_)
  {
      s += e;
      s += ',';
  }

  s.pop_back();

  result += "\"" + gen_package_prefix_ + go_path + "\"\n";
  unused_protection += "var _ = " + package_identifier + ".GoUnusedProtection__\n";
  return result;
}

/**
 * Render import lines for the system packages.
 *
 * The arg system_packages supports the following two options for import auto
 * rename in case duplications happens:
 *
 * 1. The full import path without double quotation marks, with part after the
 *    last "/" as the import identifier. e.g.:
 *    - "context" (context)
 *    - "database/sql/driver" (driver)
 * 2. A rename import with double quotation marks around the full import path,
 *    with the part before the first space as the import identifier. e.g.:
 *    - "thrift \"github.com/apache/thrift/lib/go/thrift\"" (thrift)
 *
 * If a system package's package name is different from the last part of its
 * full import path, please always rename import it for dedup to work correctly,
 * e.g. "package \"github.com/org/go-package\"".
 *
 * @param system_packages
 */
string t_go_generator::render_system_packages(std::vector<string>& system_packages) {
  string result = "";

  for (vector<string>::iterator iter = system_packages.begin(); iter != system_packages.end(); ++iter) {
    string package = *iter;
    string identifier = package;
    auto space_pos = package.find(" ");
    if (space_pos != string::npos) {
      // This is a rename import line, no need to wrap double quotation marks.
      result += "\t"+ package +"\n";
      // The part before the first space is the import identifier.
      identifier = package.substr(0, space_pos);
    } else {
      result += "\t\""+ package +"\"\n";
      // The part after the last / is the import identifier.
      auto slash_pos = package.rfind("/");
      if (slash_pos != string::npos) {
        identifier = package.substr(slash_pos+1);
      }
    }

    // Reserve these package names in case the collide with imported Thrift packages
    package_identifiers_set_.insert(identifier);
    package_identifiers_.emplace(package, identifier);
  }
  return result;
}

/**
 * Renders all the imports necessary for including another Thrift program.
 * If consts include the additional imports.
 */
string t_go_generator::render_includes(bool consts) {
  const vector<t_program*>& includes = program_->get_includes();
  string result = "";
  string unused_prot = "";
  result += go_imports_begin(consts);
  result += render_included_programs(unused_prot);

  if (includes.size() > 0) {
    result += "\n";
  }

  return result + go_imports_end() + unused_prot;
}

string t_go_generator::render_import_protection() {
  return string("var GoUnusedProtection__ int;\n\n");
}

/**
 * Renders all the imports necessary to use the accelerated TBinaryProtocol
 */
string t_go_generator::render_fastbinary_includes() {
  return "";
}

/**
 * Autogen'd comment. The different text is necessary due to
 * https://github.com/golang/go/issues/13560#issuecomment-288457920
 */
string t_go_generator::go_autogen_comment() {
  return
        std::string() +
        "// Code generated by Thrift Compiler (" + THRIFT_VERSION + "). DO NOT EDIT.\n\n";
}

/**
 * Prints standard thrift package
 */
string t_go_generator::go_package() {
  return string("package ") + package_name_ + "\n\n";
}

/**
 * Render the beginning of the import statement.
 * If consts include the additional imports.
 */
string t_go_generator::go_imports_begin(bool consts) {
  std::vector<string> system_packages;
  system_packages.push_back("bytes");
  system_packages.push_back("context");
  // If not writing constants, and there are enums, need extra imports.
  if (!consts && get_program()->get_enums().size() > 0) {
    system_packages.push_back("database/sql/driver");
  }
  system_packages.push_back("errors");
  system_packages.push_back("fmt");
  system_packages.push_back("time");
  // For the thrift import, always do rename import to make sure it's called thrift.
  system_packages.push_back("thrift \"" + gen_thrift_import_ + "\"");
  return "import (\n" + render_system_packages(system_packages);
}

/**
 * End the import statement, include undscore-assignments
 *
 * These "_ =" prevent the go compiler complaining about unused imports.
 * This will have to do in lieu of more intelligent import statement construction
 */
string t_go_generator::go_imports_end() {
  return string(
      ")\n\n"
      "// (needed to ensure safety because of naive import list construction.)\n"
      "var _ = thrift.ZERO\n"
      "var _ = fmt.Printf\n"
      "var _ = errors.New\n"
      "var _ = context.Background\n"
      "var _ = time.Now\n"
      "var _ = bytes.Equal\n\n");
}

/**
 * Closes the type files
 */
void t_go_generator::close_generator() {
  f_const_values_ << "}" << endl << endl;
  f_consts_ << f_const_values_.str();

  // Close types and constants files
  f_consts_.close();
  f_types_.close();
  format_go_output(f_types_name_);
  format_go_output(f_consts_name_);
}

/**
 * Generates a typedef.
 *
 * @param ttypedef The type definition
 */
void t_go_generator::generate_typedef(t_typedef* ttypedef) {
  generate_go_docstring(f_types_, ttypedef);
  string new_type_name(publicize(ttypedef->get_symbolic()));
  string base_type(type_to_go_type(ttypedef->get_type()));

  if (base_type == new_type_name) {
    return;
  }

  f_types_ << "type " << new_type_name << " " << base_type << endl << endl;
  // Generate a convenience function that converts an instance of a type
  // (which may be a constant) into a pointer to an instance of a type.
  f_types_ << "func " << new_type_name << "Ptr(v " << new_type_name << ") *" << new_type_name
           << " { return &v }" << endl << endl;
}

/**
 * Generates code for an enumerated type. Done using a class to scope
 * the values.
 *
 * @param tenum The enumeration
 */
void t_go_generator::generate_enum(t_enum* tenum) {
  std::ostringstream to_string_mapping, from_string_mapping;
  std::string tenum_name(publicize(tenum->get_name()));
  generate_go_docstring(f_types_, tenum);
  f_types_ << "type " << tenum_name << " int64" << endl << "const (" << endl;

  to_string_mapping << indent() << "func (p " << tenum_name << ") String() string {" << endl;
  to_string_mapping << indent() << "  switch p {" << endl;

  from_string_mapping << indent() << "func " << tenum_name << "FromString(s string) (" << tenum_name
                      << ", error) {" << endl;
  from_string_mapping << indent() << "  switch s {" << endl;

  vector<t_enum_value*> constants = tenum->get_constants();
  vector<t_enum_value*>::iterator c_iter;
  int value = -1;

  for (c_iter = constants.begin(); c_iter != constants.end(); ++c_iter) {
    value = (*c_iter)->get_value();

    string iter_std_name(escape_string((*c_iter)->get_name()));
    string iter_name((*c_iter)->get_name());
    f_types_ << indent() << "  " << tenum_name << "_" << iter_name << ' ' << tenum_name << " = "
             << value << endl;
    // Dictionaries to/from string names of enums
    to_string_mapping << indent() << "  case " << tenum_name << "_" << iter_name << ": return \""
                      << iter_std_name << "\"" << endl;

    if (iter_std_name != escape_string(iter_name)) {
      from_string_mapping << indent() << "  case \"" << iter_std_name << "\", \""
                          << escape_string(iter_name) << "\": return " << tenum_name << "_"
                          << iter_name << ", nil " << endl;
    } else {
      from_string_mapping << indent() << "  case \"" << iter_std_name << "\": return " << tenum_name
                          << "_" << iter_name << ", nil " << endl;
    }
  }

  to_string_mapping << indent() << "  }" << endl;
  to_string_mapping << indent() << "  return \"<UNSET>\"" << endl;
  to_string_mapping << indent() << "}" << endl;
  from_string_mapping << indent() << "  }" << endl;
  from_string_mapping << indent() << "  return " << tenum_name << "(0),"
                      << " fmt.Errorf(\"not a valid " << tenum_name << " string\")" << endl;
  from_string_mapping << indent() << "}" << endl;

  f_types_ << ")" << endl << endl << to_string_mapping.str() << endl << from_string_mapping.str()
           << endl << endl;

  // Generate a convenience function that converts an instance of an enum
  // (which may be a constant) into a pointer to an instance of that enum
  // type.
  f_types_ << "func " << tenum_name << "Ptr(v " << tenum_name << ") *" << tenum_name
           << " { return &v }" << endl << endl;

  // Generate MarshalText
  f_types_ << "func (p " << tenum_name << ") MarshalText() ([]byte, error) {" << endl;
  f_types_ << "return []byte(p.String()), nil" << endl;
  f_types_ << "}" << endl << endl;

  // Generate UnmarshalText
  f_types_ << "func (p *" << tenum_name << ") UnmarshalText(text []byte) error {" << endl;
  f_types_ << "q, err := " << tenum_name << "FromString(string(text))" << endl;
  f_types_ << "if (err != nil) {" << endl << "return err" << endl << "}" << endl;
  f_types_ << "*p = q" << endl;
  f_types_ << "return nil" << endl;
  f_types_ << "}" << endl << endl;

  // Generate Scan for sql.Scanner interface
  f_types_ << "func (p *" << tenum_name << ") Scan(value interface{}) error {" <<endl;
  f_types_ << "v, ok := value.(int64)" <<endl;
  f_types_ << "if !ok {" <<endl;
  f_types_ << "return errors.New(\"Scan value is not int64\")" <<endl;
  f_types_ << "}" <<endl;
  f_types_ << "*p = " << tenum_name << "(v)" << endl;
  f_types_ << "return nil" << endl;
  f_types_ << "}" << endl << endl;

  // Generate Value for driver.Valuer interface
  f_types_ << "func (p * " << tenum_name << ") Value() (driver.Value, error) {" <<endl;
  f_types_ << "  if p == nil {" << endl;
  f_types_ << "    return nil, nil" << endl;
  f_types_ << "  }" << endl;
  f_types_ << "return int64(*p), nil" << endl;
  f_types_ << "}" << endl;

}

/**
 * Generate a constant value
 */
void t_go_generator::generate_const(t_const* tconst) {
  t_type* type = tconst->get_type();
  string name = publicize(tconst->get_name());
  t_const_value* value = tconst->get_value();
  if (type->is_base_type() || type->is_enum()) {
    indent(f_consts_) << "const " << name << " = " << render_const_value(type, value, name) << endl;
  } else {
    f_const_values_ << indent() << name << " = " << render_const_value(type, value, name) << endl
                    << endl;

    f_consts_ << indent() << "var " << name << " " << type_to_go_type(type) << endl;
  }
}

/**
 * Prints the value of a constant with the given type. Note that type checking
 * is NOT performed in this function as it is always run beforehand using the
 * validate_types method in main.cc
 */
string t_go_generator::render_const_value(t_type* type, t_const_value* value, const string& name, bool opt) {
  string typedef_opt_ptr;
  if (type->is_typedef()) {
    typedef_opt_ptr = type_name(type) + "Ptr";
  }
  type = get_true_type(type);
  std::ostringstream out;

  if (type->is_base_type()) {
    t_base_type::t_base tbase = ((t_base_type*)type)->get_base();

    if (opt) {
      switch (tbase) {
        case t_base_type::TYPE_BOOL:
          if (typedef_opt_ptr != "") {
            out << typedef_opt_ptr;
          } else {
            out << "thrift.BoolPtr";
          }
          out << "(";
          out << (value->get_integer() > 0 ? "true" : "false");
          break;

        case t_base_type::TYPE_I8:
          if (typedef_opt_ptr != "") {
            out << typedef_opt_ptr;
          } else {
            out << "thrift.Int8Ptr";
          }
          out << "(";
          out << value->get_integer();
          break;
        case t_base_type::TYPE_I16:
          if (typedef_opt_ptr != "") {
            out << typedef_opt_ptr;
          } else {
            out << "thrift.Int16Ptr";
          }
          out << "(";
          out << value->get_integer();
          break;
        case t_base_type::TYPE_I32:
          if (typedef_opt_ptr != "") {
            out << typedef_opt_ptr;
          } else {
            out << "thrift.Int32Ptr";
          }
          out << "(";
          out << value->get_integer();
          break;
        case t_base_type::TYPE_I64:
          if (typedef_opt_ptr != "") {
            out << typedef_opt_ptr;
          } else {
            out << "thrift.Int64Ptr";
          }
          out << "(";
          out << value->get_integer();
          break;

        case t_base_type::TYPE_DOUBLE:
          if (typedef_opt_ptr != "") {
            out << typedef_opt_ptr;
          } else {
            out << "thrift.Float64Ptr";
          }
          out << "(";
          if (value->get_type() == t_const_value::CV_INTEGER) {
            out << value->get_integer();
          } else {
            out << value->get_double();
          }
          break;

        case t_base_type::TYPE_STRING:
          if (typedef_opt_ptr != "") {
            out << typedef_opt_ptr;
          } else {
            out << "thrift.StringPtr";
          }
          out << "(";
          out << '"' + get_escaped_string(value) + '"';
          break;

        default:
          throw "compiler error: no const of base type " + t_base_type::t_base_name(tbase);
      }
      out << ")";
    } else {
      switch (tbase) {
        case t_base_type::TYPE_STRING:
          if (type->is_binary()) {
            out << "[]byte(\"" << get_escaped_string(value) << "\")";
          } else {
            out << '"' << get_escaped_string(value) << '"';
          }

          break;

        case t_base_type::TYPE_BOOL:
          out << (value->get_integer() > 0 ? "true" : "false");
          break;

        case t_base_type::TYPE_I8:
        case t_base_type::TYPE_I16:
        case t_base_type::TYPE_I32:
        case t_base_type::TYPE_I64:
          if (opt) {
            out << "&(&struct{x int}{";
          }
          out << value->get_integer();
          if (opt) {
            out << "}).x";
          }
          break;

        case t_base_type::TYPE_DOUBLE:
          if (value->get_type() == t_const_value::CV_INTEGER) {
            out << value->get_integer();
          } else {
            out << value->get_double();
          }

          break;

        default:
          throw "compiler error: no const of base type " + t_base_type::t_base_name(tbase);
      }
    }
  } else if (type->is_enum()) {
    if (opt) {
      if (typedef_opt_ptr != "") {
        out << typedef_opt_ptr << "(";
      } else {
        out << type_name(type) << "Ptr(";
      }
    }
    out << value->get_integer();
    if (opt) {
      out << ")";
    }
  } else if (type->is_struct() || type->is_xception()) {
    out << "&" << publicize(type_name(type)) << "{";
    indent_up();
    const vector<t_field*>& fields = ((t_struct*)type)->get_members();
    vector<t_field*>::const_iterator f_iter;
    const map<t_const_value*, t_const_value*, t_const_value::value_compare>& val = value->get_map();
    map<t_const_value*, t_const_value*, t_const_value::value_compare>::const_iterator v_iter;

    for (v_iter = val.begin(); v_iter != val.end(); ++v_iter) {
      t_type* field_type = nullptr;
      bool is_optional = false;
      for (f_iter = fields.begin(); f_iter != fields.end(); ++f_iter) {
        if ((*f_iter)->get_name() == v_iter->first->get_string()) {
          field_type = (*f_iter)->get_type();
          is_optional = is_pointer_field(*f_iter);
        }
      }

      if (field_type == nullptr) {
        throw "type error: " + type->get_name() + " has no field " + v_iter->first->get_string();
      }
      out << endl << indent() << publicize(v_iter->first->get_string()) << ": "
          << render_const_value(field_type, v_iter->second, name, is_optional) << "," << endl;
    }

    indent_down();
    out << "}";

  } else if (type->is_map()) {
    t_type* ktype = ((t_map*)type)->get_key_type();
    t_type* vtype = ((t_map*)type)->get_val_type();
    const map<t_const_value*, t_const_value*, t_const_value::value_compare>& val = value->get_map();
    out << "map[" << type_to_go_key_type(ktype) << "]" << type_to_go_type(vtype) << "{" << endl;
    indent_up();
    map<t_const_value*, t_const_value*, t_const_value::value_compare>::const_iterator v_iter;

    for (v_iter = val.begin(); v_iter != val.end(); ++v_iter) {
      out << indent() << render_const_value(ktype, v_iter->first, name) << ": "
          << render_const_value(vtype, v_iter->second, name) << "," << endl;
    }

    indent_down();
    out << indent() << "}";
  } else if (type->is_list()) {
    t_type* etype = ((t_list*)type)->get_elem_type();
    const vector<t_const_value*>& val = value->get_list();
    out << "[]" << type_to_go_type(etype) << "{" << endl;
    indent_up();
    vector<t_const_value*>::const_iterator v_iter;

    for (v_iter = val.begin(); v_iter != val.end(); ++v_iter) {
      out << indent() << render_const_value(etype, *v_iter, name) << ", ";
    }

    indent_down();
    out << indent() << "}";
  } else if (type->is_set()) {
    t_type* etype = ((t_set*)type)->get_elem_type();
    const vector<t_const_value*>& val = value->get_list();
    out << "[]" << type_to_go_type(etype) << "{" << endl;
    indent_up();
    vector<t_const_value*>::const_iterator v_iter;

    for (v_iter = val.begin(); v_iter != val.end(); ++v_iter) {
      out << indent() << render_const_value(etype, *v_iter, name) << ", ";
    }

    indent_down();
    out << indent() << "}";
  } else {
    throw "CANNOT GENERATE CONSTANT FOR TYPE: " + type->get_name();
  }

  return out.str();
}

/**
 * Generates a go struct
 */
void t_go_generator::generate_struct(t_struct* tstruct) {
  generate_go_struct(tstruct, false);
}

/**
 * Generates a struct definition for a thrift exception. Basically the same
 * as a struct but extends the Exception class.
 *
 * @param txception The struct definition
 */
void t_go_generator::generate_xception(t_struct* txception) {
  generate_go_struct(txception, true);
}

/**
 * Generates a go struct
 */
void t_go_generator::generate_go_struct(t_struct* tstruct, bool is_exception) {
  generate_go_struct_definition(f_types_, tstruct, is_exception);
}

void t_go_generator::get_publicized_name_and_def_value(t_field* tfield,
                                                       string* OUT_pub_name,
                                                       t_const_value** OUT_def_value) const {
  const string base_field_name = tfield->get_name();
  const string escaped_field_name = escape_string(base_field_name);
  *OUT_pub_name = publicize(escaped_field_name);
  *OUT_def_value = tfield->get_value();
}

void t_go_generator::generate_go_struct_initializer(ostream& out,
                                                    t_struct* tstruct,
                                                    bool is_args_or_result) {
  out << publicize(type_name(tstruct), is_args_or_result) << "{";
  const vector<t_field*>& members = tstruct->get_members();
  for (auto member : members) {
    bool pointer_field = is_pointer_field(member);
    string publicized_name;
    t_const_value* def_value;
    get_publicized_name_and_def_value(member, &publicized_name, &def_value);
    if (!pointer_field && def_value != nullptr && !omit_initialization(member)) {
      out << endl << indent() << publicized_name << ": "
          << render_field_initial_value(member, member->get_name(), pointer_field) << ","
          << endl;
    }
  }

  out << "}" << endl;
}

/**
 * Generates a struct definition for a thrift data type.
 *
 * @param tstruct The struct definition
 */
void t_go_generator::generate_go_struct_definition(ostream& out,
                                                   t_struct* tstruct,
                                                   bool is_exception,
                                                   bool is_result,
                                                   bool is_args) {
  const vector<t_field*>& members = tstruct->get_members();
  const vector<t_field*>& sorted_members = tstruct->get_sorted_members();
  vector<t_field*>::const_iterator m_iter;

  std::string tstruct_name(publicize(tstruct->get_name(), is_args || is_result));
  generate_go_docstring(out, tstruct);
  out << indent() << "type " << tstruct_name << " struct {" << endl;
  /*
     Here we generate the structure specification for the fastbinary codec.
     These specifications have the following structure:
     thrift_spec -> tuple of item_spec
     item_spec -> nil | (tag, type_enum, name, spec_args, default)
     tag -> integer
     type_enum -> TType.I32 | TType.STRING | TType.STRUCT | ...
     name -> string_literal
     default -> nil  # Handled by __init__
     spec_args -> nil  # For simple types
                | (type_enum, spec_args)  # Value type for list/set
                | (type_enum, spec_args, type_enum, spec_args)
                  # Key and value for map
                | (class_name, spec_args_ptr) # For struct/exception
     class_name -> identifier  # Basically a pointer to the class
     spec_args_ptr -> expression  # just class_name.spec_args

     TODO(dreiss): Consider making this work for structs with negative tags.
  */
  // TODO(dreiss): Look into generating an empty tuple instead of nil
  // for structures with no members.
  // TODO(dreiss): Test encoding of structs where some inner structs
  // don't have thrift_spec.
  indent_up();

  int num_setable = 0;
  if (sorted_members.empty() || (sorted_members[0]->get_key() >= 0)) {
    int sorted_keys_pos = 0;

    for (m_iter = sorted_members.begin(); m_iter != sorted_members.end(); ++m_iter) {
      // Set field to optional if field is union, this is so we can get a
      // pointer to the field.
      if (tstruct->is_union())
        (*m_iter)->set_req(t_field::T_OPTIONAL);
      if (sorted_keys_pos != (*m_iter)->get_key()) {
        int first_unused = (std::max)(1, sorted_keys_pos++);
        while (sorted_keys_pos != (*m_iter)->get_key()) {
          ++sorted_keys_pos;
        }
        int last_unused = sorted_keys_pos - 1;
        if (first_unused < last_unused) {
          indent(out) << "// unused fields # " << first_unused << " to " << last_unused << endl;
        } else if (first_unused == last_unused) {
          indent(out) << "// unused field # " << first_unused << endl;
        }
      }

      t_type* fieldType = (*m_iter)->get_type();
      string goType = type_to_go_type_with_opt(fieldType, is_pointer_field(*m_iter));

      map<string,string>tags;
      tags["db"]=escape_string((*m_iter)->get_name());

      // Only add the `omitempty` tag if this field is optional and has no default value.
      // Otherwise a proper value like `false` for a bool field will be ommitted from
      // the JSON output since Go Marshal won't output `zero values`.
      bool has_default = (*m_iter)->get_value();
      bool is_optional = (*m_iter)->get_req() == t_field::T_OPTIONAL;
      if (is_optional && !has_default) {
        tags["json"]=escape_string((*m_iter)->get_name())+",omitempty";
      } else {
        tags["json"]=escape_string((*m_iter)->get_name());
      }

      // Check for user defined tags and them if there are any. User defined tags
      // can override the above db and json tags.
      std::map<string, string>::iterator it = (*m_iter)->annotations_.find("go.tag");
      if (it != (*m_iter)->annotations_.end()) {
        parse_go_tags(&tags, it->second);
      }

      string gotag;
      for (auto it = tags.begin(); it != tags.end(); ++it) {
        gotag += it->first + ":\"" + it->second + "\" ";
      }
      // Trailing whitespace
      gotag.resize(gotag.size()-1);

      indent(out) << publicize((*m_iter)->get_name()) << " " << goType << " `thrift:\""
                  << escape_string((*m_iter)->get_name()) << "," << sorted_keys_pos;
      if ((*m_iter)->get_req() == t_field::T_REQUIRED) {
        out << ",required";
      }

      out << "\" " << gotag << "`" << endl;
      sorted_keys_pos++;
    }
  } else {
    for (m_iter = members.begin(); m_iter != members.end(); ++m_iter) {
      // This fills in default values, as opposed to nulls
      out << indent() << publicize((*m_iter)->get_name()) << " "
          << type_to_go_type((*m_iter)->get_type()) << endl;
    }
  }

  indent_down();
  out << indent() << "}" << endl << endl;
  out << indent() << "func New" << tstruct_name << "() *" << tstruct_name << " {" << endl;
  out << indent() << "  return &";
  generate_go_struct_initializer(out, tstruct, is_result || is_args);
  out << indent() << "}" << endl << endl;
  // Default values for optional fields
  for (m_iter = members.begin(); m_iter != members.end(); ++m_iter) {
    string publicized_name;
    t_const_value* def_value;
    get_publicized_name_and_def_value(*m_iter, &publicized_name, &def_value);
    t_type* fieldType = (*m_iter)->get_type();
    string goType = type_to_go_type_with_opt(fieldType, false);
    string def_var_name = tstruct_name + "_" + publicized_name + "_DEFAULT";
    if ((*m_iter)->get_req() == t_field::T_OPTIONAL || is_pointer_field(*m_iter)) {
      out << indent() << "var " << def_var_name << " " << goType;
      if (def_value != nullptr) {
        out << " = " << render_const_value(fieldType, def_value, (*m_iter)->get_name());
      }
      out << endl;
    }

    // num_setable is used for deciding if Count* methods will be generated for union fields.
    // This applies to all nullable fields including slices (used for set, list and binary) and maps, not just pointers.
    t_type* type = fieldType->get_true_type();
    if (is_pointer_field(*m_iter)|| type->is_map() || type->is_set() || type->is_list() || type->is_binary()) {
      num_setable += 1;
    }

    if (is_pointer_field(*m_iter)) {
      string goOptType = type_to_go_type_with_opt(fieldType, true);
      string maybepointer = goOptType != goType ? "*" : "";
      out << indent() << "func (p *" << tstruct_name << ") Get" << publicized_name << "() "
          << goType << " {" << endl;
      out << indent() << "  if !p.IsSet" << publicized_name << "() {" << endl;
      out << indent() << "    return " << def_var_name << endl;
      out << indent() << "  }" << endl;
      out << indent() << "return " << maybepointer << "p." << publicized_name << endl;
      out << indent() << "}" << endl;
    } else {
      out << endl;
      out << indent() << "func (p *" << tstruct_name << ") Get" << publicized_name << "() "
          << goType << " {" << endl;
      out << indent() << "  return p." << publicized_name << endl;
      out << indent() << "}" << endl;
    }
  }

  if (tstruct->is_union() && num_setable > 0) {
    generate_countsetfields_helper(out, tstruct, tstruct_name, is_result);
  }

  generate_isset_helpers(out, tstruct, tstruct_name, is_result);
  generate_go_struct_reader(out, tstruct, tstruct_name, is_result);
  generate_go_struct_writer(out, tstruct, tstruct_name, is_result, num_setable > 0);
  if (!is_result && !is_args) {
    generate_go_struct_equals(out, tstruct, tstruct_name);
  }

  out << indent() << "func (p *" << tstruct_name << ") String() string {" << endl;
  out << indent() << "  if p == nil {" << endl;
  out << indent() << "    return \"<nil>\"" << endl;
  out << indent() << "  }" << endl;
  out << indent() << "  return fmt.Sprintf(\"" << escape_string(tstruct_name) << "(%+v)\", *p)"
      << endl;
  out << indent() << "}" << endl << endl;

  if (is_exception) {
    out << indent() << "func (p *" << tstruct_name << ") Error() string {" << endl;
    indent_up();
    out << indent() << "return p.String()" << endl;
    indent_down();
    out << indent() << "}" << endl << endl;

    out << indent() << "func (" << tstruct_name << ") TExceptionType() thrift.TExceptionType {" << endl;
    indent_up();
    out << indent() << "return thrift.TExceptionTypeCompiled" << endl;
    indent_down();
    out << indent() << "}" << endl << endl;

    out << indent() << "var _ thrift.TException = (*" << tstruct_name << ")(nil)"
        << endl << endl;
  }
}

/**
 * Generates the IsSet helper methods for a struct
 */
void t_go_generator::generate_isset_helpers(ostream& out,
                                            t_struct* tstruct,
                                            const string& tstruct_name,
                                            bool is_result) {
  (void)is_result;
  const vector<t_field*>& fields = tstruct->get_members();
  vector<t_field*>::const_iterator f_iter;
  const string escaped_tstruct_name(escape_string(tstruct->get_name()));

  for (f_iter = fields.begin(); f_iter != fields.end(); ++f_iter) {
    const string field_name(publicize(escape_string((*f_iter)->get_name())));
    if ((*f_iter)->get_req() == t_field::T_OPTIONAL || is_pointer_field(*f_iter)) {
      out << indent() << "func (p *" << tstruct_name << ") IsSet" << field_name << "() bool {"
          << endl;
      indent_up();
      t_type* ttype = (*f_iter)->get_type()->get_true_type();
      bool is_byteslice = ttype->is_binary();
      bool compare_to_nil_only = ttype->is_set() || ttype->is_list() || ttype->is_map()
                                 || (is_byteslice && !(*f_iter)->get_value());
      if (is_pointer_field(*f_iter) || compare_to_nil_only) {
        out << indent() << "return p." << field_name << " != nil" << endl;
      } else {
        string def_var_name = tstruct_name + "_" + field_name + "_DEFAULT";
        if (is_byteslice) {
          out << indent() << "return !bytes.Equal(p." << field_name << ", " << def_var_name << ")"
              << endl;
        } else {
          out << indent() << "return p." << field_name << " != " << def_var_name << endl;
        }
      }
      indent_down();
      out << indent() << "}" << endl << endl;
    }
  }
}

/**
 * Generates the CountSetFields helper method for a struct
 */
void t_go_generator::generate_countsetfields_helper(ostream& out,
                                                    t_struct* tstruct,
                                                    const string& tstruct_name,
                                                    bool is_result) {
  (void)is_result;
  const vector<t_field*>& fields = tstruct->get_members();
  vector<t_field*>::const_iterator f_iter;
  const string escaped_tstruct_name(escape_string(tstruct->get_name()));

  out << indent() << "func (p *" << tstruct_name << ") CountSetFields" << tstruct_name << "() int {"
      << endl;
  indent_up();
  out << indent() << "count := 0" << endl;
  for (f_iter = fields.begin(); f_iter != fields.end(); ++f_iter) {
    if ((*f_iter)->get_req() == t_field::T_REQUIRED)
      continue;

    t_type* type = (*f_iter)->get_type()->get_true_type();

    if (!(is_pointer_field(*f_iter) || type->is_map() || type->is_set() || type->is_list() || type->is_binary()))
      continue;

    const string field_name(publicize(escape_string((*f_iter)->get_name())));

    out << indent() << "if (p.IsSet" << field_name << "()) {" << endl;
    indent_up();
    out << indent() << "count++" << endl;
    indent_down();
    out << indent() << "}" << endl;
  }

  out << indent() << "return count" << endl << endl;
  indent_down();
  out << indent() << "}" << endl << endl;
}

/**
 * Generates the read method for a struct
 */
void t_go_generator::generate_go_struct_reader(ostream& out,
                                               t_struct* tstruct,
                                               const string& tstruct_name,
                                               bool is_result) {
  (void)is_result;
  const vector<t_field*>& fields = tstruct->get_members();
  vector<t_field*>::const_iterator f_iter;
  string escaped_tstruct_name(escape_string(tstruct->get_name()));
  out << indent() << "func (p *" << tstruct_name << ") " << read_method_name_ << "(ctx context.Context, iprot thrift.TProtocol) error {"
      << endl;
  indent_up();
  out << indent() << "if _, err := iprot.ReadStructBegin(ctx); err != nil {" << endl;
  out << indent() << "  return thrift.PrependError(fmt.Sprintf(\"%T read error: \", p), err)"
      << endl;
  out << indent() << "}" << endl << endl;

  // Required variables does not have IsSet functions, so we need tmp vars to check them.
  for (f_iter = fields.begin(); f_iter != fields.end(); ++f_iter) {
    if ((*f_iter)->get_req() == t_field::T_REQUIRED) {
      const string field_name(publicize(escape_string((*f_iter)->get_name())));
      indent(out) << "var isset" << field_name << " bool = false;" << endl;
    }
  }
  out << endl;

  // Loop over reading in fields
  indent(out) << "for {" << endl;
  indent_up();
  // Read beginning field marker
  out << indent() << "_, fieldTypeId, fieldId, err := iprot.ReadFieldBegin(ctx)" << endl;
  out << indent() << "if err != nil {" << endl;
  out << indent() << "  return thrift.PrependError(fmt.Sprintf("
                     "\"%T field %d read error: \", p, fieldId), err)" << endl;
  out << indent() << "}" << endl;
  // Check for field STOP marker and break
  out << indent() << "if fieldTypeId == thrift.STOP { break; }" << endl;

  string thriftFieldTypeId;
  // Generate deserialization code for known cases
  int32_t field_id = -1;

  // Switch statement on the field we are reading, false if no fields present
  bool have_switch = !fields.empty();
  if (have_switch) {
    indent(out) << "switch fieldId {" << endl;
  }

  // All the fields we know
  for (f_iter = fields.begin(); f_iter != fields.end(); ++f_iter) {
    field_id = (*f_iter)->get_key();

    // if negative id, ensure we generate a valid method name
    string field_method_prefix("ReadField");
    int32_t field_method_suffix = field_id;

    if (field_method_suffix < 0) {
      field_method_prefix += "_";
      field_method_suffix *= -1;
    }

    out << indent() << "case " << field_id << ":" << endl;
    indent_up();
    thriftFieldTypeId = type_to_enum((*f_iter)->get_type());

    if (thriftFieldTypeId == "thrift.BINARY") {
      thriftFieldTypeId = "thrift.STRING";
    }

    out << indent() << "if fieldTypeId == " << thriftFieldTypeId << " {" << endl;
    out << indent() << "  if err := p." << field_method_prefix << field_method_suffix << "(ctx, iprot); err != nil {"
        << endl;
    out << indent() << "    return err" << endl;
    out << indent() << "  }" << endl;

    // Mark required field as read
    if ((*f_iter)->get_req() == t_field::T_REQUIRED) {
      const string field_name(publicize(escape_string((*f_iter)->get_name())));
      out << indent() << "  isset" << field_name << " = true" << endl;
    }

    out << indent() << "} else {" << endl;
    out << indent() << "  if err := iprot.Skip(ctx, fieldTypeId); err != nil {" << endl;
    out << indent() << "    return err" << endl;
    out << indent() << "  }" << endl;
    out << indent() << "}" << endl;


    indent_down();
  }

  // Begin switch default case
  if (have_switch) {
    out << indent() << "default:" << endl;
    indent_up();
  }

  // Skip unknown fields in either case
  out << indent() << "if err := iprot.Skip(ctx, fieldTypeId); err != nil {" << endl;
  out << indent() << "  return err" << endl;
  out << indent() << "}" << endl;

  // End switch default case
  if (have_switch) {
    indent_down();
    out << indent() << "}" << endl;
  }

  // Read field end marker
  out << indent() << "if err := iprot.ReadFieldEnd(ctx); err != nil {" << endl;
  out << indent() << "  return err" << endl;
  out << indent() << "}" << endl;
  indent_down();
  out << indent() << "}" << endl;
  out << indent() << "if err := iprot.ReadStructEnd(ctx); err != nil {" << endl;
  out << indent() << "  return thrift.PrependError(fmt.Sprintf("
                     "\"%T read struct end error: \", p), err)" << endl;
  out << indent() << "}" << endl;

  // Return error if any required fields are missing.
  for (f_iter = fields.begin(); f_iter != fields.end(); ++f_iter) {
    if ((*f_iter)->get_req() == t_field::T_REQUIRED) {
      const string field_name(publicize(escape_string((*f_iter)->get_name())));
      out << indent() << "if !isset" << field_name << "{" << endl;
      out << indent() << "  return thrift.NewTProtocolExceptionWithType(thrift.INVALID_DATA, "
                         "fmt.Errorf(\"Required field " << field_name << " is not set\"));" << endl;
      out << indent() << "}" << endl;
    }
  }

  out << indent() << "return nil" << endl;
  indent_down();
  out << indent() << "}" << endl << endl;

  for (f_iter = fields.begin(); f_iter != fields.end(); ++f_iter) {
    string field_type_name(publicize((*f_iter)->get_type()->get_name()));
    string field_name(publicize((*f_iter)->get_name()));
    string field_method_prefix("ReadField");
    int32_t field_id = (*f_iter)->get_key();
    int32_t field_method_suffix = field_id;

    if (field_method_suffix < 0) {
      field_method_prefix += "_";
      field_method_suffix *= -1;
    }

    out << indent() << "func (p *" << tstruct_name << ")  " << field_method_prefix << field_method_suffix
        << "(ctx context.Context, iprot thrift.TProtocol) error {" << endl;
    indent_up();
    generate_deserialize_field(out, *f_iter, false, "p.");
    indent_down();
    out << indent() << "  return nil" << endl;
    out << indent() << "}" << endl << endl;
  }
}

void t_go_generator::generate_go_struct_writer(ostream& out,
                                               t_struct* tstruct,
                                               const string& tstruct_name,
                                               bool is_result,
                                               bool uses_countsetfields) {
  (void)is_result;
  string name(tstruct->get_name());
  const vector<t_field*>& fields = tstruct->get_sorted_members();
  vector<t_field*>::const_iterator f_iter;
  indent(out) << "func (p *" << tstruct_name << ") " << write_method_name_ << "(ctx context.Context, oprot thrift.TProtocol) error {" << endl;
  indent_up();
  if (tstruct->is_union() && uses_countsetfields) {
    std::string tstruct_name(publicize(tstruct->get_name()));
    out << indent() << "if c := p.CountSetFields" << tstruct_name << "(); c != 1 {" << endl
        << indent()
        << "  return fmt.Errorf(\"%T write union: exactly one field must be set (%d set)\", p, c)"
        << endl << indent() << "}" << endl;
  }
  out << indent() << "if err := oprot.WriteStructBegin(ctx, \"" << name << "\"); err != nil {" << endl;
  out << indent() << "  return thrift.PrependError(fmt.Sprintf("
                     "\"%T write struct begin error: \", p), err) }" << endl;

  string field_name;
  string escape_field_name;
  // t_const_value* field_default_value;
  t_field::e_req field_required;
  int32_t field_id = -1;

  out << indent() << "if p != nil {" << endl;
  indent_up();

  for (f_iter = fields.begin(); f_iter != fields.end(); ++f_iter) {
    string field_method_prefix("writeField");
    field_name = (*f_iter)->get_name();
    escape_field_name = escape_string(field_name);
    field_id = (*f_iter)->get_key();
    int32_t field_method_suffix = field_id;

    if (field_method_suffix < 0) {
      field_method_prefix += "_";
      field_method_suffix *= -1;
    }

    out << indent() << "if err := p." << field_method_prefix << field_method_suffix
        << "(ctx, oprot); err != nil { return err }" << endl;
  }

  indent_down();
  out << indent() << "}" << endl;

  // Write the struct map
  out << indent() << "if err := oprot.WriteFieldStop(ctx); err != nil {" << endl;
  out << indent() << "  return thrift.PrependError(\"write field stop error: \", err) }" << endl;
  out << indent() << "if err := oprot.WriteStructEnd(ctx); err != nil {" << endl;
  out << indent() << "  return thrift.PrependError(\"write struct stop error: \", err) }" << endl;
  out << indent() << "return nil" << endl;
  indent_down();
  out << indent() << "}" << endl << endl;

  for (f_iter = fields.begin(); f_iter != fields.end(); ++f_iter) {
    string field_method_prefix("writeField");
    field_id = (*f_iter)->get_key();
    field_name = (*f_iter)->get_name();
    escape_field_name = escape_string(field_name);
    // field_default_value = (*f_iter)->get_value();
    field_required = (*f_iter)->get_req();
    int32_t field_method_suffix = field_id;

    if (field_method_suffix < 0) {
      field_method_prefix += "_";
      field_method_suffix *= -1;
    }

    out << indent() << "func (p *" << tstruct_name << ") " << field_method_prefix << field_method_suffix
        << "(ctx context.Context, oprot thrift.TProtocol) (err error) {" << endl;
    indent_up();

    if (field_required == t_field::T_OPTIONAL) {
      out << indent() << "if p.IsSet" << publicize(field_name) << "() {" << endl;
      indent_up();
    }

    out << indent() << "if err := oprot.WriteFieldBegin(ctx, \"" << escape_field_name << "\", "
        << type_to_enum((*f_iter)->get_type()) << ", " << field_id << "); err != nil {" << endl;
    out << indent() << "  return thrift.PrependError(fmt.Sprintf(\"%T write field begin error "
        << field_id << ":" << escape_field_name << ": \", p), err) }" << endl;

    // Write field contents
    generate_serialize_field(out, *f_iter, "p.");

    // Write field closer
    out << indent() << "if err := oprot.WriteFieldEnd(ctx); err != nil {" << endl;
    out << indent() << "  return thrift.PrependError(fmt.Sprintf(\"%T write field end error "
        << field_id << ":" << escape_field_name << ": \", p), err) }" << endl;

    if (field_required == t_field::T_OPTIONAL) {
      indent_down();
      out << indent() << "}" << endl;
    }

    indent_down();
    out << indent() << "  return err" << endl;
    out << indent() << "}" << endl << endl;
  }
}

void t_go_generator::generate_go_struct_equals(ostream& out,
                                               t_struct* tstruct,
                                               const string& tstruct_name) {
  string name(tstruct->get_name());
  const vector<t_field*>& fields = tstruct->get_sorted_members();
  vector<t_field*>::const_iterator f_iter;
  indent(out) << "func (p *" << tstruct_name << ") " << equals_method_name_ << "(other *"
              << tstruct_name << ") bool {" << endl;
  indent_up();

  string field_name;
  string publicize_field_name;
  out << indent() << "if p == other {" << endl;
  indent_up();
  out << indent() << "return true" << endl;
  indent_down();
  out << indent() << "} else if p == nil || other == nil {" << endl;
  indent_up();
  out << indent() << "return false" << endl;
  indent_down();
  out << indent() << "}" << endl;

  for (f_iter = fields.begin(); f_iter != fields.end(); ++f_iter) {
    field_name = (*f_iter)->get_name();
    t_type* field_type = (*f_iter)->get_type();
    publicize_field_name = publicize(field_name);
    string goType = type_to_go_type_with_opt(field_type, is_pointer_field(*f_iter));

    string tgt = "p." + publicize_field_name;
    string src = "other." + publicize_field_name;
    t_type* ttype = field_type->get_true_type();
    // Compare field contents
    if (is_pointer_field(*f_iter)
        && (ttype->is_base_type() || ttype->is_enum() || ttype->is_container())) {
      string tgtv = "(*" + tgt + ")";
      string srcv = "(*" + src + ")";
      out << indent() << "if " << tgt << " != " << src << " {" << endl;
      indent_up();
      out << indent() << "if " << tgt << " == nil || " << src << " == nil {" << endl;
      indent_up();
      out << indent() << "return false" << endl;
      indent_down();
      out << indent() << "}" << endl;
      generate_go_equals(out, field_type, tgtv, srcv);
      indent_down();
      out << indent() << "}" << endl;
    } else {
      generate_go_equals(out, field_type, tgt, src);
    }
  }
  out << indent() << "return true" << endl;
  indent_down();
  out << indent() << "}" << endl << endl;
}

/**
 * Generates a thrift service.
 *
 * @param tservice The service definition
 */
void t_go_generator::generate_service(t_service* tservice) {
  string test_suffix("_test");
  string filename = lowercase(service_name_);
  string f_service_name;

  generate_service_interface(tservice);
  generate_service_client(tservice);
  generate_service_server(tservice);
  generate_service_helpers(tservice);
  if(!skip_remote_) {
    generate_service_remote(tservice);
  }
  f_types_ << endl;
}

/**
 * Generates helper functions for a service.
 *
 * @param tservice The service to generate a header definition for
 */
void t_go_generator::generate_service_helpers(t_service* tservice) {
  vector<t_function*> functions = tservice->get_functions();
  vector<t_function*>::iterator f_iter;
  f_types_ << "// HELPER FUNCTIONS AND STRUCTURES" << endl << endl;

  for (f_iter = functions.begin(); f_iter != functions.end(); ++f_iter) {
    t_struct* ts = (*f_iter)->get_arglist();
    generate_go_struct_definition(f_types_, ts, false, false, true);
    generate_go_function_helpers(*f_iter);
  }
}

/**
 * Generates a struct and helpers for a function.
 *
 * @param tfunction The function
 */
void t_go_generator::generate_go_function_helpers(t_function* tfunction) {
  if (!tfunction->is_oneway()) {
    t_struct result(program_, tfunction->get_name() + "_result");
    t_field success(tfunction->get_returntype(), "success", 0);
    success.set_req(t_field::T_OPTIONAL);

    if (!tfunction->get_returntype()->is_void()) {
      result.append(&success);
    }

    t_struct* xs = tfunction->get_xceptions();
    const vector<t_field*>& fields = xs->get_members();
    vector<t_field*>::const_iterator f_iter;

    for (f_iter = fields.begin(); f_iter != fields.end(); ++f_iter) {
      t_field* f = *f_iter;
      f->set_req(t_field::T_OPTIONAL);
      result.append(f);
    }

    generate_go_struct_definition(f_types_, &result, false, true);
  }
}

/**
 * Generates a service interface definition.
 *
 * @param tservice The service to generate a header definition for
 */
void t_go_generator::generate_service_interface(t_service* tservice) {
  string extends = "";
  string extends_if = "";
  string serviceName(publicize(tservice->get_name()));
  string interfaceName = serviceName;

  if (tservice->get_extends() != nullptr) {
    extends = type_name(tservice->get_extends());
    size_t index = extends.rfind(".");

    if (index != string::npos) {
      extends_if = "\n" + indent() + "  " + extends.substr(0, index + 1)
                   + publicize(extends.substr(index + 1)) + "\n";
    } else {
      extends_if = "\n" + indent() + publicize(extends) + "\n";
    }
  }

  f_types_ << indent() << "type " << interfaceName << " interface {" << extends_if;
  indent_up();
  generate_go_docstring(f_types_, tservice);
  vector<t_function*> functions = tservice->get_functions();

  if (!functions.empty()) {
    f_types_ << endl;
    vector<t_function*>::iterator f_iter;

    for (f_iter = functions.begin(); f_iter != functions.end(); ++f_iter) {
      generate_go_docstring(f_types_, (*f_iter));
      f_types_ << indent() << function_signature_if(*f_iter, "", true) << endl;
    }
  }

  indent_down();
  f_types_ << indent() << "}" << endl << endl;
}

/**
 * Generates a service client definition.
 *
 * @param tservice The service to generate a server for.
 */
void t_go_generator::generate_service_client(t_service* tservice) {
  string extends = "";
  string extends_field = "";
  string extends_client = "";
  string extends_client_new = "";
  string serviceName(publicize(tservice->get_name()));

  if (tservice->get_extends() != nullptr) {
    extends = type_name(tservice->get_extends());
    size_t index = extends.rfind(".");

    if (index != string::npos) {
      extends_client = extends.substr(0, index + 1) + publicize(extends.substr(index + 1))
                       + "Client";
      extends_client_new = extends.substr(0, index + 1) + "New"
                           + publicize(extends.substr(index + 1)) + "Client";
    } else {
      extends_client = publicize(extends) + "Client";
      extends_client_new = "New" + extends_client;
    }
  }

  extends_field = extends_client.substr(extends_client.find(".") + 1);

  generate_go_docstring(f_types_, tservice);
  f_types_ << indent() << "type " << serviceName << "Client struct {" << endl;
  indent_up();

  if (!extends_client.empty()) {
    f_types_ << indent() << "*" << extends_client << endl;
  } else {
    f_types_ << indent() << "c thrift.TClient" << endl;
    f_types_ << indent() << "meta thrift.ResponseMeta" << endl;
  }

  indent_down();
  f_types_ << indent() << "}" << endl << endl;

  // Legacy constructor function
  f_types_ << indent() << "func New" << serviceName
             << "ClientFactory(t thrift.TTransport, f thrift.TProtocolFactory) *" << serviceName
             << "Client {" << endl;
  indent_up();
  f_types_ << indent() << "return &" << serviceName << "Client";

  if (!extends.empty()) {
    f_types_ << "{" << extends_field << ": " << extends_client_new << "Factory(t, f)}";
  } else {
    indent_up();
    f_types_ << "{" << endl;
    f_types_ << indent() << "c: thrift.NewTStandardClient(f.GetProtocol(t), f.GetProtocol(t))," << endl;
    indent_down();
    f_types_ << indent() << "}" << endl;
  }

  indent_down();
  f_types_ << indent() << "}" << endl << endl;
  // Legacy constructor function with custom input & output protocols
  f_types_
      << indent() << "func New" << serviceName
      << "ClientProtocol(t thrift.TTransport, iprot thrift.TProtocol, oprot thrift.TProtocol) *"
      << serviceName << "Client {" << endl;
  indent_up();
  f_types_ << indent() << "return &" << serviceName << "Client";

  if (!extends.empty()) {
    f_types_ << "{" << extends_field << ": " << extends_client_new << "Protocol(t, iprot, oprot)}"
               << endl;
  } else {
    indent_up();
    f_types_ << "{" << endl;
    f_types_ << indent() << "c: thrift.NewTStandardClient(iprot, oprot)," << endl;
    indent_down();
    f_types_ << indent() << "}" << endl;
  }

  indent_down();
  f_types_ << indent() << "}" << endl << endl;

  // Constructor function
  f_types_ << indent() << "func New" << serviceName
    << "Client(c thrift.TClient) *" << serviceName << "Client {" << endl;
  indent_up();
  f_types_ << indent() << "return &" << serviceName << "Client{" << endl;

  indent_up();
  if (!extends.empty()) {
    f_types_ << indent() << extends_field << ": " << extends_client_new << "(c)," << endl;
  } else {
    f_types_ << indent() << "c: c," << endl;
  }
  indent_down();
  f_types_ << indent() << "}" << endl;

  indent_down();
  f_types_ << indent() << "}" << endl << endl;

  if (extends.empty()) {
    f_types_ << indent() << "func (p *" << serviceName << "Client) Client_() thrift.TClient {" << endl;
    indent_up();
    f_types_ << indent() << "return p.c" << endl;
    indent_down();
    f_types_ << indent() << "}" << endl << endl;

    f_types_ << indent() << "func (p *" << serviceName << "Client) LastResponseMeta_() thrift.ResponseMeta {" << endl;
    indent_up();
    f_types_ << indent() << "return p.meta" << endl;
    indent_down();
    f_types_ << indent() << "}" << endl << endl;

    f_types_ << indent() << "func (p *" << serviceName << "Client) SetLastResponseMeta_(meta thrift.ResponseMeta) {" << endl;
    indent_up();
    f_types_ << indent() << "p.meta = meta" << endl;
    indent_down();
    f_types_ << indent() << "}" << endl << endl;
  }

  // Generate client method implementations
  vector<t_function*> functions = tservice->get_functions();
  vector<t_function*>::const_iterator f_iter;

  for (f_iter = functions.begin(); f_iter != functions.end(); ++f_iter) {
    t_struct* arg_struct = (*f_iter)->get_arglist();
    const vector<t_field*>& fields = arg_struct->get_members();
    vector<t_field*>::const_iterator fld_iter;
    string funname = publicize((*f_iter)->get_name());
    // Open function
    generate_go_docstring(f_types_, (*f_iter));
    f_types_ << indent() << "func (p *" << serviceName << "Client) "
               << function_signature_if(*f_iter, "", true) << " {" << endl;
    indent_up();

    std::string method = (*f_iter)->get_name();
    std::string argsType = publicize(method + "_args", true);
    std::string argsName = tmp("_args");
    f_types_ << indent() << "var " << argsName << " " << argsType << endl;

    for (fld_iter = fields.begin(); fld_iter != fields.end(); ++fld_iter) {
      f_types_ << indent() << argsName << "." << publicize((*fld_iter)->get_name())
               << " = " << variable_name_to_go_name((*fld_iter)->get_name()) << endl;
    }

    if (!(*f_iter)->is_oneway()) {
      std::string metaName = tmp("_meta");
      std::string resultName = tmp("_result");
      std::string resultType = publicize(method + "_result", true);
      f_types_ << indent() << "var " << resultName << " " << resultType << endl;
      f_types_ << indent() << "var " << metaName << " thrift.ResponseMeta" << endl;
      f_types_ << indent() << metaName << ", _err = p.Client_().Call(ctx, \""
        << method << "\", &" << argsName << ", &" << resultName << ")" << endl;
      f_types_ << indent() << "p.SetLastResponseMeta_(" << metaName << ")" << endl;
      f_types_ << indent() << "if _err != nil {" << endl;

      indent_up();
      f_types_ << indent() << "return" << endl;
      indent_down();
      f_types_ << indent() << "}" << endl;

      t_struct* xs = (*f_iter)->get_xceptions();
      const std::vector<t_field*>& xceptions = xs->get_members();
      vector<t_field*>::const_iterator x_iter;

      if (!xceptions.empty()) {
        f_types_ << indent() << "switch {" << endl;

        for (x_iter = xceptions.begin(); x_iter != xceptions.end(); ++x_iter) {
          const std::string pubname = publicize((*x_iter)->get_name());
          const std::string field = resultName + "." + pubname;

          f_types_ << indent() << "case " << field << "!= nil:" << endl;
          indent_up();

          if (!(*f_iter)->get_returntype()->is_void()) {
            f_types_ << indent() << "return _r, " << field << endl;
          } else {
            f_types_ << indent() << "return "<< field << endl;
          }

          indent_down();
        }

        f_types_ << indent() << "}" << endl << endl;
      }

      if ((*f_iter)->get_returntype()->is_struct()) {
        // Check if the result is nil, which likely means we have a new
        // exception added but unknown to the client yet
        // (e.g. client hasn't updated the thrift file).
        // Sadly this check can only be reliable done when the return type is a
        // struct in go.
        std::string retName = tmp("_ret");
        f_types_ << indent() << "if " << retName << " := " << resultName
                 << ".GetSuccess(); " << retName << " != nil {" << endl;
        indent_up();
        f_types_ << indent() << "return " << retName << ", nil" << endl;
        indent_down();
        f_types_ << indent() << "}" << endl;
        f_types_ << indent() << "return nil, "
                 << "thrift.NewTApplicationException(thrift.MISSING_RESULT, \""
                 << method << " failed: unknown result\")" << endl;
      } else if (!(*f_iter)->get_returntype()->is_void()) {
        f_types_ << indent() << "return " << resultName << ".GetSuccess(), nil" << endl;
      } else {
        f_types_ << indent() << "return nil" << endl;
      }
    } else {
      // Since we don't have response meta for oneway calls, overwrite it with
      // an empty one to avoid users getting the meta from last call and
      // mistaken it as from the oneway call.
      f_types_ << indent() << "p.SetLastResponseMeta_(thrift.ResponseMeta{})" << endl;
      // TODO: would be nice to not to duplicate the call generation
      f_types_ << indent() << "if _, err := p.Client_().Call(ctx, \""
        << method << "\", &" << argsName << ", nil); err != nil {" << endl;

      indent_up();
      f_types_ << indent() << "return err" << endl;
      indent_down();
      f_types_ << indent() << "}" << endl;
      f_types_ << indent() << "return nil" << endl;
    }

    indent_down();
    f_types_ << "}" << endl << endl;
  }
}

/**
 * Generates a command line tool for making remote requests
 *
 * @param tservice The service to generate a remote for.
 */
void t_go_generator::generate_service_remote(t_service* tservice) {
  vector<t_function*> functions;
  std::unordered_map<std::string, std::string> func_to_service;

  // collect all functions including inherited functions
  t_service* parent = tservice;
  while (parent != nullptr) {
    vector<t_function*> p_functions = parent->get_functions();
    functions.insert(functions.end(), p_functions.begin(), p_functions.end());

    // We need to maintain a map of functions names to the name of their parent.
    // This is because functions may come from a parent service, and if we need
    // to create the arguments struct (e.g. `NewParentServiceNameFuncNameArgs()`)
    // we need to make sure to specify the correct service name.
    for (vector<t_function*>::iterator f_iter = p_functions.begin(); f_iter != p_functions.end(); ++f_iter) {
      auto it = func_to_service.find((*f_iter)->get_name());
      if (it == func_to_service.end()) {
        func_to_service.emplace((*f_iter)->get_name(), parent->get_name());
      }
    }

    parent = parent->get_extends();
  }

  // This file is not useful if there are no functions; don't generate it
  if (functions.size() == 0) {
    return;
  }

  string f_remote_dir = package_dir_ + "/" + underscore(service_name_) + "-remote";
  MKDIR(f_remote_dir.c_str());

  vector<t_function*>::iterator f_iter;
  string f_remote_name = f_remote_dir + "/"
                         + underscore(service_name_) + "-remote.go";
  ofstream_with_content_based_conditional_update f_remote;
  f_remote.open(f_remote_name.c_str());

  string unused_protection;

  std::vector<string> system_packages;
  system_packages.push_back("context");
  system_packages.push_back("flag");
  system_packages.push_back("fmt");
  system_packages.push_back("math");
  system_packages.push_back("net");
  system_packages.push_back("net/url");
  system_packages.push_back("os");
  system_packages.push_back("strconv");
  system_packages.push_back("strings");
  // For the thrift import, always do rename import to make sure it's called thrift.
  system_packages.push_back("thrift \"" + gen_thrift_import_ + "\"");

  f_remote << go_autogen_comment();
  f_remote << indent() << "package main" << endl << endl;
  f_remote << indent() << "import (" << endl;
  f_remote << render_system_packages(system_packages);
  f_remote << indent() << render_included_programs(unused_protection);
  f_remote << render_program_import(program_, unused_protection);
  f_remote << indent() << ")" << endl;
  f_remote << indent() << endl;
  f_remote << indent() << unused_protection; // filled in render_included_programs()
  f_remote << indent() << endl;
  f_remote << indent() << "func Usage() {" << endl;
  f_remote << indent() << "  fmt.Fprintln(os.Stderr, \"Usage of \", os.Args[0], \" "
                          "[-h host:port] [-u url] [-f[ramed]] function [arg1 [arg2...]]:\")"
           << endl;
  f_remote << indent() << "  flag.PrintDefaults()" << endl;
  f_remote << indent() << "  fmt.Fprintln(os.Stderr, \"\\nFunctions:\")" << endl;

  string package_name_aliased = package_identifiers_[get_real_go_module(program_)];

  for (f_iter = functions.begin(); f_iter != functions.end(); ++f_iter) {
    f_remote << "  fmt.Fprintln(os.Stderr, \"  " << (*f_iter)->get_returntype()->get_name() << " "
             << (*f_iter)->get_name() << "(";
    t_struct* arg_struct = (*f_iter)->get_arglist();
    const std::vector<t_field*>& args = arg_struct->get_members();
    std::vector<t_field*>::size_type num_args = args.size();
    bool first = true;

    for (std::vector<t_field*>::size_type i = 0; i < num_args; ++i) {
      if (first) {
        first = false;
      } else {
        f_remote << ", ";
      }

      f_remote << args[i]->get_type()->get_name() << " " << args[i]->get_name();
    }

    f_remote << ")\")" << endl;
  }

  f_remote << indent() << "  fmt.Fprintln(os.Stderr)" << endl;
  f_remote << indent() << "  os.Exit(0)" << endl;
  f_remote << indent() << "}" << endl;
  f_remote << indent() << endl;

  f_remote << indent() << "type httpHeaders map[string]string" << endl;
  f_remote << indent() << endl;
  f_remote << indent() << "func (h httpHeaders) String() string {" << endl;
  f_remote << indent() << "  var m map[string]string = h" << endl;
  f_remote << indent() << "  return fmt.Sprintf(\"%s\", m)" << endl;
  f_remote << indent() << "}" << endl;
  f_remote << indent() << endl;
  f_remote << indent() << "func (h httpHeaders) Set(value string) error {" << endl;
  f_remote << indent() << "  parts := strings.Split(value, \": \")" << endl;
  f_remote << indent() << "  if len(parts) != 2 {" << endl;
  f_remote << indent() << "    return fmt.Errorf(\"header should be of format 'Key: Value'\")" << endl;
  f_remote << indent() << "  }" << endl;
  f_remote << indent() << "  h[parts[0]] = parts[1]" << endl;
  f_remote << indent() << "  return nil" << endl;
  f_remote << indent() << "}" << endl;
  f_remote << indent() << endl;

  f_remote << indent() << "func main() {" << endl;
  indent_up();
  f_remote << indent() << "flag.Usage = Usage" << endl;
  f_remote << indent() << "var host string" << endl;
  f_remote << indent() << "var port int" << endl;
  f_remote << indent() << "var protocol string" << endl;
  f_remote << indent() << "var urlString string" << endl;
  f_remote << indent() << "var framed bool" << endl;
  f_remote << indent() << "var useHttp bool" << endl;
  f_remote << indent() << "headers := make(httpHeaders)" << endl;
  f_remote << indent() << "var parsedUrl *url.URL" << endl;
  f_remote << indent() << "var trans thrift.TTransport" << endl;
  f_remote << indent() << "_ = strconv.Atoi" << endl;
  f_remote << indent() << "_ = math.Abs" << endl;
  f_remote << indent() << "flag.Usage = Usage" << endl;
  f_remote << indent() << "flag.StringVar(&host, \"h\", \"localhost\", \"Specify host and port\")"
           << endl;
  f_remote << indent() << "flag.IntVar(&port, \"p\", 9090, \"Specify port\")" << endl;
  f_remote << indent() << "flag.StringVar(&protocol, \"P\", \"binary\", \""
                          "Specify the protocol (binary, compact, simplejson, json)\")" << endl;
  f_remote << indent() << "flag.StringVar(&urlString, \"u\", \"\", \"Specify the url\")" << endl;
  f_remote << indent() << "flag.BoolVar(&framed, \"framed\", false, \"Use framed transport\")"
           << endl;
  f_remote << indent() << "flag.BoolVar(&useHttp, \"http\", false, \"Use http\")" << endl;
  f_remote << indent() << "flag.Var(headers, \"H\", \"Headers to set on the http(s) request (e.g. -H \\\"Key: Value\\\")\")" << endl;
  f_remote << indent() << "flag.Parse()" << endl;
  f_remote << indent() << endl;
  f_remote << indent() << "if len(urlString) > 0 {" << endl;
  f_remote << indent() << "  var err error" << endl;
  f_remote << indent() << "  parsedUrl, err = url.Parse(urlString)" << endl;
  f_remote << indent() << "  if err != nil {" << endl;
  f_remote << indent() << "    fmt.Fprintln(os.Stderr, \"Error parsing URL: \", err)" << endl;
  f_remote << indent() << "    flag.Usage()" << endl;
  f_remote << indent() << "  }" << endl;
  f_remote << indent() << "  host = parsedUrl.Host" << endl;
  f_remote << indent() << "  useHttp = len(parsedUrl.Scheme) <= 0 || parsedUrl.Scheme == \"http\" || parsedUrl.Scheme == \"https\""
           << endl;
  f_remote << indent() << "} else if useHttp {" << endl;
  f_remote << indent() << "  _, err := url.Parse(fmt.Sprint(\"http://\", host, \":\", port))"
           << endl;
  f_remote << indent() << "  if err != nil {" << endl;
  f_remote << indent() << "    fmt.Fprintln(os.Stderr, \"Error parsing URL: \", err)" << endl;
  f_remote << indent() << "    flag.Usage()" << endl;
  f_remote << indent() << "  }" << endl;
  f_remote << indent() << "}" << endl;
  f_remote << indent() << endl;
  f_remote << indent() << "cmd := flag.Arg(0)" << endl;
  f_remote << indent() << "var err error" << endl;
  f_remote << indent() << "var cfg *thrift.TConfiguration = nil" << endl;
  f_remote << indent() << "if useHttp {" << endl;
  f_remote << indent() << "  trans, err = thrift.NewTHttpClient(parsedUrl.String())" << endl;
  f_remote << indent() << "  if len(headers) > 0 {" << endl;
  f_remote << indent() << "    httptrans := trans.(*thrift.THttpClient)" << endl;
  f_remote << indent() << "    for key, value := range headers {" << endl;
  f_remote << indent() << "      httptrans.SetHeader(key, value)" << endl;
  f_remote << indent() << "    }" << endl;
  f_remote << indent() << "  }" << endl;
  f_remote << indent() << "} else {" << endl;
  f_remote << indent() << "  portStr := fmt.Sprint(port)" << endl;
  f_remote << indent() << "  if strings.Contains(host, \":\") {" << endl;
  f_remote << indent() << "         host, portStr, err = net.SplitHostPort(host)" << endl;
  f_remote << indent() << "         if err != nil {" << endl;
  f_remote << indent() << "                 fmt.Fprintln(os.Stderr, \"error with host:\", err)"
           << endl;
  f_remote << indent() << "                 os.Exit(1)" << endl;
  f_remote << indent() << "         }" << endl;
  f_remote << indent() << "  }" << endl;
  f_remote << indent() << "  trans = thrift.NewTSocketConf(net.JoinHostPort(host, portStr), cfg)" << endl;
  f_remote << indent() << "  if err != nil {" << endl;
  f_remote << indent() << "    fmt.Fprintln(os.Stderr, \"error resolving address:\", err)" << endl;
  f_remote << indent() << "    os.Exit(1)" << endl;
  f_remote << indent() << "  }" << endl;
  f_remote << indent() << "  if framed {" << endl;
  f_remote << indent() << "    trans = thrift.NewTFramedTransportConf(trans, cfg)" << endl;
  f_remote << indent() << "  }" << endl;
  f_remote << indent() << "}" << endl;
  f_remote << indent() << "if err != nil {" << endl;
  f_remote << indent() << "  fmt.Fprintln(os.Stderr, \"Error creating transport\", err)" << endl;
  f_remote << indent() << "  os.Exit(1)" << endl;
  f_remote << indent() << "}" << endl;
  f_remote << indent() << "defer trans.Close()" << endl;
  f_remote << indent() << "var protocolFactory thrift.TProtocolFactory" << endl;
  f_remote << indent() << "switch protocol {" << endl;
  f_remote << indent() << "case \"compact\":" << endl;
  f_remote << indent() << "  protocolFactory = thrift.NewTCompactProtocolFactoryConf(cfg)" << endl;
  f_remote << indent() << "  break" << endl;
  f_remote << indent() << "case \"simplejson\":" << endl;
  f_remote << indent() << "  protocolFactory = thrift.NewTSimpleJSONProtocolFactoryConf(cfg)" << endl;
  f_remote << indent() << "  break" << endl;
  f_remote << indent() << "case \"json\":" << endl;
  f_remote << indent() << "  protocolFactory = thrift.NewTJSONProtocolFactory()" << endl;
  f_remote << indent() << "  break" << endl;
  f_remote << indent() << "case \"binary\", \"\":" << endl;
  f_remote << indent() << "  protocolFactory = thrift.NewTBinaryProtocolFactoryConf(cfg)" << endl;
  f_remote << indent() << "  break" << endl;
  f_remote << indent() << "default:" << endl;
  f_remote << indent() << "  fmt.Fprintln(os.Stderr, \"Invalid protocol specified: \", protocol)"
           << endl;
  f_remote << indent() << "  Usage()" << endl;
  f_remote << indent() << "  os.Exit(1)" << endl;
  f_remote << indent() << "}" << endl;
  f_remote << indent() << "iprot := protocolFactory.GetProtocol(trans)" << endl;
  f_remote << indent() << "oprot := protocolFactory.GetProtocol(trans)" << endl;
  f_remote << indent() << "client := " << package_name_aliased << ".New" << publicize(service_name_)
           << "Client(thrift.NewTStandardClient(iprot, oprot))" << endl;
  f_remote << indent() << "if err := trans.Open(); err != nil {" << endl;
  f_remote << indent() << "  fmt.Fprintln(os.Stderr, \"Error opening socket to \", "
                          "host, \":\", port, \" \", err)" << endl;
  f_remote << indent() << "  os.Exit(1)" << endl;
  f_remote << indent() << "}" << endl;
  f_remote << indent() << endl;
  f_remote << indent() << "switch cmd {" << endl;

  for (f_iter = functions.begin(); f_iter != functions.end(); ++f_iter) {
    t_struct* arg_struct = (*f_iter)->get_arglist();
    const std::vector<t_field*>& args = arg_struct->get_members();
    std::vector<t_field*>::size_type num_args = args.size();
    string funcName((*f_iter)->get_name());
    string pubName(publicize(funcName));
    string argumentsName(publicize(funcName + "_args", true, func_to_service[funcName]));
    f_remote << indent() << "case \"" << escape_string(funcName) << "\":" << endl;
    indent_up();
    f_remote << indent() << "if flag.NArg() - 1 != " << num_args << " {" << endl;
    f_remote << indent() << "  fmt.Fprintln(os.Stderr, \"" << escape_string(pubName) << " requires "
             << num_args << " args\")" << endl;
    f_remote << indent() << "  flag.Usage()" << endl;
    f_remote << indent() << "}" << endl;

    for (std::vector<t_field*>::size_type i = 0; i < num_args; ++i) {
      std::vector<t_field*>::size_type flagArg = i + 1;
      t_type* the_type(args[i]->get_type());
      t_type* the_type2(get_true_type(the_type));

      if (the_type2->is_enum()) {
        f_remote << indent() << "tmp" << i << ", err := (strconv.Atoi(flag.Arg(" << flagArg << ")))"
                 << endl;
        f_remote << indent() << "if err != nil {" << endl;
        f_remote << indent() << "  Usage()" << endl;
        f_remote << indent() << " return" << endl;
        f_remote << indent() << "}" << endl;
        f_remote << indent() << "argvalue" << i << " := " << package_name_aliased << "."
                 << publicize(the_type->get_name()) << "(tmp" << i << ")" << endl;
      } else if (the_type2->is_base_type()) {
        t_base_type::t_base e = ((t_base_type*)the_type2)->get_base();
        string err(tmp("err"));

        switch (e) {
        case t_base_type::TYPE_VOID:
          break;

        case t_base_type::TYPE_STRING:
          if (the_type2->is_binary()) {
            f_remote << indent() << "argvalue" << i << " := []byte(flag.Arg(" << flagArg << "))"
                     << endl;
          } else {
            f_remote << indent() << "argvalue" << i << " := flag.Arg(" << flagArg << ")" << endl;
          }
          break;

        case t_base_type::TYPE_BOOL:
          f_remote << indent() << "argvalue" << i << " := flag.Arg(" << flagArg << ") == \"true\""
                   << endl;
          break;

        case t_base_type::TYPE_I8:
          f_remote << indent() << "tmp" << i << ", " << err << " := (strconv.Atoi(flag.Arg("
                   << flagArg << ")))" << endl;
          f_remote << indent() << "if " << err << " != nil {" << endl;
          f_remote << indent() << "  Usage()" << endl;
          f_remote << indent() << "  return" << endl;
          f_remote << indent() << "}" << endl;
          f_remote << indent() << "argvalue" << i << " := int8(tmp" << i << ")" << endl;
          break;

        case t_base_type::TYPE_I16:
          f_remote << indent() << "tmp" << i << ", " << err << " := (strconv.Atoi(flag.Arg("
                   << flagArg << ")))" << endl;
          f_remote << indent() << "if " << err << " != nil {" << endl;
          f_remote << indent() << "  Usage()" << endl;
          f_remote << indent() << "  return" << endl;
          f_remote << indent() << "}" << endl;
          f_remote << indent() << "argvalue" << i << " := int16(tmp" << i << ")" << endl;
          break;

        case t_base_type::TYPE_I32:
          f_remote << indent() << "tmp" << i << ", " << err << " := (strconv.Atoi(flag.Arg("
                   << flagArg << ")))" << endl;
          f_remote << indent() << "if " << err << " != nil {" << endl;
          f_remote << indent() << "  Usage()" << endl;
          f_remote << indent() << "  return" << endl;
          f_remote << indent() << "}" << endl;
          f_remote << indent() << "argvalue" << i << " := int32(tmp" << i << ")" << endl;
          break;

        case t_base_type::TYPE_I64:
          f_remote << indent() << "argvalue" << i << ", " << err
                   << " := (strconv.ParseInt(flag.Arg(" << flagArg << "), 10, 64))" << endl;
          f_remote << indent() << "if " << err << " != nil {" << endl;
          f_remote << indent() << "  Usage()" << endl;
          f_remote << indent() << "  return" << endl;
          f_remote << indent() << "}" << endl;
          break;

        case t_base_type::TYPE_DOUBLE:
          f_remote << indent() << "argvalue" << i << ", " << err
                   << " := (strconv.ParseFloat(flag.Arg(" << flagArg << "), 64))" << endl;
          f_remote << indent() << "if " << err << " != nil {" << endl;
          f_remote << indent() << "  Usage()" << endl;
          f_remote << indent() << "  return" << endl;
          f_remote << indent() << "}" << endl;
          break;

        default:
          throw("Invalid base type in generate_service_remote");
        }

        // f_remote << publicize(args[i]->get_name()) << "(strconv.Atoi(flag.Arg(" << flagArg <<
        // ")))";
      } else if (the_type2->is_struct()) {
        string arg(tmp("arg"));
        string mbTrans(tmp("mbTrans"));
        string err1(tmp("err"));
        string factory(tmp("factory"));
        string jsProt(tmp("jsProt"));
        string err2(tmp("err"));
        std::string tstruct_name(publicize(the_type->get_name()));
        std::string tstruct_module( module_name(the_type));
        if(tstruct_module.empty()) {
          tstruct_module = package_name_aliased;
        }

        f_remote << indent() << arg << " := flag.Arg(" << flagArg << ")" << endl;
        f_remote << indent() << mbTrans << " := thrift.NewTMemoryBufferLen(len(" << arg << "))"
                 << endl;
        f_remote << indent() << "defer " << mbTrans << ".Close()" << endl;
        f_remote << indent() << "_, " << err1 << " := " << mbTrans << ".WriteString(" << arg << ")"
                 << endl;
        f_remote << indent() << "if " << err1 << " != nil {" << endl;
        f_remote << indent() << "  Usage()" << endl;
        f_remote << indent() << "  return" << endl;
        f_remote << indent() << "}" << endl;
        f_remote << indent() << factory << " := thrift.NewTJSONProtocolFactory()" << endl;
        f_remote << indent() << jsProt << " := " << factory << ".GetProtocol(" << mbTrans << ")"
                 << endl;
        f_remote << indent() << "argvalue" << i << " := " << tstruct_module << ".New" << tstruct_name
                 << "()" << endl;
        f_remote << indent() << err2 << " := argvalue" << i << "." << read_method_name_ <<  "(context.Background(), " << jsProt << ")" << endl;
        f_remote << indent() << "if " << err2 << " != nil {" << endl;
        f_remote << indent() << "  Usage()" << endl;
        f_remote << indent() << "  return" << endl;
        f_remote << indent() << "}" << endl;
      } else if (the_type2->is_container() || the_type2->is_xception()) {
        string arg(tmp("arg"));
        string mbTrans(tmp("mbTrans"));
        string err1(tmp("err"));
        string factory(tmp("factory"));
        string jsProt(tmp("jsProt"));
        string err2(tmp("err"));
        std::string argName(publicize(args[i]->get_name()));
        f_remote << indent() << arg << " := flag.Arg(" << flagArg << ")" << endl;
        f_remote << indent() << mbTrans << " := thrift.NewTMemoryBufferLen(len(" << arg << "))"
                 << endl;
        f_remote << indent() << "defer " << mbTrans << ".Close()" << endl;
        f_remote << indent() << "_, " << err1 << " := " << mbTrans << ".WriteString(" << arg << ")"
                 << endl;
        f_remote << indent() << "if " << err1 << " != nil { " << endl;
        f_remote << indent() << "  Usage()" << endl;
        f_remote << indent() << "  return" << endl;
        f_remote << indent() << "}" << endl;
        f_remote << indent() << factory << " := thrift.NewTJSONProtocolFactory()" << endl;
        f_remote << indent() << jsProt << " := " << factory << ".GetProtocol(" << mbTrans << ")"
                 << endl;
        f_remote << indent() << "containerStruct" << i << " := " << package_name_aliased << ".New"
                 << argumentsName << "()" << endl;
        f_remote << indent() << err2 << " := containerStruct" << i << ".ReadField" << (i + 1) << "(context.Background(), "
                 << jsProt << ")" << endl;
        f_remote << indent() << "if " << err2 << " != nil {" << endl;
        f_remote << indent() << "  Usage()" << endl;
        f_remote << indent() << "  return" << endl;
        f_remote << indent() << "}" << endl;
        f_remote << indent() << "argvalue" << i << " := containerStruct" << i << "." << argName
                 << endl;
      } else {
        throw("Invalid argument type in generate_service_remote");
      }

      if (the_type->is_typedef()) {
        std::string typedef_module(module_name(the_type));
        if(typedef_module.empty()) {
          typedef_module = package_name_aliased;
        }
        f_remote << indent() << "value" << i << " := " << typedef_module << "."
                 << publicize(the_type->get_name()) << "(argvalue" << i << ")" << endl;
      } else {
        f_remote << indent() << "value" << i << " := argvalue" << i << endl;
      }
    }

    f_remote << indent() << "fmt.Print(client." << pubName << "(";
    bool argFirst = true;

    f_remote << "context.Background()";
    for (std::vector<t_field*>::size_type i = 0; i < num_args; ++i) {
      if (argFirst) {
        argFirst = false;
        f_remote << ", ";
      } else {
        f_remote << ", ";
      }

      if (args[i]->get_type()->is_enum()) {
        f_remote << "value" << i;
      } else if (args[i]->get_type()->is_base_type()) {
        t_base_type::t_base e = ((t_base_type*)(args[i]->get_type()))->get_base();

        switch (e) {
        case t_base_type::TYPE_VOID:
          break;

        case t_base_type::TYPE_STRING:
        case t_base_type::TYPE_BOOL:
        case t_base_type::TYPE_I8:
        case t_base_type::TYPE_I16:
        case t_base_type::TYPE_I32:
        case t_base_type::TYPE_I64:
        case t_base_type::TYPE_DOUBLE:
          f_remote << "value" << i;
          break;

        default:
          throw("Invalid base type in generate_service_remote");
        }

        // f_remote << publicize(args[i]->get_name()) << "(strconv.Atoi(flag.Arg(" << flagArg <<
        // ")))";
      } else {
        f_remote << "value" << i;
      }
    }

    f_remote << "))" << endl;
    f_remote << indent() << "fmt.Print(\"\\n\")" << endl;
    f_remote << indent() << "break" << endl;
    indent_down();
  }

  f_remote << indent() << "case \"\":" << endl;
  f_remote << indent() << "  Usage()" << endl;
  f_remote << indent() << "  break" << endl;
  f_remote << indent() << "default:" << endl;
  f_remote << indent() << "  fmt.Fprintln(os.Stderr, \"Invalid function \", cmd)" << endl;
  f_remote << indent() << "}" << endl;
  indent_down();
  f_remote << indent() << "}" << endl;
  // Close service file
  f_remote.close();
  format_go_output(f_remote_name);
#ifndef _MSC_VER
  // Make file executable, love that bitwise OR action
  chmod(f_remote_name.c_str(),
        S_IRUSR | S_IWUSR | S_IXUSR
#ifndef _WIN32
        | S_IRGRP | S_IXGRP | S_IROTH | S_IXOTH
#endif
        );
#endif
}

/**
 * Generates a service server definition.
 *
 * @param tservice The service to generate a server for.
 */
void t_go_generator::generate_service_server(t_service* tservice) {
  // Generate the dispatch methods
  vector<t_function*> functions = tservice->get_functions();
  vector<t_function*>::iterator f_iter;
  string extends = "";
  string extends_processor = "";
  string extends_processor_new = "";
  string serviceName(publicize(tservice->get_name()));

  if (tservice->get_extends() != nullptr) {
    extends = type_name(tservice->get_extends());
    size_t index = extends.rfind(".");

    if (index != string::npos) {
      extends_processor = extends.substr(0, index + 1) + publicize(extends.substr(index + 1))
                          + "Processor";
      extends_processor_new = extends.substr(0, index + 1) + "New"
                              + publicize(extends.substr(index + 1)) + "Processor";
    } else {
      extends_processor = publicize(extends) + "Processor";
      extends_processor_new = "New" + extends_processor;
    }
  }

  string pServiceName(privatize(tservice->get_name()));
  // Generate the header portion
  string self(tmp("self"));

  if (extends_processor.empty()) {
    f_types_ << indent() << "type " << serviceName << "Processor struct {" << endl;
    f_types_ << indent() << "  processorMap map[string]thrift.TProcessorFunction" << endl;
    f_types_ << indent() << "  handler " << serviceName << endl;
    f_types_ << indent() << "}" << endl << endl;
    f_types_ << indent() << "func (p *" << serviceName
               << "Processor) AddToProcessorMap(key string, processor thrift.TProcessorFunction) {"
               << endl;
    f_types_ << indent() << "  p.processorMap[key] = processor" << endl;
    f_types_ << indent() << "}" << endl << endl;
    f_types_ << indent() << "func (p *" << serviceName
               << "Processor) GetProcessorFunction(key string) "
                  "(processor thrift.TProcessorFunction, ok bool) {" << endl;
    f_types_ << indent() << "  processor, ok = p.processorMap[key]" << endl;
    f_types_ << indent() << "  return processor, ok" << endl;
    f_types_ << indent() << "}" << endl << endl;
    f_types_ << indent() << "func (p *" << serviceName
               << "Processor) ProcessorMap() map[string]thrift.TProcessorFunction {" << endl;
    f_types_ << indent() << "  return p.processorMap" << endl;
    f_types_ << indent() << "}" << endl << endl;
    f_types_ << indent() << "func New" << serviceName << "Processor(handler " << serviceName
               << ") *" << serviceName << "Processor {" << endl << endl;
    f_types_
        << indent() << "  " << self << " := &" << serviceName
        << "Processor{handler:handler, processorMap:make(map[string]thrift.TProcessorFunction)}"
        << endl;

    for (f_iter = functions.begin(); f_iter != functions.end(); ++f_iter) {
      string escapedFuncName(escape_string((*f_iter)->get_name()));
      f_types_ << indent() << "  " << self << ".processorMap[\"" << escapedFuncName << "\"] = &"
                 << pServiceName << "Processor" << publicize((*f_iter)->get_name())
                 << "{handler:handler}" << endl;
    }

    string x(tmp("x"));
    f_types_ << indent() << "return " << self << endl;
    f_types_ << indent() << "}" << endl << endl;
    f_types_ << indent() << "func (p *" << serviceName
               << "Processor) Process(ctx context.Context, iprot, oprot thrift.TProtocol) (success bool, err "
                  "thrift.TException) {" << endl;
    f_types_ << indent() << "  name, _, seqId, err2 := iprot.ReadMessageBegin(ctx)" << endl;
    f_types_ << indent() << "  if err2 != nil { return false, thrift.WrapTException(err2) }" << endl;
    f_types_ << indent() << "  if processor, ok := p.GetProcessorFunction(name); ok {" << endl;
    f_types_ << indent() << "    return processor.Process(ctx, seqId, iprot, oprot)" << endl;
    f_types_ << indent() << "  }" << endl;
    f_types_ << indent() << "  iprot.Skip(ctx, thrift.STRUCT)" << endl;
    f_types_ << indent() << "  iprot.ReadMessageEnd(ctx)" << endl;
    f_types_ << indent() << "  " << x
               << " := thrift.NewTApplicationException(thrift.UNKNOWN_METHOD, \"Unknown function "
                  "\" + name)" << endl;
    f_types_ << indent() << "  oprot.WriteMessageBegin(ctx, name, thrift.EXCEPTION, seqId)" << endl;
    f_types_ << indent() << "  " << x << ".Write(ctx, oprot)" << endl;
    f_types_ << indent() << "  oprot.WriteMessageEnd(ctx)" << endl;
    f_types_ << indent() << "  oprot.Flush(ctx)" << endl;
    f_types_ << indent() << "  return false, " << x << endl;
    f_types_ << indent() << "" << endl;
    f_types_ << indent() << "}" << endl << endl;
  } else {
    f_types_ << indent() << "type " << serviceName << "Processor struct {" << endl;
    f_types_ << indent() << "  *" << extends_processor << endl;
    f_types_ << indent() << "}" << endl << endl;
    f_types_ << indent() << "func New" << serviceName << "Processor(handler " << serviceName
               << ") *" << serviceName << "Processor {" << endl;
    f_types_ << indent() << "  " << self << " := &" << serviceName << "Processor{"
               << extends_processor_new << "(handler)}" << endl;

    for (f_iter = functions.begin(); f_iter != functions.end(); ++f_iter) {
      string escapedFuncName(escape_string((*f_iter)->get_name()));
      f_types_ << indent() << "  " << self << ".AddToProcessorMap(\"" << escapedFuncName
                 << "\", &" << pServiceName << "Processor" << publicize((*f_iter)->get_name())
                 << "{handler:handler})" << endl;
    }

    f_types_ << indent() << "  return " << self << endl;
    f_types_ << indent() << "}" << endl << endl;
  }

  // Generate the process subfunctions
  for (f_iter = functions.begin(); f_iter != functions.end(); ++f_iter) {
    generate_process_function(tservice, *f_iter);
  }

  f_types_ << endl;
}

/**
 * Generates a process function definition.
 *
 * @param tfunction The function to write a dispatcher for
 */
void t_go_generator::generate_process_function(t_service* tservice, t_function* tfunction) {
  // Open function
  string processorName = privatize(tservice->get_name()) + "Processor"
                         + publicize(tfunction->get_name());
  string argsname = publicize(tfunction->get_name() + "_args", true);
  string resultname = publicize(tfunction->get_name() + "_result", true);

  // t_struct* xs = tfunction->get_xceptions();
  // const std::vector<t_field*>& xceptions = xs->get_members();
  f_types_ << indent() << "type " << processorName << " struct {" << endl;
  f_types_ << indent() << "  handler " << publicize(tservice->get_name()) << endl;
  f_types_ << indent() << "}" << endl << endl;
  f_types_ << indent() << "func (p *" << processorName
             << ") Process(ctx context.Context, seqId int32, iprot, oprot thrift.TProtocol) (success bool, err "
                "thrift.TException) {" << endl;
  indent_up();
  string write_err;
  if (!tfunction->is_oneway()) {
    write_err = tmp("_write_err");
    f_types_ << indent() << "var " << write_err << " error" << endl;
  }
  f_types_ << indent() << "args := " << argsname << "{}" << endl;
  f_types_ << indent() << "if err2 := args." << read_method_name_ <<  "(ctx, iprot); err2 != nil {" << endl;
  indent_up();
  f_types_ << indent() << "iprot.ReadMessageEnd(ctx)" << endl;
  if (!tfunction->is_oneway()) {
    f_types_ << indent()
               << "x := thrift.NewTApplicationException(thrift.PROTOCOL_ERROR, err2.Error())"
               << endl;
    f_types_ << indent() << "oprot.WriteMessageBegin(ctx, \"" << escape_string(tfunction->get_name())
               << "\", thrift.EXCEPTION, seqId)" << endl;
    f_types_ << indent() << "x.Write(ctx, oprot)" << endl;
    f_types_ << indent() << "oprot.WriteMessageEnd(ctx)" << endl;
    f_types_ << indent() << "oprot.Flush(ctx)" << endl;
  }
  f_types_ << indent() << "return false, thrift.WrapTException(err2)" << endl;
  indent_down();
  f_types_ << indent() << "}" << endl;
  f_types_ << indent() << "iprot.ReadMessageEnd(ctx)" << endl << endl;

  // Even though we never create the goroutine in oneway handlers,
  // always have (nop) tickerCancel defined makes the writing part of code
  // generating easier and less error-prone.
  f_types_ << indent() << "tickerCancel := func() {}" << endl;
  // Only create the goroutine for non-oneways.
  if (!tfunction->is_oneway()) {
    f_types_ << indent() << "// Start a goroutine to do server side connectivity check." << endl;
    f_types_ << indent() << "if thrift.ServerConnectivityCheckInterval > 0 {" << endl;

    indent_up();
    f_types_ << indent() << "var cancel context.CancelFunc" << endl;
    f_types_ << indent() << "ctx, cancel = context.WithCancel(ctx)" << endl;
    f_types_ << indent() << "defer cancel()" << endl;
    f_types_ << indent() << "var tickerCtx context.Context" << endl;
    f_types_ << indent() << "tickerCtx, tickerCancel = context.WithCancel(context.Background())" << endl;
    f_types_ << indent() << "defer tickerCancel()" << endl;
    f_types_ << indent() << "go func(ctx context.Context, cancel context.CancelFunc) {" << endl;

    indent_up();
    f_types_ << indent() << "ticker := time.NewTicker(thrift.ServerConnectivityCheckInterval)" << endl;
    f_types_ << indent() << "defer ticker.Stop()" << endl;
    f_types_ << indent() << "for {" << endl;

    indent_up();
    f_types_ << indent() << "select {" << endl;
    f_types_ << indent() << "case <-ctx.Done():" << endl;
    indent_up();
    f_types_ << indent() << "return" << endl;
    indent_down();
    f_types_ << indent() << "case <-ticker.C:" << endl;

    indent_up();
    f_types_ << indent() << "if !iprot.Transport().IsOpen() {" << endl;
    indent_up();
    f_types_ << indent() << "cancel()" << endl;
    f_types_ << indent() << "return" << endl;
    indent_down();
    f_types_ << indent() << "}" << endl;
    indent_down();
    f_types_ << indent() << "}" << endl;
    indent_down();
    f_types_ << indent() << "}" << endl;
    indent_down();
    f_types_ << indent() << "}(tickerCtx, cancel)" << endl;
    indent_down();
    f_types_ << indent() << "}" << endl << endl;
  } else {
    // Make sure we don't get the defined but unused compiling error.
    f_types_ << indent() << "_ = tickerCancel" << endl << endl;
  }

  if (!tfunction->is_oneway()) {
    f_types_ << indent() << "result := " << resultname << "{}" << endl;
  }
  bool need_reference = type_need_reference(tfunction->get_returntype());

  f_types_ << indent() << "if ";

  if (!tfunction->is_oneway()) {
    if (!tfunction->get_returntype()->is_void()) {
      f_types_ << "retval, ";
    }
  }

  // Generate the function call
  t_struct* arg_struct = tfunction->get_arglist();
  const std::vector<t_field*>& fields = arg_struct->get_members();
  vector<t_field*>::const_iterator f_iter;
  f_types_ << "err2 := p.handler." << publicize(tfunction->get_name()) << "(";
  bool first = true;

  f_types_ << "ctx";
  for (f_iter = fields.begin(); f_iter != fields.end(); ++f_iter) {
    if (first) {
      first = false;
      f_types_ << ", ";
    } else {
      f_types_ << ", ";
    }

    f_types_ << "args." << publicize((*f_iter)->get_name());
  }

  f_types_ << "); err2 != nil {" << endl;
  indent_up();
  f_types_ << indent() << "tickerCancel()" << endl;
  f_types_ << indent() << "err = thrift.WrapTException(err2)" << endl;

  t_struct* exceptions = tfunction->get_xceptions();
  const vector<t_field*>& x_fields = exceptions->get_members();
  if (!x_fields.empty()) {
    f_types_ << indent() << "switch v := err2.(type) {" << endl;

    vector<t_field*>::const_iterator xf_iter;

    for (xf_iter = x_fields.begin(); xf_iter != x_fields.end(); ++xf_iter) {
      f_types_ << indent() << "case " << type_to_go_type(((*xf_iter)->get_type())) << ":"
                 << endl;
      indent_up();
      f_types_ << indent() << "result." << publicize((*xf_iter)->get_name()) << " = v" << endl;
      indent_down();
    }

    f_types_ << indent() << "default:" << endl;
    indent_up();
  }

  if (!tfunction->is_oneway()) {
    // Avoid writing the error to the wire if it's ErrAbandonRequest
    f_types_ << indent() << "if errors.Is(err2, thrift.ErrAbandonRequest) {" << endl;
    indent_up();
    f_types_ << indent() << "return false, thrift.WrapTException(err2)" << endl;
    indent_down();
    f_types_ << indent() << "}" << endl;

    string exc(tmp("_exc"));
    f_types_ << indent() << exc << " := thrift.NewTApplicationException(thrift.INTERNAL_ERROR, "
                              "\"Internal error processing " << escape_string(tfunction->get_name())
               << ": \" + err2.Error())" << endl;

    f_types_ << indent() << "if err2 := oprot.WriteMessageBegin(ctx, \"" << escape_string(tfunction->get_name())
               << "\", thrift.EXCEPTION, seqId); err2 != nil {" << endl;
    indent_up();
    f_types_ << indent() << write_err << " = thrift.WrapTException(err2)" << endl;
    indent_down();
    f_types_ << indent() << "}" << endl;

    f_types_ << indent() << "if err2 := " << exc << ".Write(ctx, oprot); "
               << write_err << " == nil && err2 != nil {" << endl;
    indent_up();
    f_types_ << indent() << write_err << " = thrift.WrapTException(err2)" << endl;
    indent_down();
    f_types_ << indent() << "}" << endl;

    f_types_ << indent() << "if err2 := oprot.WriteMessageEnd(ctx); "
               << write_err << " == nil && err2 != nil {" << endl;
    indent_up();
    f_types_ << indent() << write_err << " = thrift.WrapTException(err2)" << endl;
    indent_down();
    f_types_ << indent() << "}" << endl;

    f_types_ << indent() << "if err2 := oprot.Flush(ctx); "
               << write_err << " == nil && err2 != nil {" << endl;
    indent_up();
    f_types_ << indent() << write_err << " = thrift.WrapTException(err2)" << endl;
    indent_down();
    f_types_ << indent() << "}" << endl;

    f_types_ << indent() << "if " << write_err << " != nil {" << endl;
    indent_up();
    f_types_ << indent() << "return false, thrift.WrapTException(" << write_err << ")" << endl;
    indent_down();
    f_types_ << indent() << "}" << endl;

    // return success=true as long as writing to the wire was successful.
    f_types_ << indent() << "return true, err" << endl;
  }

  if (!x_fields.empty()) {
    indent_down();
    f_types_ << indent() << "}" << endl; // closes switch
  }

  indent_down();
  f_types_ << indent() << "}"; // closes err2 != nil

  if (!tfunction->is_oneway()) {
    if (!tfunction->get_returntype()->is_void()) {
      f_types_ << " else {" << endl; // make sure we set Success retval only on success
      indent_up();
      f_types_ << indent() << "result.Success = ";
      if (need_reference) {
        f_types_ << "&";
      }
      f_types_ << "retval" << endl;
      indent_down();
      f_types_ << indent() << "}" << endl;
    } else {
      f_types_ << endl;
    }
    f_types_ << indent() << "tickerCancel()" << endl;

    f_types_ << indent() << "if err2 := oprot.WriteMessageBegin(ctx, \""
               << escape_string(tfunction->get_name()) << "\", thrift.REPLY, seqId); err2 != nil {"
               << endl;
    indent_up();
    f_types_ << indent() << write_err << " = thrift.WrapTException(err2)" << endl;
    indent_down();
    f_types_ << indent() << "}" << endl;

    f_types_ << indent() << "if err2 := result." << write_method_name_ << "(ctx, oprot); "
               << write_err << " == nil && err2 != nil {" << endl;
    indent_up();
    f_types_ << indent() << write_err << " = thrift.WrapTException(err2)" << endl;
    indent_down();
    f_types_ << indent() << "}" << endl;

    f_types_ << indent() << "if err2 := oprot.WriteMessageEnd(ctx); "
               << write_err << " == nil && err2 != nil {" << endl;
    indent_up();
    f_types_ << indent() << write_err << " = thrift.WrapTException(err2)" << endl;
    indent_down();
    f_types_ << indent() << "}" << endl;

    f_types_ << indent() << "if err2 := oprot.Flush(ctx); " << write_err << " == nil && err2 != nil {" << endl;
    indent_up();
    f_types_ << indent() << write_err << " = thrift.WrapTException(err2)" << endl;
    indent_down();
    f_types_ << indent() << "}" << endl;

    f_types_ << indent() << "if " << write_err << " != nil {" << endl;
    indent_up();
    f_types_ << indent() << "return false, thrift.WrapTException(" << write_err << ")" << endl;
    indent_down();
    f_types_ << indent() << "}" << endl;

    // return success=true as long as writing to the wire was successful.
    f_types_ << indent() << "return true, err" << endl;
  } else {
    f_types_ << endl;
    f_types_ << indent() << "tickerCancel()" << endl;
    f_types_ << indent() << "return true, err" << endl;
  }
  indent_down();
  f_types_ << indent() << "}" << endl << endl;
}

/**
 * Deserializes a field of any type.
 */
void t_go_generator::generate_deserialize_field(ostream& out,
                                                t_field* tfield,
                                                bool declare,
                                                string prefix,
                                                bool inclass,
                                                bool coerceData,
                                                bool inkey,
                                                bool in_container_value) {
  (void)inclass;
  (void)coerceData;
  t_type* orig_type = tfield->get_type();
  t_type* type = get_true_type(orig_type);
  string name(prefix + publicize(tfield->get_name()));

  if (type->is_void()) {
    throw "CANNOT GENERATE DESERIALIZE CODE FOR void TYPE: " + name;
  }

  if (type->is_struct() || type->is_xception()) {
    generate_deserialize_struct(out,
                                (t_struct*)type,
                                is_pointer_field(tfield, in_container_value),
                                declare,
                                name);
  } else if (type->is_container()) {
    generate_deserialize_container(out, orig_type, is_pointer_field(tfield), declare, name);
  } else if (type->is_base_type() || type->is_enum()) {

    if (declare) {
      string type_name = inkey ? type_to_go_key_type(tfield->get_type())
                               : type_to_go_type(tfield->get_type());

      out << "var " << tfield->get_name() << " " << type_name << endl;
    }

    indent(out) << "if v, err := iprot.";

    if (type->is_base_type()) {
      t_base_type::t_base tbase = ((t_base_type*)type)->get_base();

      switch (tbase) {
      case t_base_type::TYPE_VOID:
        throw "compiler error: cannot serialize void field in a struct: " + name;
        break;

      case t_base_type::TYPE_STRING:
        if (type->is_binary() && !inkey) {
          out << "ReadBinary(ctx)";
        } else {
          out << "ReadString(ctx)";
        }

        break;

      case t_base_type::TYPE_BOOL:
        out << "ReadBool(ctx)";
        break;

      case t_base_type::TYPE_I8:
        out << "ReadByte(ctx)";
        break;

      case t_base_type::TYPE_I16:
        out << "ReadI16(ctx)";
        break;

      case t_base_type::TYPE_I32:
        out << "ReadI32(ctx)";
        break;

      case t_base_type::TYPE_I64:
        out << "ReadI64(ctx)";
        break;

      case t_base_type::TYPE_DOUBLE:
        out << "ReadDouble(ctx)";
        break;

      default:
        throw "compiler error: no Go name for base type " + t_base_type::t_base_name(tbase);
      }
    } else if (type->is_enum()) {
      out << "ReadI32(ctx)";
    }

    out << "; err != nil {" << endl;
    out << indent() << "return thrift.PrependError(\"error reading field " << tfield->get_key()
        << ": \", err)" << endl;

    out << "} else {" << endl;
    string wrap;

    if (type->is_enum() || orig_type->is_typedef()) {
      wrap = publicize(type_name(orig_type));
    } else if (((t_base_type*)type)->get_base() == t_base_type::TYPE_I8) {
      wrap = "int8";
    }

    string maybe_address = (is_pointer_field(tfield) ? "&" : "");
    if (wrap == "") {
      indent(out) << name << " = " << maybe_address << "v" << endl;
    } else {
      indent(out) << "temp := " << wrap << "(v)" << endl;
      indent(out) << name << " = " << maybe_address << "temp" << endl;
    }

    out << "}" << endl;
  } else {
    throw "INVALID TYPE IN generate_deserialize_field '" + type->get_name() + "' for field '"
        + tfield->get_name() + "'";
  }
}

/**
 * Generates an unserializer for a struct, calling read()
 */
void t_go_generator::generate_deserialize_struct(ostream& out,
                                                 t_struct* tstruct,
                                                 bool pointer_field,
                                                 bool declare,
                                                 string prefix) {
  string eq(declare ? " := " : " = ");

  out << indent() << prefix << eq << (pointer_field ? "&" : "");
  generate_go_struct_initializer(out, tstruct);
  out << indent() << "if err := " << prefix << "." << read_method_name_ <<  "(ctx, iprot); err != nil {" << endl;
  out << indent() << "  return thrift.PrependError(fmt.Sprintf(\"%T error reading struct: \", "
      << prefix << "), err)" << endl;
  out << indent() << "}" << endl;
}

/**
 * Serialize a container by writing out the header followed by
 * data and then a footer.
 */
void t_go_generator::generate_deserialize_container(ostream& out,
                                                    t_type* orig_type,
                                                    bool pointer_field,
                                                    bool declare,
                                                    string prefix) {
  t_type* ttype = get_true_type(orig_type);
  string eq(" = ");

  if (declare) {
    eq = " := ";
  }

  // Declare variables, read header
  if (ttype->is_map()) {
    out << indent() << "_, _, size, err := iprot.ReadMapBegin(ctx)" << endl;
    out << indent() << "if size < 0 {" << endl;
    out << indent() << "  return errors.New(\"map size is negative\")" << endl;
    out << indent() << "}" << endl;
    out << indent() << "if err != nil {" << endl;
    out << indent() << "  return thrift.PrependError(\"error reading map begin: \", err)" << endl;
    out << indent() << "}" << endl;
    out << indent() << "tMap := make(" << type_to_go_type(orig_type) << ", size)" << endl;
    out << indent() << prefix << eq << " " << (pointer_field ? "&" : "") << "tMap" << endl;
  } else if (ttype->is_set()) {
    out << indent() << "_, size, err := iprot.ReadSetBegin(ctx)" << endl;
    out << indent() << "if size < 0 {" << endl;
    out << indent() << "  return errors.New(\"set size is negative\")" << endl;
    out << indent() << "}" << endl;
    out << indent() << "if err != nil {" << endl;
    out << indent() << "  return thrift.PrependError(\"error reading set begin: \", err)" << endl;
    out << indent() << "}" << endl;
    out << indent() << "tSet := make(" << type_to_go_type(orig_type) << ", 0, size)" << endl;
    out << indent() << prefix << eq << " " << (pointer_field ? "&" : "") << "tSet" << endl;
  } else if (ttype->is_list()) {
    out << indent() << "_, size, err := iprot.ReadListBegin(ctx)" << endl;
    out << indent() << "if size < 0 {" << endl;
    out << indent() << "  return errors.New(\"list size is negative\")" << endl;
    out << indent() << "}" << endl;
    out << indent() << "if err != nil {" << endl;
    out << indent() << "  return thrift.PrependError(\"error reading list begin: \", err)" << endl;
    out << indent() << "}" << endl;
    out << indent() << "tSlice := make(" << type_to_go_type(orig_type) << ", 0, size)" << endl;
    out << indent() << prefix << eq << " " << (pointer_field ? "&" : "") << "tSlice" << endl;
  } else {
    throw "INVALID TYPE IN generate_deserialize_container '" + ttype->get_name() + "' for prefix '"
        + prefix + "'";
  }

  // For loop iterates over elements
  out << indent() << "for i := 0; i < size; i ++ {" << endl;
  indent_up();

  if (pointer_field) {
    prefix = "(*" + prefix + ")";
  }
  if (ttype->is_map()) {
    generate_deserialize_map_element(out, (t_map*)ttype, declare, prefix);
  } else if (ttype->is_set()) {
    generate_deserialize_set_element(out, (t_set*)ttype, declare, prefix);
  } else if (ttype->is_list()) {
    generate_deserialize_list_element(out, (t_list*)ttype, declare, prefix);
  }

  indent_down();
  out << indent() << "}" << endl;

  // Read container end
  if (ttype->is_map()) {
    out << indent() << "if err := iprot.ReadMapEnd(ctx); err != nil {" << endl;
    out << indent() << "  return thrift.PrependError(\"error reading map end: \", err)" << endl;
    out << indent() << "}" << endl;
  } else if (ttype->is_set()) {
    out << indent() << "if err := iprot.ReadSetEnd(ctx); err != nil {" << endl;
    out << indent() << "  return thrift.PrependError(\"error reading set end: \", err)" << endl;
    out << indent() << "}" << endl;
  } else if (ttype->is_list()) {
    out << indent() << "if err := iprot.ReadListEnd(ctx); err != nil {" << endl;
    out << indent() << "  return thrift.PrependError(\"error reading list end: \", err)" << endl;
    out << indent() << "}" << endl;
  }
}

/**
 * Generates code to deserialize a map
 */
void t_go_generator::generate_deserialize_map_element(ostream& out,
                                                      t_map* tmap,
                                                      bool declare,
                                                      string prefix) {
  (void)declare;
  string key = tmp("_key");
  string val = tmp("_val");
  t_field fkey(tmap->get_key_type(), key);
  t_field fval(tmap->get_val_type(), val);
  fkey.set_req(t_field::T_OPT_IN_REQ_OUT);
  fval.set_req(t_field::T_OPT_IN_REQ_OUT);
  generate_deserialize_field(out, &fkey, true, "", false, false, true);
  generate_deserialize_field(out, &fval, true, "", false, false, false, true);
  indent(out) << prefix << "[" << key << "] = " << val << endl;
}

/**
 * Write a set element
 */
void t_go_generator::generate_deserialize_set_element(ostream& out,
                                                      t_set* tset,
                                                      bool declare,
                                                      string prefix) {
  (void)declare;
  string elem = tmp("_elem");
  t_field felem(tset->get_elem_type(), elem);
  felem.set_req(t_field::T_OPT_IN_REQ_OUT);
  generate_deserialize_field(out, &felem, true, "", false, false, false, true);
  indent(out) << prefix << " = append(" << prefix << ", " << elem << ")" << endl;
}

/**
 * Write a list element
 */
void t_go_generator::generate_deserialize_list_element(ostream& out,
                                                       t_list* tlist,
                                                       bool declare,
                                                       string prefix) {
  (void)declare;
  string elem = tmp("_elem");
  t_field felem(((t_list*)tlist)->get_elem_type(), elem);
  felem.set_req(t_field::T_OPT_IN_REQ_OUT);
  generate_deserialize_field(out, &felem, true, "", false, false, false, true);
  indent(out) << prefix << " = append(" << prefix << ", " << elem << ")" << endl;
}

/**
 * Serializes a field of any type.
 *
 * @param tfield The field to serialize
 * @param prefix Name to prepend to field name
 */
void t_go_generator::generate_serialize_field(ostream& out,
                                              t_field* tfield,
                                              string prefix,
                                              bool inkey) {
  t_type* type = get_true_type(tfield->get_type());
  string name(prefix + publicize(tfield->get_name()));

  // Do nothing for void types
  if (type->is_void()) {
    throw "compiler error: cannot generate serialize for void type: " + name;
  }

  if (type->is_struct() || type->is_xception()) {
    generate_serialize_struct(out, (t_struct*)type, name);
  } else if (type->is_container()) {
    generate_serialize_container(out, type, is_pointer_field(tfield), name);
  } else if (type->is_base_type() || type->is_enum()) {
    indent(out) << "if err := oprot.";

    if (is_pointer_field(tfield)) {
      name = "*" + name;
    }

    if (type->is_base_type()) {
      t_base_type::t_base tbase = ((t_base_type*)type)->get_base();

      switch (tbase) {
      case t_base_type::TYPE_VOID:
        throw "compiler error: cannot serialize void field in a struct: " + name;
        break;

      case t_base_type::TYPE_STRING:
        if (type->is_binary() && !inkey) {
          out << "WriteBinary(ctx, " << name << ")";
        } else {
          out << "WriteString(ctx, string(" << name << "))";
        }

        break;

      case t_base_type::TYPE_BOOL:
        out << "WriteBool(ctx, bool(" << name << "))";
        break;

      case t_base_type::TYPE_I8:
        out << "WriteByte(ctx, int8(" << name << "))";
        break;

      case t_base_type::TYPE_I16:
        out << "WriteI16(ctx, int16(" << name << "))";
        break;

      case t_base_type::TYPE_I32:
        out << "WriteI32(ctx, int32(" << name << "))";
        break;

      case t_base_type::TYPE_I64:
        out << "WriteI64(ctx, int64(" << name << "))";
        break;

      case t_base_type::TYPE_DOUBLE:
        out << "WriteDouble(ctx, float64(" << name << "))";
        break;

      default:
        throw "compiler error: no Go name for base type " + t_base_type::t_base_name(tbase);
      }
    } else if (type->is_enum()) {
      out << "WriteI32(ctx, int32(" << name << "))";
    }

    out << "; err != nil {" << endl;
    out << indent() << "return thrift.PrependError(fmt.Sprintf(\"%T."
        << escape_string(tfield->get_name()) << " (" << tfield->get_key()
        << ") field write error: \", p), err) }" << endl;
  } else {
    throw "compiler error: Invalid type in generate_serialize_field '" + type->get_name()
        + "' for field '" + name + "'";
  }
}

/**
 * Serializes all the members of a struct.
 *
 * @param tstruct The struct to serialize
 * @param prefix  String prefix to attach to all fields
 */
void t_go_generator::generate_serialize_struct(ostream& out, t_struct* tstruct, string prefix) {
  (void)tstruct;
  out << indent() << "if err := " << prefix << "." << write_method_name_ << "(ctx, oprot); err != nil {" << endl;
  out << indent() << "  return thrift.PrependError(fmt.Sprintf(\"%T error writing struct: \", "
      << prefix << "), err)" << endl;
  out << indent() << "}" << endl;
}

void t_go_generator::generate_serialize_container(ostream& out,
                                                  t_type* ttype,
                                                  bool pointer_field,
                                                  string prefix) {
  if (pointer_field) {
    prefix = "*" + prefix;
  }
  if (ttype->is_map()) {
    out << indent() << "if err := oprot.WriteMapBegin(ctx, "
        << type_to_enum(((t_map*)ttype)->get_key_type()) << ", "
        << type_to_enum(((t_map*)ttype)->get_val_type()) << ", "
        << "len(" << prefix << ")); err != nil {" << endl;
    out << indent() << "  return thrift.PrependError(\"error writing map begin: \", err)" << endl;
    out << indent() << "}" << endl;
  } else if (ttype->is_set()) {
    out << indent() << "if err := oprot.WriteSetBegin(ctx, "
        << type_to_enum(((t_set*)ttype)->get_elem_type()) << ", "
        << "len(" << prefix << ")); err != nil {" << endl;
    out << indent() << "  return thrift.PrependError(\"error writing set begin: \", err)" << endl;
    out << indent() << "}" << endl;
  } else if (ttype->is_list()) {
    out << indent() << "if err := oprot.WriteListBegin(ctx, "
        << type_to_enum(((t_list*)ttype)->get_elem_type()) << ", "
        << "len(" << prefix << ")); err != nil {" << endl;
    out << indent() << "  return thrift.PrependError(\"error writing list begin: \", err)" << endl;
    out << indent() << "}" << endl;
  } else {
    throw "compiler error: Invalid type in generate_serialize_container '" + ttype->get_name()
        + "' for prefix '" + prefix + "'";
  }

  if (ttype->is_map()) {
    t_map* tmap = (t_map*)ttype;
    out << indent() << "for k, v := range " << prefix << " {" << endl;
    indent_up();
    generate_serialize_map_element(out, tmap, "k", "v");
    indent_down();
    indent(out) << "}" << endl;
  } else if (ttype->is_set()) {
    t_set* tset = (t_set*)ttype;
    out << indent() << "for i := 0; i<len(" << prefix << "); i++ {" << endl;
    indent_up();
    out << indent() << "for j := i+1; j<len(" << prefix << "); j++ {" << endl;
    indent_up();
    string wrapped_prefix = prefix;
    if (pointer_field) {
      wrapped_prefix = "(" + prefix + ")";
    }
    string goType = type_to_go_type(tset->get_elem_type());
    out << indent() << "if func(tgt, src " << goType << ") bool {" << endl;
    indent_up();
    generate_go_equals(out, tset->get_elem_type(), "tgt", "src");
    out << indent() << "return true" << endl;
    indent_down();
    out << indent() << "}(" << wrapped_prefix << "[i], " << wrapped_prefix << "[j]) {" << endl;
    indent_up();
    out << indent()
        << "return thrift.PrependError(\"\", fmt.Errorf(\"%T error writing set field: slice is not "
           "unique\", "
        << wrapped_prefix << "))" << endl;
    indent_down();
    out << indent() << "}" << endl;
    indent_down();
    out << indent() << "}" << endl;
    indent_down();
    out << indent() << "}" << endl;
    out << indent() << "for _, v := range " << prefix << " {" << endl;
    indent_up();
    generate_serialize_set_element(out, tset, "v");
    indent_down();
    indent(out) << "}" << endl;
  } else if (ttype->is_list()) {
    t_list* tlist = (t_list*)ttype;
    out << indent() << "for _, v := range " << prefix << " {" << endl;

    indent_up();
    generate_serialize_list_element(out, tlist, "v");
    indent_down();
    indent(out) << "}" << endl;
  }

  if (ttype->is_map()) {
    out << indent() << "if err := oprot.WriteMapEnd(ctx); err != nil {" << endl;
    out << indent() << "  return thrift.PrependError(\"error writing map end: \", err)" << endl;
    out << indent() << "}" << endl;
  } else if (ttype->is_set()) {
    out << indent() << "if err := oprot.WriteSetEnd(ctx); err != nil {" << endl;
    out << indent() << "  return thrift.PrependError(\"error writing set end: \", err)" << endl;
    out << indent() << "}" << endl;
  } else if (ttype->is_list()) {
    out << indent() << "if err := oprot.WriteListEnd(ctx); err != nil {" << endl;
    out << indent() << "  return thrift.PrependError(\"error writing list end: \", err)" << endl;
    out << indent() << "}" << endl;
  }
}

/**
 * Serializes the members of a map.
 *
 */
void t_go_generator::generate_serialize_map_element(ostream& out,
                                                    t_map* tmap,
                                                    string kiter,
                                                    string viter) {
  t_field kfield(tmap->get_key_type(), "");
  t_field vfield(tmap->get_val_type(), "");
  kfield.set_req(t_field::T_OPT_IN_REQ_OUT);
  vfield.set_req(t_field::T_OPT_IN_REQ_OUT);
  generate_serialize_field(out, &kfield, kiter, true);
  generate_serialize_field(out, &vfield, viter);
}

/**
 * Serializes the members of a set.
 */
void t_go_generator::generate_serialize_set_element(ostream& out, t_set* tset, string prefix) {
  t_field efield(tset->get_elem_type(), "");
  efield.set_req(t_field::T_OPT_IN_REQ_OUT);
  generate_serialize_field(out, &efield, prefix);
}

/**
 * Serializes the members of a list.
 */
void t_go_generator::generate_serialize_list_element(ostream& out, t_list* tlist, string prefix) {
  t_field efield(tlist->get_elem_type(), "");
  efield.set_req(t_field::T_OPT_IN_REQ_OUT);
  generate_serialize_field(out, &efield, prefix);
}

/**
 * Compares any type
 */
void t_go_generator::generate_go_equals(ostream& out, t_type* ori_type, string tgt, string src) {

  t_type* ttype = get_true_type(ori_type);
  // Do nothing for void types
  if (ttype->is_void()) {
    throw "compiler error: cannot generate equals for void type: " + tgt;
  }

  if (ttype->is_struct() || ttype->is_xception()) {
    generate_go_equals_struct(out, ttype, tgt, src);
  } else if (ttype->is_container()) {
    generate_go_equals_container(out, ttype, tgt, src);
  } else if (ttype->is_base_type() || ttype->is_enum()) {
    out << indent() << "if ";
    if (ttype->is_base_type()) {
      t_base_type::t_base tbase = ((t_base_type*)ttype)->get_base();
      switch (tbase) {
      case t_base_type::TYPE_VOID:
        throw "compiler error: cannot equals void: " + tgt;
        break;

      case t_base_type::TYPE_STRING:
        if (ttype->is_binary()) {
          out << "bytes.Compare(" << tgt << ", " << src << ") != 0";
        } else {
          out << tgt << " != " << src;
        }
        break;

      case t_base_type::TYPE_BOOL:
      case t_base_type::TYPE_I8:
      case t_base_type::TYPE_I16:
      case t_base_type::TYPE_I32:
      case t_base_type::TYPE_I64:
      case t_base_type::TYPE_DOUBLE:
        out << tgt << " != " << src;
        break;

      default:
        throw "compiler error: no Go name for base type " + t_base_type::t_base_name(tbase);
      }
    } else if (ttype->is_enum()) {
      out << tgt << " != " << src;
    }

    out << " { return false }" << endl;
  } else {
    throw "compiler error: Invalid type in generate_go_equals '" + ttype->get_name() + "' for '"
        + tgt + "'";
  }
}

/**
 * Compares the members of a struct
 */
void t_go_generator::generate_go_equals_struct(ostream& out,
                                               t_type* ttype,
                                               string tgt,
                                               string src) {
  (void)ttype;
  out << indent() << "if !" << tgt << "." << equals_method_name_ << "(" << src
      << ") { return false }" << endl;
}

/**
 * Compares any container type
 */
void t_go_generator::generate_go_equals_container(ostream& out,
                                                  t_type* ttype,
                                                  string tgt,
                                                  string src) {
  out << indent() << "if len(" << tgt << ") != len(" << src << ") { return false }" << endl;
  if (ttype->is_map()) {
    t_map* tmap = (t_map*)ttype;
    out << indent() << "for k, _tgt := range " << tgt << " {" << endl;
    indent_up();
    string element_source = tmp("_src");
    out << indent() << element_source << " := " << src << "[k]" << endl;
    generate_go_equals(out, tmap->get_val_type(), "_tgt", element_source);
    indent_down();
    indent(out) << "}" << endl;
  } else if (ttype->is_list() || ttype->is_set()) {
    t_type* elem;
    if (ttype->is_list()) {
      t_list* temp = (t_list*)ttype;
      elem = temp->get_elem_type();
    } else {
      t_set* temp = (t_set*)ttype;
      elem = temp->get_elem_type();
    }
    out << indent() << "for i, _tgt := range " << tgt << " {" << endl;
    indent_up();
    string element_source = tmp("_src");
    out << indent() << element_source << " := " << src << "[i]" << endl;
    generate_go_equals(out, elem, "_tgt", element_source);
    indent_down();
    indent(out) << "}" << endl;
  } else {
    throw "INVALID TYPE IN generate_go_equals_container '" + ttype->get_name();
  }
}

/**
 * Generates the docstring for a given struct.
 */
void t_go_generator::generate_go_docstring(ostream& out, t_struct* tstruct) {
  generate_go_docstring(out, tstruct, tstruct, "Attributes");
}

/**
 * Generates the docstring for a given function.
 */
void t_go_generator::generate_go_docstring(ostream& out, t_function* tfunction) {
  generate_go_docstring(out, tfunction, tfunction->get_arglist(), "Parameters");
}

/**
 * Generates the docstring for a struct or function.
 */
void t_go_generator::generate_go_docstring(ostream& out,
                                           t_doc* tdoc,
                                           t_struct* tstruct,
                                           const char* subheader) {
  bool has_doc = false;
  stringstream ss;

  if (tdoc->has_doc()) {
    has_doc = true;
    ss << tdoc->get_doc();
  }

  const vector<t_field*>& fields = tstruct->get_members();

  if (fields.size() > 0) {
    if (has_doc) {
      ss << endl;
    }

    has_doc = true;
    ss << subheader << ":\n";
    vector<t_field*>::const_iterator p_iter;

    for (p_iter = fields.begin(); p_iter != fields.end(); ++p_iter) {
      t_field* p = *p_iter;
      ss << " - " << publicize(p->get_name());

      if (p->has_doc()) {
        ss << ": " << p->get_doc();
      } else {
        ss << endl;
      }
    }
  }

  if (has_doc) {
    generate_docstring_comment(out, "", "// ", ss.str(), "");
  }
}

/**
 * Generates the docstring for a generic object.
 */
void t_go_generator::generate_go_docstring(ostream& out, t_doc* tdoc) {
  if (tdoc->has_doc()) {
    generate_docstring_comment(out, "", "//", tdoc->get_doc(), "");
  }
}

/**
 * Declares an argument, which may include initialization as necessary.
 *
 * @param tfield The field
 */
string t_go_generator::declare_argument(t_field* tfield) {
  std::ostringstream result;
  result << publicize(tfield->get_name()) << "=";

  if (tfield->get_value() != nullptr) {
    result << "thrift_spec[" << tfield->get_key() << "][4]";
  } else {
    result << "nil";
  }

  return result.str();
}

/**
 * Renders a struct field initial value.
 *
 * @param tfield The field, which must have `tfield->get_value() != nullptr`
 */
string t_go_generator::render_field_initial_value(t_field* tfield,
                                                  const string& name,
                                                  bool optional_field) {
  t_type* type = get_true_type(tfield->get_type());

  if (optional_field) {
    // The caller will make a second pass for optional fields,
    // assigning the result of render_const_value to "*field_name". It
    // is maddening that Go syntax does not allow for a type-agnostic
    // way to initialize a pointer to a const value, but so it goes.
    // The alternative would be to write type specific functions that
    // convert from const values to pointer types, but given the lack
    // of overloading it would be messy.
    return "new(" + type_to_go_type(tfield->get_type()) + ")";
  } else {
    return render_const_value(type, tfield->get_value(), name);
  }
}

/**
 * Renders a function signature of the form 'type name(args)'
 *
 * @param tfunction Function definition
 * @return String of rendered function definition
 */
string t_go_generator::function_signature(t_function* tfunction, string prefix) {
  // TODO(mcslee): Nitpicky, no ',' if argument_list is empty
  return publicize(prefix + tfunction->get_name()) + "(" + argument_list(tfunction->get_arglist())
         + ")";
}

/**
 * Renders an interface function signature of the form 'type name(args)'
 *
 * @param tfunction Function definition
 * @return String of rendered function definition
 */
string t_go_generator::function_signature_if(t_function* tfunction, string prefix, bool addError) {
  string signature = publicize(prefix + tfunction->get_name()) + "(";
  signature += "ctx context.Context";
  if (!tfunction->get_arglist()->get_members().empty()) {
    signature += ", " + argument_list(tfunction->get_arglist());
  }
  signature += ") (";

  t_type* ret = tfunction->get_returntype();
  t_struct* exceptions = tfunction->get_xceptions();
  string errs = argument_list(exceptions);

  if (!ret->is_void()) {
    signature += "_r " + type_to_go_type(ret);

    if (addError || errs.size() == 0) {
      signature += ", ";
    }
  }

  if (addError) {
    signature += "_err error";
  }

  signature += ")";
  return signature;
}

/**
 * Renders a field list
 */
string t_go_generator::argument_list(t_struct* tstruct) {
  string result = "";
  const vector<t_field*>& fields = tstruct->get_members();
  vector<t_field*>::const_iterator f_iter;
  bool first = true;

  for (f_iter = fields.begin(); f_iter != fields.end(); ++f_iter) {
    if (first) {
      first = false;
    } else {
      result += ", ";
    }

    result += variable_name_to_go_name((*f_iter)->get_name()) + " "
              + type_to_go_type((*f_iter)->get_type());
  }

  return result;
}

string t_go_generator::type_name(t_type* ttype) {
  string module( module_name(ttype));
  if( ! module.empty()) {
    return module + "." + ttype->get_name();
  }

  return ttype->get_name();
}

string t_go_generator::module_name(t_type* ttype) {
  t_program* program = ttype->get_program();

  if (program != nullptr && program != program_) {
    if (program->get_namespace("go").empty() ||
        program_->get_namespace("go").empty() ||
        program->get_namespace("go") != program_->get_namespace("go")) {
      string module(get_real_go_module(program));
      return package_identifiers_[module];
    }
  }

  return "";
}

/**
 * Converts the parse type to a go tyoe
 */
string t_go_generator::type_to_enum(t_type* type) {
  type = get_true_type(type);

  if (type->is_base_type()) {
    t_base_type::t_base tbase = ((t_base_type*)type)->get_base();

    switch (tbase) {
    case t_base_type::TYPE_VOID:
      throw "NO T_VOID CONSTRUCT";

    case t_base_type::TYPE_STRING:
      /* this is wrong, binary is still a string type internally
      if (type->is_binary()) {
          return "thrift.BINARY";
      }
      */
      return "thrift.STRING";

    case t_base_type::TYPE_BOOL:
      return "thrift.BOOL";

    case t_base_type::TYPE_I8:
      return "thrift.BYTE";

    case t_base_type::TYPE_I16:
      return "thrift.I16";

    case t_base_type::TYPE_I32:
      return "thrift.I32";

    case t_base_type::TYPE_I64:
      return "thrift.I64";

    case t_base_type::TYPE_DOUBLE:
      return "thrift.DOUBLE";
    }
  } else if (type->is_enum()) {
    return "thrift.I32";
  } else if (type->is_struct() || type->is_xception()) {
    return "thrift.STRUCT";
  } else if (type->is_map()) {
    return "thrift.MAP";
  } else if (type->is_set()) {
    return "thrift.SET";
  } else if (type->is_list()) {
    return "thrift.LIST";
  }

  throw "INVALID TYPE IN type_to_enum: " + type->get_name();
}

/**
 * Converts the parse type to a go map type, will throw an exception if it will
 * not produce a valid go map type.
 */
string t_go_generator::type_to_go_key_type(t_type* type) {
  t_type* resolved_type = type;

  while (resolved_type->is_typedef()) {
    resolved_type = ((t_typedef*)resolved_type)->get_type()->get_true_type();
  }

  if (resolved_type->is_map() || resolved_type->is_list() || resolved_type->is_set()) {
    throw "Cannot produce a valid type for a Go map key: " + type_to_go_type(type) + " - aborting.";
  }

  if (resolved_type->is_binary())
    return "string";

  return type_to_go_type(type);
}

/**
 * Converts the parse type to a go type
 */
string t_go_generator::type_to_go_type(t_type* type) {
  return type_to_go_type_with_opt(type, false);
}

/**
 * Converts the parse type to a go type, taking into account whether the field
 * associated with the type is T_OPTIONAL.
 */
string t_go_generator::type_to_go_type_with_opt(t_type* type,
                                                bool optional_field) {
  string maybe_pointer(optional_field ? "*" : "");

  if (type->is_typedef() && ((t_typedef*)type)->is_forward_typedef()) {
    type = ((t_typedef*)type)->get_true_type();
  }

  if (type->is_base_type()) {
    t_base_type::t_base tbase = ((t_base_type*)type)->get_base();

    switch (tbase) {
    case t_base_type::TYPE_VOID:
      throw "";

    case t_base_type::TYPE_STRING:
      if (type->is_binary()) {
        return maybe_pointer + "[]byte";
      }

      return maybe_pointer + "string";

    case t_base_type::TYPE_BOOL:
      return maybe_pointer + "bool";

    case t_base_type::TYPE_I8:
      return maybe_pointer + "int8";

    case t_base_type::TYPE_I16:
      return maybe_pointer + "int16";

    case t_base_type::TYPE_I32:
      return maybe_pointer + "int32";

    case t_base_type::TYPE_I64:
      return maybe_pointer + "int64";

    case t_base_type::TYPE_DOUBLE:
      return maybe_pointer + "float64";
    }
  } else if (type->is_enum()) {
    return maybe_pointer + publicize(type_name(type));
  } else if (type->is_struct() || type->is_xception()) {
    return "*" + publicize(type_name(type));
  } else if (type->is_map()) {
    t_map* t = (t_map*)type;
    string keyType = type_to_go_key_type(t->get_key_type());
    string valueType = type_to_go_type(t->get_val_type());
    return maybe_pointer + string("map[") + keyType + "]" + valueType;
  } else if (type->is_set()) {
    t_set* t = (t_set*)type;
    string elemType = type_to_go_type(t->get_elem_type());
    return maybe_pointer + string("[]") + elemType;
  } else if (type->is_list()) {
    t_list* t = (t_list*)type;
    string elemType = type_to_go_type(t->get_elem_type());
    return maybe_pointer + string("[]") + elemType;
  } else if (type->is_typedef()) {
    return maybe_pointer + publicize(type_name(type));
  }

  throw "INVALID TYPE IN type_to_go_type: " + type->get_name();
}

/** See the comment inside generate_go_struct_definition for what this is. */
string t_go_generator::type_to_spec_args(t_type* ttype) {
  while (ttype->is_typedef()) {
    ttype = ((t_typedef*)ttype)->get_type();
  }

  if (ttype->is_base_type() || ttype->is_enum()) {
    return "nil";
  } else if (ttype->is_struct() || ttype->is_xception()) {
    return "(" + type_name(ttype) + ", " + type_name(ttype) + ".thrift_spec)";
  } else if (ttype->is_map()) {
    return "(" + type_to_enum(((t_map*)ttype)->get_key_type()) + ","
           + type_to_spec_args(((t_map*)ttype)->get_key_type()) + ","
           + type_to_enum(((t_map*)ttype)->get_val_type()) + ","
           + type_to_spec_args(((t_map*)ttype)->get_val_type()) + ")";
  } else if (ttype->is_set()) {
    return "(" + type_to_enum(((t_set*)ttype)->get_elem_type()) + ","
           + type_to_spec_args(((t_set*)ttype)->get_elem_type()) + ")";
  } else if (ttype->is_list()) {
    return "(" + type_to_enum(((t_list*)ttype)->get_elem_type()) + ","
           + type_to_spec_args(((t_list*)ttype)->get_elem_type()) + ")";
  }

  throw "INVALID TYPE IN type_to_spec_args: " + ttype->get_name();
}

// parses a string of struct tags into key/value pairs and writes them to the given map
void t_go_generator::parse_go_tags(map<string,string>* tags, const string in) {
  string key;
  string value;

  size_t mode=0; // 0/1/2 for key/value/whitespace
  size_t index=0;
  for (auto it=in.begin(); it<in.end(); ++it, ++index) {
      // Normally we start in key mode because the IDL is expected to be in
      // (go.tag="key:\"value\"") format, but if there is leading whitespace
      // we need to start in whitespace mode.
      if (index==0 && mode==0 && in[index]==' ') {
        mode=2;
      }

      if (mode==2) {
          if (in[index]==' ') {
              continue;
          }
          mode=0;
      }

      if (mode==0) {
          if (in[index]==':') {
              mode=1;
              index++;
              it++;
              continue;
          }
          key+=in[index];
      } else if (mode==1) {
          if (in[index]=='"') {
              (*tags)[key]=value;
              key=value="";
              mode=2;
              continue;
          }
          value+=in[index];
      }
  }
}

bool format_go_output(const string& file_path) {

  // formatting via gofmt deactivated due to THRIFT-3893
  // Please look at the ticket and make sure you fully understand all the implications
  // before submitting a patch that enables this feature again. Thank you.
  (void) file_path;
  return false;

  /*
  const string command = "gofmt -w " + file_path;

  if (system(command.c_str()) == 0) {
    return true;
  }

  fprintf(stderr, "WARNING - Running '%s' failed.\n", command.c_str());
  return false;
  */
 }

THRIFT_REGISTER_GENERATOR(go, "Go",
                          "    package_prefix=  Package prefix for generated files.\n" \
                          "    thrift_import=   Override thrift package import path (default:" + DEFAULT_THRIFT_IMPORT + ")\n" \
                          "    package=         Package name (default: inferred from thrift file name)\n" \
                          "    ignore_initialisms\n"
                          "                     Disable automatic spelling correction of initialisms (e.g. \"URL\")\n" \
                          "    read_write_private\n"
                          "                     Make read/write methods private, default is public Read/Write\n"
                          "    skip_remote\n"
                          "                     Skip the generating of -remote folders for the client binaries for services\n")

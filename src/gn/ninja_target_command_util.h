// Copyright 2018 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef TOOLS_GN_NINJA_TARGET_COMMAND_WRITER_H_
#define TOOLS_GN_NINJA_TARGET_COMMAND_WRITER_H_

#include "base/json/string_escape.h"
#include "gn/config_values_extractors.h"
#include "gn/escape.h"
#include "gn/filesystem_utils.h"
#include "gn/path_output.h"
#include "gn/target.h"
#include "gn/toolchain.h"

struct DefineWriter {
  DefineWriter() { options.mode = ESCAPE_NINJA_COMMAND; }
  DefineWriter(EscapingMode mode, bool escape_strings)
      : escape_strings(escape_strings) {
    options.mode = mode;
  }

  void operator()(const std::string& s, std::ostream& out) const {
    out << " ";
    if (escape_strings) {
      std::string dest;
      base::EscapeJSONString(s, false, &dest);
      EscapeStringToStream(out, "-D" + dest, options);
      return;
    }
    EscapeStringToStream(out, "-D" + s, options);
  }

  EscapeOptions options;
  bool escape_strings = false;
};

struct IncludeWriter {
  explicit IncludeWriter(PathOutput& path_output) : path_output_(path_output) {}
  ~IncludeWriter() = default;

  void operator()(const SourceDir& d, std::ostream& out) const {
    std::ostringstream path_out;
    path_output_.WriteDir(path_out, d, PathOutput::DIR_NO_LAST_SLASH);
    const std::string& path = path_out.str();
    if (path[0] == '"')
      out << " \"-I" << path.substr(1);
    else
      out << " -I" << path;
  }

  PathOutput& path_output_;
};

// has_precompiled_headers is set when this substitution matches a tool type
// that supports precompiled headers, and this target supports precompiled
// headers. It doesn't indicate if the tool has precompiled headers (this
// will be looked up by this function).
//
// The tool_type indicates the corresponding tool for flags that are
// tool-specific (e.g. "cflags_c"). For non-tool-specific flags (e.g.
// "defines") tool_type should be TYPE_NONE.
void WriteOneFlag(const Target* target,
                  const Substitution* subst_enum,
                  bool has_precompiled_headers,
                  const char* tool_name,
                  const std::vector<std::string>& (ConfigValues::*getter)()
                      const,
                  EscapeOptions flag_escape_options,
                  PathOutput& path_output,
                  std::ostream& out,
                  bool write_substitution = true);

// Fills |outputs| with the object or gch file for the precompiled header of the
// given type (flag type and tool type must match).
void GetPCHOutputFiles(const Target* target,
                       const char* tool_name,
                       std::vector<OutputFile>* outputs);

std::string GetGCCPCHOutputExtension(const char* tool_name);
std::string GetWindowsPCHObjectExtension(const char* tool_name,
                                         const std::string& obj_extension);

#endif  // TOOLS_GN_NINJA_TARGET_COMMAND_WRITER_H_

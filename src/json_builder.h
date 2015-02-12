// This file is part of MicroRestD <http://github.com/ufal/microrestd/>.
//
// Copyright 2015 Institute of Formal and Applied Linguistics, Faculty of
// Mathematics and Physics, Charles University in Prague, Czech Republic.
//
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

#include <cstring>
#include <vector>

#include "string_piece.h"

namespace ufal {
namespace microrestd {

// Declarations
class json_builder {
 public:
  // Clear
  inline json_builder& clear();

  // Encode
  inline json_builder& object();
  inline json_builder& array();
  inline json_builder& close();
  inline json_builder& key(string_piece str);
  inline json_builder& value(string_piece str);
  inline json_builder& value_xml_content(string_piece str);
  inline json_builder& value_open();
  inline json_builder& value_append(string_piece str);
  inline json_builder& value_xml_content_append(string_piece str);
  inline json_builder& value_close();
  inline json_builder& compact(bool compact);

  // Close all open objects and arrays
  inline json_builder& close_all();

  // Return current json
  inline string_piece current();

 private:
  // Remove current json prefix; for json_response_generator
  inline void discard_prefix(size_t length);

 private:
  inline void start_element(bool key);
  inline void encode(string_piece str);
  inline void encode_xml_content(string_piece str);

  std::vector<char> json;
  std::vector<char> stack;
  bool comma_needed = false;
  bool compacting = false;
};


// Definitions
json_builder& json_builder::clear() {
  json.clear();
  stack.clear();
  comma_needed = false;
  return *this;
}

json_builder& json_builder::object() {
  start_element(false);
  json.push_back('{');
  if (!compacting) json.push_back('\n');
  stack.push_back('}');
  comma_needed = false;
  return *this;
}

json_builder& json_builder::array() {
  start_element(false);
  json.push_back('[');
  if (!compacting) json.push_back('\n');
  stack.push_back(']');
  comma_needed = false;
  return *this;
}

json_builder& json_builder::close() {
  if (!stack.empty()) {
    if (!compacting) json.insert(json.end(), stack.size() - 1, ' ');
    json.push_back(stack.back());
    if (!compacting) json.push_back('\n');
    stack.pop_back();
    comma_needed = true;
  }
  return *this;
}

json_builder& json_builder::key(string_piece str) {
  start_element(true);
  json.push_back('"');
  encode(str);
  json.push_back('"');
  json.push_back(':');
  if (!compacting) json.push_back(' ');
  comma_needed = false;
  return *this;
}

json_builder& json_builder::value(string_piece str) {
  return value_open().value_append(str).value_close();
}

json_builder& json_builder::value_xml_content(string_piece str) {
  return value_open().value_xml_content_append(str).value_close();
}

json_builder& json_builder::value_open() {
  start_element(false);
  json.push_back('"');
  return *this;
}

json_builder& json_builder::value_append(string_piece str) {
  encode(str);
  return *this;
}

json_builder& json_builder::value_xml_content_append(string_piece str) {
  encode_xml_content(str);
  return *this;
}

json_builder& json_builder::value_close() {
  json.push_back('"');
  if (!compacting) json.push_back('\n');
  comma_needed = true;
  return *this;
}

json_builder& json_builder::compact(bool compact) {
  compacting = compact;
  if (!compacting && !json.empty() && json.back() != '\n') json.push_back('\n');
  return *this;
}

json_builder& json_builder::close_all() {
  while (!stack.empty()) close();
  return *this;
}

string_piece json_builder::current() {
  return string_piece(json.data(), json.size());
}

void json_builder::discard_prefix(size_t length) {
  if (!length) return;

  size_t json_size = json.size();
  if (length < json_size) {
    memmove(json.data(), json.data() + length, json_size - length);
    json.resize(json_size - length);
  } else {
    json.clear();
  }
}

void json_builder::start_element(bool key) {
  if (!compacting && !stack.empty() && (stack.back() != '}' || key)) json.insert(json.end(), stack.size() - (comma_needed ? 1 : 0), ' ');
  if (comma_needed && (stack.empty() || stack.back() != '}' || key)) json.push_back(',');
}

void json_builder::encode(string_piece str) {
  for (; str.len; str.str++, str.len--)
    switch (*str.str) {
      case '"': json.push_back('\\'); json.push_back('\"'); break;
      case '\\': json.push_back('\\'); json.push_back('\\'); break;
      case '\b': json.push_back('\\'); json.push_back('b'); break;
      case '\f': json.push_back('\\'); json.push_back('f'); break;
      case '\n': json.push_back('\\'); json.push_back('n'); break;
      case '\r': json.push_back('\\'); json.push_back('r'); break;
      case '\t': json.push_back('\\'); json.push_back('t'); break;
      default:
        if (((unsigned char)*str.str) < 32) {
          json.push_back('u'); json.push_back('0'); json.push_back('0'); json.push_back('0' + (*str.str >> 4)); json.push_back("0123456789ABCDEF"[*str.str & 0xF]);
        } else {
          json.push_back(*str.str);
        }
    }
}

void json_builder::encode_xml_content(string_piece str) {
  for (; str.len; str.str++, str.len--)
    switch (*str.str) {
      case '&': json.push_back('&'); json.push_back('a'); json.push_back('m'); json.push_back('p'); json.push_back(';'); break;
      case '<': json.push_back('&'); json.push_back('l'); json.push_back('t'); json.push_back(';'); break;
      case '>': json.push_back('&'); json.push_back('g'); json.push_back('t'); json.push_back(';'); break;
      case '"': json.push_back('&'); json.push_back('q'); json.push_back('u'); json.push_back('o'); json.push_back('t'); json.push_back(';'); break;
      case '\\': json.push_back('\\'); json.push_back('\\'); break;
      case '\b': json.push_back('\\'); json.push_back('b'); break;
      case '\f': json.push_back('\\'); json.push_back('f'); break;
      case '\n': json.push_back('\\'); json.push_back('n'); break;
      case '\r': json.push_back('\\'); json.push_back('r'); break;
      case '\t': json.push_back('\\'); json.push_back('t'); break;
      default:
        if (((unsigned char)*str.str) < 32) {
          json.push_back('u'); json.push_back('0'); json.push_back('0'); json.push_back('0' + (*str.str >> 4)); json.push_back("0123456789ABCDEF"[*str.str & 0xF]);
        } else {
          json.push_back(*str.str);
        }
    }
}

} // namespace microrestd
} // namespace ufal
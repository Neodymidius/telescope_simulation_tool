/*
 This file is part of SIXTE.

 SIXTE is free software: you can redistribute it and/or modify it
 under the terms of the GNU General Public License as published by
 the Free Software Foundation, either version 3 of the License, or
 any later version.

 SIXTE is distributed in the hope that it will be useful,
 but WITHOUT ANY WARRANTY; without even the implied warranty of
 MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 GNU General Public License for more details.

 For a copy of the GNU General Public License see
 <http://www.gnu.org/licenses/>.


 Copyright 2025 Remeis-Sternwarte, Friedrich-Alexander-Universitaet
                Erlangen-Nuernberg
*/

#include "lib/XMLData.h"

#include <filesystem>
#include <sstream>
#include <stack>
#include <cmath>

  template<typename T>
  std::string numberToString(const T& value) {
    if constexpr (std::is_integral_v<T>) {
      return std::to_string(value);
    } else {
      std::ostringstream oss;
      oss << std::setprecision(15) << value;
      return oss.str();
    }
  }

  template<typename T>
  void replaceVariableInAttributes(pugi::xml_node& xml_node, const std::string& variable_name, const T& replacement_value) {
    if (!xml_node) return;

    std::stack<pugi::xml_node> nodes_to_process;
    nodes_to_process.push(xml_node);

    const std::string replacement_string = numberToString(replacement_value);

    while (!nodes_to_process.empty()) {
      pugi::xml_node current_node = nodes_to_process.top();
      nodes_to_process.pop();

      // Process attributes of current node
      for (pugi::xml_attribute& attr: current_node.attributes()) {
        std::string attr_value(attr.value());

        // Replace all occurrences of the variable
        size_t search_pos = 0;
        while ((search_pos = attr_value.find(variable_name, search_pos)) != std::string::npos) {
          attr_value.replace(search_pos, variable_name.length(), replacement_string);
          search_pos += replacement_string.length();  // Move past the replacement
        }

        attr.set_value(attr_value.c_str());
      }

      // Add all children to stack for processing
      for (pugi::xml_node child_node: current_node.children()) {
        nodes_to_process.push(child_node);
      }
    }
  }

// Explicit template instantiations
  template void replaceVariableInAttributes<int>(pugi::xml_node&, const std::string&, const int&);
  template void replaceVariableInAttributes<double>(pugi::xml_node&, const std::string&, const double&);

  XMLNode::XMLNode(pugi::xml_node xml_node) : node_(xml_node) {}

  XMLNode XMLNode::child(const std::string& name) const {
    pugi::xml_node child_node = node_.child(name.c_str());
    if (!child_node) {
      throw NodeNotFound("XML node '" + std::string(node_.name()) +
                         "' does not contain required child node '" + name + "'");
    }
    return XMLNode(child_node);
  }

  std::optional<XMLNode> XMLNode::optionalChild(const std::string& name) const noexcept {
  pugi::xml_node child_node = node_.child(name.c_str());
  if (!child_node) {
  return std::nullopt;
}
return XMLNode(child_node);
}

std::vector<XMLNode> XMLNode::children(const std::string& name) const {
std::vector<XMLNode> result;
for (pugi::xml_node child : node_.children(name.c_str())) {
  result.emplace_back(child);
}
return result;
}

std::vector<XMLNode> XMLNode::allChildren() const {
std::vector<XMLNode> result;
for (pugi::xml_node child : node_.children()) {
  result.emplace_back(child);
}
return result;
}

bool XMLNode::hasChild(const std::string& name) const noexcept {
return node_.child(name.c_str());
}

bool XMLNode::hasAttribute(const std::string& name) const noexcept {
return node_.attribute(name.c_str());
}


int XMLNode::attributeAsInt(const std::string& name) const {
auto result = tryParseAttribute<int>(name);
if (!result) {
  throw AttributeNotFound("XML node '" + std::string(node_.name()) +
                          "' is missing required attribute '" + name + "'");
}
return *result;
}

int XMLNode::attributeAsIntOr(const std::string& name, int default_val) const {
auto result = tryParseAttribute<int>(name);
return result.value_or(default_val);
}

double XMLNode::attributeAsDouble(const std::string& name) const {
auto result = tryParseAttribute<double>(name);
if (!result) {
  throw AttributeNotFound("XML node '" + std::string(node_.name()) +
                          "' is missing required attribute '" + name + "'");
}
return *result;
}

double XMLNode::attributeAsDoubleOr(const std::string& name, double default_val) const {
auto result = tryParseAttribute<double>(name);
return result.value_or(default_val);
}

std::string XMLNode::attributeAsString(const std::string& name) const {
auto result = tryParseAttribute<std::string>(name);
if (!result) {
  throw AttributeNotFound("XML node '" + std::string(node_.name()) +
                          "' is missing required attribute '" + name + "'");
}
return *result;
}

std::string XMLNode::attributeAsStringOr(const std::string& name, const std::string& default_val) const noexcept {
auto result = tryParseAttribute<std::string>(name);
return result.value_or(default_val);
}

pugi::xml_node& XMLNode::node() {
return node_;
}

const pugi::xml_node& XMLNode::node() const {
return node_;
}

std::string XMLNode::name() const noexcept {
return node_.name();
}

XMLData::XMLData(const pugi::xml_document& xml_document, const std::string& xml_path) {
xml_document_.reset(xml_document);

expandAllLoops();

xml_dirname_ = std::filesystem::path(xml_path).parent_path().string() + "/";
}

XMLData::XMLData(const std::string& xml_filename) {
pugi::xml_parse_result result = xml_document_.load_file(xml_filename.c_str());
if (!result) {
  throw XMLDataException("Could not load XML file '" + xml_filename + "': " + result.description());
}

expandAllLoops();

xml_dirname_ = std::filesystem::path(xml_filename).parent_path().string() + "/";
}

XMLData::XMLData(const XMLData& xml_data) {
xml_document_.reset(xml_data.xml_document_);
xml_dirname_ = xml_data.xml_dirname_;
}

void XMLData::saveFile(const std::string& path) const {
if (!xml_document_.save_file(path.c_str())) {
  throw XMLDataException("Could not save XML as " + path);
}
}

void XMLData::expandAllLoops() {
// normal loops
for (;;) {
  auto loop_node_opt = findNodeByName("loop");
  if (!loop_node_opt) break;
  expandLoop(*loop_node_opt);
}

// hexagon loops
for (;;) {
  auto loop_node_opt = findNodeByName("hexagonloop");
  if (!loop_node_opt) break;
  expandHexLoop(*loop_node_opt);
}
}

std::optional<XMLNode> XMLData::findNodeByName(const std::string& node_name) const noexcept {
pugi::xml_node found_node = xml_document_.find_node([&node_name](pugi::xml_node node) {
  return std::string(node.name()) == node_name;
});
if (!found_node) {
return std::nullopt;
}
return XMLNode(found_node);
}

XMLNode XMLData::child(const std::string& name) const {
pugi::xml_node child_node = xml_document_.document_element().child(name.c_str());
if (!child_node) {
  throw NodeNotFound("XML document root does not contain required child node '" + name + "'");
}
return XMLNode(child_node);
}

std::optional<XMLNode> XMLData::optionalChild(const std::string& name) const noexcept {
pugi::xml_node child_node = xml_document_.document_element().child(name.c_str());
if (!child_node) {
return std::nullopt;
}
return XMLNode(child_node);
}

std::vector<XMLNode> XMLData::children(const std::string& name) const {
std::vector<XMLNode> result;
for (pugi::xml_node child : xml_document_.document_element().children(name.c_str())) {
  result.emplace_back(child);
}
return result;
}

bool XMLData::hasChild(const std::string& name) const noexcept {
return static_cast<bool>(xml_document_.document_element().child(name.c_str()));
}

std::string XMLData::dirname() const noexcept {
return xml_dirname_;
}

const pugi::xml_document& XMLData::document() const noexcept {
return xml_document_;
}

XMLNode XMLData::root() const {
return XMLNode(xml_document_.document_element());
}

void XMLData::expandLoop(const XMLNode& loop_node) {
int start = loop_node.attributeAsInt("start");
int end = loop_node.attributeAsInt("end");
int increment = loop_node.attributeAsInt("increment");
std::string variable = loop_node.attributeAsString("variable");

// get number of steps, which depends on the increment
int num_steps = (end - start) / increment;
if (num_steps < 0) {
  std::stringstream msg;
  msg << "Invalid XML loop with start=" << start << ", end=" << end << ", increment=" << increment << std::endl;
  throw XMLDataException(msg.str());
}
num_steps += 1; // need to count the first element, where ii=start
for (int step = 0; step < num_steps; step += 1) {
  int ii = start + increment * step;
  for (pugi::xml_node child: loop_node.node().children()) {
    pugi::xml_node new_node = loop_node.node().parent().insert_copy_before(child, loop_node.node());
    if (!new_node)
      throw XMLDataException("Could not expand loops in XML");

    replaceVariableInAttributes(new_node, variable, ii);
  }
}

// Remove old loop_node
if (!loop_node.node().parent().remove_child(loop_node.node())) {
  throw XMLDataException(std::string("Could not remove node '") + loop_node.node().name() +
                         "' from XML while expanding loops");
}
}

void XMLData::expandHexLoop(const XMLNode& loop_node) {
double radius = loop_node.attributeAsDouble("radius");
double pixelpitch = loop_node.attributeAsDouble("pixelpitch");
bool cross = 1 == loop_node.attributeAsInt("cross");

int n_pixels_tot = 0;
int n_pixels_line = 0;
double current_radius = 0.;
double current_height;
double line_number;

// handle both upper (sign==1) and lower (sign==-1) parts
for (int sign : {1,-1}) {
  if (cross) {
    current_height = .5 * pixelpitch;
    line_number = sign * .5;
  } else {
    current_height = (sign == 1 ? 0. : pixelpitch);
    line_number = (sign == 1 ? 0 : -1);
  }

  while (current_height < 0.5*radius*std::sqrt(3.)) {
    current_radius = radius - current_height / tan(M_PI/3.);

    if (cross) {
      n_pixels_line = 2*floor(current_radius / pixelpitch + .5);
    } else {
      n_pixels_line = 2*floor(current_radius / pixelpitch) + 1;
    }
    n_pixels_tot += n_pixels_line;

    for (int ii=0; ii<n_pixels_line; ii++) {
      // calculate X and Y
      double posx = ii-(n_pixels_line-1)/2.;
      double posy = line_number;

      // copy the loop node, substitute variables and insert
      for (pugi::xml_node child: loop_node.node().children()) {
        pugi::xml_node new_node = loop_node.node().parent().insert_copy_before(child, loop_node.node());
        if (!new_node)
          throw XMLDataException("Could not expand loops in XML");

        replaceVariableInAttributes(new_node, "$p", pixelpitch);
        replaceVariableInAttributes(new_node, "$x", posx);
        replaceVariableInAttributes(new_node, "$y", posy);
      }
    }
    current_height += pixelpitch;
    line_number += sign;
  }
}

// Remove old loop_node
if (!loop_node.node().parent().remove_child(loop_node.node())) {
  throw XMLDataException(std::string("Could not remove node '") + loop_node.node().name() +
                         "' from XML while expanding hexagon loops");
}
}




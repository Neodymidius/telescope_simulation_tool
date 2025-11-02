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

#pragma once

#include <pugixml.hpp>
#include <stdexcept>
#include <string>
#include <vector>
#include <optional>
#include <limits>

class XMLDataException : public std::runtime_error {
public:
    explicit XMLDataException(const std::string& message)
            : std::runtime_error(message) {}
};

class NodeNotFound : public XMLDataException {
public:
    explicit NodeNotFound(const std::string& message)
            : XMLDataException(message) {}
};

class AttributeNotFound : public XMLDataException {
public:
    explicit AttributeNotFound(const std::string& message)
            : XMLDataException(message) {}
};

class AttributeParseError : public XMLDataException {
public:
    explicit AttributeParseError(const std::string& message)
            : XMLDataException(message) {}
};

template<typename T>
void replaceVariableInAttributes(pugi::xml_node& xml_node, const std::string& variable_name, const T& replacement_value);

/**
* @brief Wrapper for XML node with fluent interface.
*/
class XMLNode {
public:
    /**
     * @brief Construct XMLNode wrapper around pugixml node.
     * @param xml_node The pugixml node to wrap
     */
    explicit XMLNode(pugi::xml_node xml_node);

    /**
     * @brief Navigate to named child node.
     * @param name Name of child node
     * @return XMLNode wrapper for child
     * @throws NodeNotFound if child doesn't exist
     */
    [[nodiscard]] XMLNode child(const std::string& name) const;

    /**
     * @brief Navigate to optional named child node.
     * @param name Name of child node
     * @return Optional XMLNode wrapper, empty if child doesn't exist
     */
    [[nodiscard]] std::optional<XMLNode> optionalChild(const std::string& name) const noexcept;

    /**
     * @brief Get collection of all children with given name.
     * @param name Name of child nodes
     * @return Vector of XMLNode wrappers for all matching children
     */
    [[nodiscard]] std::vector<XMLNode> children(const std::string& name) const;

    /**
     * @brief Get collection of all child nodes.
     * @return Vector of XMLNode wrappers for all children
     */
    [[nodiscard]] std::vector<XMLNode> allChildren() const;

    /**
     * @brief Check if named child node exists.
     * @param name Name of child node
     * @return True if child exists, false otherwise
     */
    [[nodiscard]] bool hasChild(const std::string& name) const noexcept;

    /**
     * @brief Check if named attribute exists.
     * @param name Name of attribute
     * @return True if attribute exists, false otherwise
     */
    [[nodiscard]] bool hasAttribute(const std::string& name) const noexcept;

    /**
     * @brief Extract attribute value as integer.
     * @param name Name of attribute
     * @return Parsed integer value
     * @throws AttributeNotFound if attribute doesn't exist
     * @throws AttributeParseError if attribute cannot be parsed
     */
    [[nodiscard]] int attributeAsInt(const std::string& name) const;

    /**
     * @brief Extract attribute value as integer with default fallback.
     * @param name Name of attribute
     * @param default_val Value to return if attribute doesn't exist
     * @return Parsed integer value or default
     * @throws AttributeParseError if attribute exists but cannot be parsed
     */
    [[nodiscard]] int attributeAsIntOr(const std::string& name, int default_val) const;

    /**
     * @brief Extract attribute value as double.
     * @param name Name of attribute
     * @return Parsed double value
     * @throws AttributeNotFound if attribute doesn't exist
     * @throws AttributeParseError if attribute cannot be parsed
     */
    [[nodiscard]] double attributeAsDouble(const std::string& name) const;

    /**
     * @brief Extract attribute value as double with default fallback.
     * @param name Name of attribute
     * @param default_val Value to return if attribute doesn't exist
     * @return Parsed double value or default
     * @throws AttributeParseError if attribute exists but cannot be parsed
     */
    [[nodiscard]] double attributeAsDoubleOr(const std::string& name, double default_val) const;

    /**
     * @brief Extract attribute value as string.
     * @param name Name of attribute
     * @return String value
     * @throws AttributeNotFound if attribute doesn't exist
     */
    [[nodiscard]] std::string attributeAsString(const std::string& name) const;

    /**
     * @brief Extract attribute value as string with default fallback.
     * @param name Name of attribute
     * @param default_val Value to return if attribute doesn't exist
     * @return String value or default
     */
    [[nodiscard]] std::string attributeAsStringOr(const std::string& name, const std::string& default_val) const noexcept;

    /**
     * @brief Access underlying pugixml node.
     * @return Reference to pugi::xml_node
     */
    pugi::xml_node& node();

    /**
     * @brief Access underlying pugixml node (const version).
     * @return Const reference to pugi::xml_node
     */
    [[nodiscard]] const pugi::xml_node& node() const;

    /**
     * @brief Get node name.
     * @return Node name as string
     */
    [[nodiscard]] std::string name() const noexcept;

private:
    /**
     * @brief Helper template to parse attributes with type safety.
     * @param name Name of attribute
     * @return Optional parsed value, empty if attribute doesn't exist
     * @throws AttributeParseError if parsing fails
     */
    template<typename T>
    [[nodiscard]] std::optional<T> tryParseAttribute(const std::string& name) const;

    pugi::xml_node node_;
};

// After the XMLNode class definition in XMLData.h

template<> inline std::optional<std::string>
XMLNode::tryParseAttribute<std::string>(const std::string& name) const {
    auto attr = node_.attribute(name.c_str());
    if (!attr) return std::nullopt;
    return std::string(attr.value());
}

template<> inline std::optional<int>
XMLNode::tryParseAttribute<int>(const std::string& name) const {
    auto attr = node_.attribute(name.c_str());
    if (!attr) return std::nullopt;

    const char* s = attr.value();
    char* end = nullptr;
    errno = 0;
    long v = std::strtol(s, &end, 10);
    if (errno != 0 || end == s || *end != '\0') {
        throw AttributeParseError("Failed to parse int attribute '" + name + "'");
    }
    if (v < std::numeric_limits<int>::min() || v > std::numeric_limits<int>::max()) {
        throw AttributeParseError("Int out of range for attribute '" + name + "'");
    }
    return static_cast<int>(v);
}

template<> inline std::optional<double>
XMLNode::tryParseAttribute<double>(const std::string& name) const {
    auto attr = node_.attribute(name.c_str());
    if (!attr) return std::nullopt;

    const char* s = attr.value();
    char* end = nullptr;
    errno = 0;
    double v = std::strtod(s, &end);
    if (errno != 0 || end == s || *end != '\0') {
        throw AttributeParseError("Failed to parse double attribute '" + name + "'");
    }
    return v;
}


/**
* @brief Wrapper for XML document with fluent interface.
*/
class XMLData {
public:
    /**
     * @brief Construct from already parsed XML document.
     * @param xml_document Already parsed pugixml document
     * @param xml_path File path for directory resolution
     */
    XMLData(const pugi::xml_document& xml_document, const std::string& xml_path);

    /**
     * @brief Construct by loading XML from file.
     * @param xml_filename Path to XML file to load
     * @throws XMLDataException if file cannot be loaded
     */
    explicit XMLData(const std::string& xml_filename);

    /**
     * @brief Copy constructor.
     * @param xml_data XMLData instance to copy
     */
    XMLData(const XMLData& xml_data);

    /**
     * @brief Save XML document to file.
     * @param path Output file path
     * @throws XMLDataException if save fails
     */
    void saveFile(const std::string& path) const;

    /**
     * @brief Expand all loop and hexagonloop elements in document.
     */
    void expandAllLoops();

    /**
     * @brief Navigate to named child node of document root.
     * @param name Name of child node
     * @return XMLNode wrapper
     * @throws NodeNotFound if child doesn't exist
     */
    [[nodiscard]] XMLNode child(const std::string& name) const;

    /**
     * @brief Navigate to optional named child node of document root.
     * @param name Name of child node
     * @return Optional XMLNode wrapper, empty if child doesn't exist
     */
    [[nodiscard]] std::optional<XMLNode> optionalChild(const std::string& name) const noexcept;

    /**
     * @brief Get collection of all root children with given name.
     * @param name Name of child nodes
     * @return Vector of XMLNode wrappers for all matching children
     */
    [[nodiscard]] std::vector<XMLNode> children(const std::string& name) const;

    /**
     * @brief Check if named child node exists in document root.
     * @param name Name of child node
     * @return True if child exists, false otherwise
     */
    [[nodiscard]] bool hasChild(const std::string& name) const noexcept;

    /**
     * @brief Get directory path of XML file.
     * @return Directory path (includes trailing slash)
     */
    [[nodiscard]] std::string dirname() const noexcept;

    /**
     * @brief Access underlying pugixml document.
     * @return Const reference to pugi::xml_document
     */
    [[nodiscard]] const pugi::xml_document& document() const noexcept;

    /**
     * @brief Access document root element as XMLNode.
     * @return XMLNode wrapper for document root element
     */
    [[nodiscard]] XMLNode root() const;

    /**
     * @brief Find first node with given name anywhere in document tree.
     * @param node_name Name of node to find
     * @return Optional XMLNode wrapper, empty if node doesn't exist
     */
    [[nodiscard]] std::optional<XMLNode> findNodeByName(const std::string& node_name) const noexcept;

private:

    static void expandLoop(const XMLNode& loop_node);
    static void expandHexLoop(const XMLNode& loop_node);

    pugi::xml_document xml_document_;
    std::string xml_dirname_;
};


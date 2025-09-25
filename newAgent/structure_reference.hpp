#pragma once

#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include "third_party/nlohmann_json/single_include/nlohmann/json.hpp"

using json = nlohmann::json;

class StructureReference {
private:
    json structures_data;

public:
    // Constructor: Load the JSON file
    StructureReference(const std::string& json_file_path) {
        std::ifstream file(json_file_path);
        if (!file.is_open()) {
            throw std::runtime_error("Failed to open JSON file: " + json_file_path);
        }
        file >> structures_data;
        if (!structures_data.contains("structures") || !structures_data["structures"].is_array()) {
            throw std::runtime_error("Invalid JSON structure: Missing or invalid 'structures' array");
        }
    }

    // Get all structure IDs
    std::vector<std::string> getStructureIds() const {
        std::vector<std::string> ids;
        for (const auto& structure : structures_data["structures"]) {
            if (structure.contains("id")) {
                ids.push_back(structure["id"]);
            }
        }
        return ids;
    }

    // Get positive tags for a specific structure ID
    std::vector<std::string> getPositiveTags(const std::string& id) const {
        for (const auto& structure : structures_data["structures"]) {
            if (structure.contains("id") && structure["id"] == id && structure.contains("positive_tags")) {
                return structure["positive_tags"].get<std::vector<std::string>>();
            }
        }
        return {};  // Return empty if not found
    }

    // Get negative tags for a specific structure ID
    std::vector<std::string> getNegativeTags(const std::string& id) const {
        for (const auto& structure : structures_data["structures"]) {
            if (structure.contains("id") && structure["id"] == id && structure.contains("negative_tags")) {
                return structure["negative_tags"].get<std::vector<std::string>>();
            }
        }
        return {};  // Return empty if not found
    }

    // Get full description for a structure (for inclusion in prompts)
    std::string getStructureDescription(const std::string& id) const {
        std::string desc = "Structure: " + id + "\n";
        auto pos_tags = getPositiveTags(id);
        auto neg_tags = getNegativeTags(id);
        if (!pos_tags.empty()) {
            desc += "Positive Tags: ";
            for (size_t i = 0; i < pos_tags.size(); ++i) {
                desc += pos_tags[i];
                if (i < pos_tags.size() - 1) desc += ", ";
            }
            desc += "\n";
        }
        if (!neg_tags.empty()) {
            desc += "Negative Tags: ";
            for (size_t i = 0; i < neg_tags.size(); ++i) {
                desc += neg_tags[i];
                if (i < neg_tags.size() - 1) desc += ", ";
            }
            desc += "\n";
        }
        return desc;
    }

    // Get all structures as a formatted string (for context in prompts)
    std::string getAllStructuresContext() const {
        std::string context = "Available GLSL Structures:\n";
        for (const auto& structure : structures_data["structures"]) {
            if (structure.contains("id")) {
                context += getStructureDescription(structure["id"]) + "\n";
            }
        }
        return context;
    }
};
#pragma once
#include <vector>
#include <nlohmann/json.hpp>

using json = nlohmann::json;

namespace Tools {

    static json send_message = {
        {"type", "function"},
        {"function", {
            {"name", "send_message"},
            {"description", "Send a message to the user."},
            {"parameters", {
                {"type", "object"},
                {"properties", {
                    {"message", {
                        {"type", "string"},
                        {"description", "The content of the message to send."}
                    }}
                }},
                {"required", {"message"}}
            }}
        }}
    };

    static json core_memory_append = {
        {"type", "function"},
        {"function", {
            {"name", "core_memory_append"},
            {"description", "Append to the contents of a specific memory block."},
            {"parameters", {
                {"type", "object"},
                {"properties", {
                    {"label", {
                        {"type", "string"},
                        {"description", "The label of the memory block (e.g. 'human', 'persona')."}
                    }},
                    {"content", {
                        {"type", "string"},
                        {"description", "The content to append."}
                    }}
                }},
                {"required", {"label", "content"}}
            }}
        }}
    };

    static json core_memory_replace = {
        {"type", "function"},
        {"function", {
            {"name", "core_memory_replace"},
            {"description", "Replace the contents of a specific memory block with new content."},
            {"parameters", {
                {"type", "object"},
                {"properties", {
                    {"label", {
                        {"type", "string"},
                        {"description", "The label of the memory block (e.g. 'human', 'persona')."}
                    }},
                    {"old_content", {
                        {"type", "string"},
                        {"description", "The content to replace. Must match exactly."}
                    }},
                    {"new_content", {
                        {"type", "string"},
                        {"description", "The new content."}
                    }}
                }},
                {"required", {"label", "old_content", "new_content"}}
            }}
        }}
    };

    static std::vector<json> get_all_tools() {
        return {send_message, core_memory_append, core_memory_replace};
    }
}

#include "LLMClient.hpp"
#include <cpr/cpr.h>
#include <iostream>

LLMClient::LLMClient(const std::string& api_key, const std::string& base_url, const std::string& model)
    : api_key(api_key), base_url(base_url), model(model) {}

void LLMClient::printDebug(const std::string& label, const std::string& content) {
    std::cout << "[DEBUG] " << label << ": " << content << std::endl;
}

json LLMClient::chatCompletion(const std::vector<json>& messages, const std::vector<json>& tools) {
    if (base_url.find("googleapis.com") != std::string::npos) {
        // --- Gemini Native API Adapter ---

        // 1. Convert Messages
        json contents = json::array();
        json system_instruction;
        bool has_system = false;

        for (const auto& msg : messages) {
            std::string role = msg.value("role", "");
            if (role == "system") {
                system_instruction = {
                    {"role", "user"}, // System instruction often passed as parts, or specific field. v1beta supports system_instruction field.
                    {"parts", {{{"text", msg.value("content", "")}}}}
                };
                // Gemini v1beta supports "system_instruction" at top level, but role must be "system"??
                // Actually, "system_instruction" field is separate. content role should be "model" or "user".
                // Let's use the explicit 'system_instruction' field.
                system_instruction = {
                    {"parts", {{{"text", msg.value("content", "")}}}}
                };
                has_system = true;
            } else if (role == "user") {
                contents.push_back({
                    {"role", "user"},
                    {"parts", {{{"text", msg.value("content", "")}}}}
                });
            } else if (role == "assistant") {
                json parts = json::array();
                if (msg.contains("tool_calls")) {
                    for (const auto& tc : msg["tool_calls"]) {
                        json func_call = {
                            {"name", tc["function"]["name"]},
                            {"args", json::parse(tc["function"]["arguments"].get<std::string>())}
                        };
                        parts.push_back({{"functionCall", func_call}});
                    }
                } else {
                    parts.push_back({{"text", msg.value("content", "")}});
                }
                contents.push_back({
                    {"role", "model"},
                    {"parts", parts}
                });
            } else if (role == "tool") {
                // Map to 'function' role
                // Gemini expects 'functionResponse' object
                json function_response = {
                    {"name", msg.value("name", "")},
                    {"response", {
                        {"content", msg.value("content", "")} // Wrap simple content or parse if json? verify.
                        // Gemini response args should be object. If content is json string, parse it. 
                        // If it's just text, wrap it. Let's try to parse if looks like json, else wrap.
                    }}
                };
                
                // Try parsing content as JSON for better structured response
                try {
                     json content_json = json::parse(msg.value("content", ""));
                     function_response["response"] = content_json;
                } catch(...) {
                     function_response["response"] = {{"result", msg.value("content", "")}};
                }

                contents.push_back({
                    {"role", "function"},
                    {"parts", {{{"functionResponse", function_response}}}}
                });
            }
        }

        // 2. Convert Tools
        json gemini_tools = json::array();
        if (!tools.empty()) {
            json funcs = json::array();
            for (const auto& t : tools) {
                if (t["type"] == "function") {
                    funcs.push_back(t["function"]);
                }
            }
            gemini_tools.push_back({{"function_declarations", funcs}});
        }

        // 3. Construct Payload
        json payload = {
            {"contents", contents}
        };
        if (has_system) {
            payload["system_instruction"] = system_instruction;
        }
        if (!gemini_tools.empty()) {
            payload["tools"] = gemini_tools;
        }

        // 4. Send Request (Native URL)
        // URL: base_url + "/models/" + model + ":generateContent?key=" + api_key
        // Note: base_url is https://generativelanguage.googleapis.com/v1beta
        std::string full_url = base_url + "/models/" + model + ":generateContent?key=" + api_key;

        // printDebug("Gemini Payload", payload.dump(2));

        cpr::Response r = cpr::Post(
            cpr::Url{full_url},
            cpr::Header{{"Content-Type", "application/json"}},
            cpr::Body{payload.dump()}
        );

        if (r.status_code != 200) {
            std::cerr << "Error: Gemini API request failed with status " << r.status_code << std::endl;
            std::cerr << "Response: " << r.text << std::endl;
            return {{"error", r.text}};
        }

        // 5. Adapt Response
        try {
            json gemini_resp = json::parse(r.text);
            json choice_msg = {{"role", "assistant"}};
            
            // Check candidates
            if (gemini_resp.contains("candidates") && !gemini_resp["candidates"].empty()) {
                auto& candidate = gemini_resp["candidates"][0];
                if (candidate.contains("content") && candidate["content"].contains("parts")) {
                    auto& parts = candidate["content"]["parts"];
                    
                    json tool_calls = json::array();
                    std::string text_content = "";

                    for (const auto& part : parts) {
                        if (part.contains("text")) {
                            text_content += part["text"].get<std::string>();
                        } else if (part.contains("functionCall")) {
                            auto& fc = part["functionCall"];
                            tool_calls.push_back({
                                {"id", "call_" + fc["name"].get<std::string>()}, // Fake ID
                                {"type", "function"},
                                {"function", {
                                    {"name", fc["name"]},
                                    {"arguments", fc["args"].dump()} // OpenAI expects stringified JSON
                                }}
                            });
                        }
                    }

                    if (!text_content.empty()) choice_msg["content"] = text_content;
                    else choice_msg["content"] = nullptr;
                    
                    if (!tool_calls.empty()) choice_msg["tool_calls"] = tool_calls;
                }
            }

            return {
                {"choices", {
                    {{"message", choice_msg}}
                }}
            };

        } catch (json::parse_error& e) {
            return {{"error", "JSON parse error"}};
        }
    }

    // --- Original OpenAI Logic ---
    json payload = {
        {"model", model},
        {"messages", messages}
    };

    if (!tools.empty()) {
        payload["tools"] = tools;
        payload["tool_choice"] = "auto";
    }

    // printDebug("Request Payload", payload.dump(2));

    cpr::Response r = cpr::Post(
        cpr::Url{base_url + "/chat/completions"},
        cpr::Header{
            {"Authorization", "Bearer " + api_key},
            {"Content-Type", "application/json"}
        },
        cpr::Body{payload.dump()}
    );

    if (r.status_code != 200) {
        std::cerr << "Error: API request failed with status " << r.status_code << std::endl;
        std::cerr << "Response: " << r.text << std::endl;
        return {{"error", r.text}};
    }

    try {
        json response = json::parse(r.text);
        return response;
    } catch (json::parse_error& e) {
        std::cerr << "Error: Failed to parse JSON response: " << e.what() << std::endl;
        return {{"error", "JSON parse error"}};
    }
}

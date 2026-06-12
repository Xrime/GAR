//
// Created by xint2 on 11/06/2026.
//
#include "../include/ui/tui_app.h"
#include "../include/ui/settings.h"
#include "../include/ui/renderer.h"
#include  "../include/security/header_analyzer.h"
#include "../include/security/tls_inspector.h"
#include "../include/security/request_replay.h"
#include "../include/anonymity/tor_control.h"
#include "../include/anonymity/fingerprint.h"
#include "../include/core/dns_resolver.h"
#include  <ftxui/component/component.hpp>
#include <ftxui/component/screen_interactive.hpp>
#include <ftxui/dom/elements.hpp>
#include <sstream>
#include <chrono>
#include <functional>
#include <algorithm>
#include <thread>
#include <utility>
#include <cstdlib>
#include <gumbo.h>

using namespace ftxui;
namespace gar::ui {
    static std::string trim(std::string s) {
        while (!s.empty() && (s.front() == ' ' || s.front() == '\t' || s.front() == '\r')) s.erase(s.begin());
        while (!s.empty() && (s.back() == ' ' || s.back() == '\t' || s.back() == '\r' || s.back() == '\n')) s.pop_back();
        return s;
    }
    static std::string url_encode(const std::string& s) {
        std::ostringstream out;
        for (unsigned char c : s) {
            if (std::isalnum(c) || c == '-' || c == '_' || c == '.' || c == '~') out << c;
            else out << '%' << std::uppercase << std::hex << (int)c;
        }
        return out.str();
    }
    std::string tuiApp::normalizeUrl(std::string input) {
        input = trim(input);
        if (input.empty()) return input;
        if (input.find(' ') != std::string::npos || input.find('.') == std::string::npos) {
            return "https://lite.duckduckgo.com/lite/?q=" + url_encode(input);
        }

        if (input.rfind("http://", 0) == 0 || input.rfind("https://", 0) == 0) return input;
        return "https://" + input;
    }
    std::string tuiApp::makeAbsoluteUrl(const std::string& base_url, const std::string& href) {
        if (href.empty()) return "";
        if (href.rfind("http://", 0) == 0 || href.rfind("https://", 0) == 0) return href;
        if (href.rfind("//", 0) == 0) return "https:" + href;
        if (href.rfind("javascript:", 0) == 0 || href.rfind("mailto:", 0) == 0) return "";
        size_t scheme_pos = base_url.find("://");
        if (scheme_pos == std::string::npos) return href;
        size_t host_start = scheme_pos + 3;
        size_t path_start = base_url.find('/', host_start);
        std::string root = (path_start == std::string::npos) ? base_url : base_url.substr(0, path_start);
        if (!href.empty() && href[0] == '/') return root + href;
        std::string base_dir = base_url;
        size_t last_slash = base_dir.find_last_of('/');
        if (last_slash != std::string::npos && last_slash > host_start) {
            base_dir = base_dir.substr(0, last_slash + 1);
        } else {
            base_dir = root + "/";
        }
        return base_dir + href;
    }std::string tuiApp::htmlToText(const std::string& html) {
        GumboOutput* output = gumbo_parse(html.c_str());
        if (!output) return "Error parsing HTML";
        std::string text_content;
        using GumboWalkFunc = std::function<void(GumboNode*)>;

        GumboWalkFunc walk = [&](auto* node) {
            if (!node) return;
            if (node->type == GUMBO_NODE_TEXT) {
                text_content += node->v.text.text;
                text_content += " ";
                return;
            }
            if (node->type == GUMBO_NODE_ELEMENT) {
                GumboTag tag = node->v.element.tag;
                if (tag == GUMBO_TAG_SCRIPT || tag == GUMBO_TAG_STYLE || tag == GUMBO_TAG_HEAD || tag == GUMBO_TAG_NOSCRIPT) return;
                if (tag == GUMBO_TAG_P || tag == GUMBO_TAG_DIV || tag == GUMBO_TAG_BR || tag == GUMBO_TAG_LI || tag == GUMBO_TAG_H1 || tag == GUMBO_TAG_H2) {
                    text_content += "\n";
                }
                GumboVector* children = &node->v.element.children;
                for (unsigned int i = 0; i < children->length; ++i) {
                    walk(static_cast<GumboNode*>(children->data[i]));
                }
                if (tag == GUMBO_TAG_P || tag == GUMBO_TAG_DIV || tag == GUMBO_TAG_H1 || tag == GUMBO_TAG_H2) {
                    text_content += "\n";
                }
            }
        };
        walk(output->root);
        gumbo_destroy_output(&kGumboDefaultOptions, output);
        struct Replace {
            static void all(std::string& str, const std::string& from, const std::string& to) {
                size_t p = 0;
                while ((p = str.find(from, p)) != std::string::npos) {
                    str.replace(p, from.size(), to);
                    p += to.size();
                }
            }
        };
        Replace::all(text_content, "&amp;",  "&");
        Replace::all(text_content, "&lt;",   "<");
        Replace::all(text_content, "&gt;",   ">");
        Replace::all(text_content, "&quot;", "\"");
        Replace::all(text_content, "&#39;",  "'");
        Replace::all(text_content, "&nbsp;", " ");
        std::string final_text;
        bool last_was_newline = false;
        for (char c : text_content) {
            if (c == '\n' || c == '\r') {
                if (!last_was_newline) {
                    final_text += '\n';
                    last_was_newline = true;
                }
            } else {
                final_text += c;
                last_was_newline = false;
            }
        }
        return final_text;
    }
    void tuiApp::extractLinksFromHtml(const std::string& html, const std::string& base_url) {
        links_.clear();
        GumboOutput* output = gumbo_parse(html.c_str());
        if (!output) return;

        using GumboWalkFunc = std::function<void(GumboNode*)>;
        GumboWalkFunc walk = [&](auto* node) {
            if (!node || links_.size() >= 50) return;
            if (node->type == GUMBO_NODE_ELEMENT && node->v.element.tag == GUMBO_TAG_A) {
                GumboAttribute* href_attr = gumbo_get_attribute(&node->v.element.attributes, "href");
                if (href_attr && href_attr->value) {
                    std::string full = makeAbsoluteUrl(base_url, href_attr->value);
                    if (!full.empty()) {
                        linkItem item;
                        item.url   = full;
                        item.label = (full.size() > 60) ? full.substr(0, 57) + "..." : full;
                        links_.push_back(std::move(item));
                    }
                }
            }
            if (node->type == GUMBO_NODE_ELEMENT) {
                GumboVector* children = &node->v.element.children;
                for (unsigned int i = 0; i < children->length; ++i)
                    walk(static_cast<GumboNode*>(children->data[i]));
            }
        };
        walk(output->root);
        gumbo_destroy_output(&kGumboDefaultOptions, output);
    }

    void tuiApp::extractMediaFromHtml(const std::string& html, const std::string& base_url) {
        media_.clear();
        GumboOutput* output = gumbo_parse(html.c_str());
        if (!output) return;

        using GumboWalkFunc = std::function<void(GumboNode*)>;
        GumboWalkFunc walk = [&](auto* node) {
            if (!node || media_.size() >= 50) return;
            if (node->type == GUMBO_NODE_ELEMENT) {
                auto tag = node->v.element.tag;

                auto try_add = [&](const char* type_str, const char* attr_str) {
                    GumboAttribute* attr = gumbo_get_attribute(&node->v.element.attributes, attr_str);
                    if (attr && attr->value) {
                        std::string url = makeAbsoluteUrl(base_url, attr->value);
                        if (!url.empty()) {
                            mediaItem mi;
                            mi.type = type_str;
                            mi.url  = url;
                            media_.push_back(std::move(mi));
                        }
                    }
                };

                if      (tag == GUMBO_TAG_IMG)    try_add("image", "src");
                else if (tag == GUMBO_TAG_VIDEO)  try_add("video", "src");
                else if (tag == GUMBO_TAG_AUDIO)  try_add("audio", "src");

                GumboVector* children = &node->v.element.children;
                for (unsigned int i = 0; i < children->length; ++i)
                    walk(static_cast<GumboNode*>(children->data[i]));
            }
        };
        walk(output->root);
        gumbo_destroy_output(&kGumboDefaultOptions, output);
    }

    void tuiApp::extractFormFromHtml(const std::string& html, const std::string& base_url) {
        forms_.clear();
        GumboOutput* output = gumbo_parse(html.c_str());
        if (!output) return;

        using GumboWalkFunc = std::function<void(GumboNode*)>;
        GumboWalkFunc walk = [&](auto* node) {
            if (!node) return;

            if (node->type == GUMBO_NODE_ELEMENT && node->v.element.tag == GUMBO_TAG_FORM) {
                formItem form;

                GumboAttribute* action_attr = gumbo_get_attribute(&node->v.element.attributes, "action");
                form.action = action_attr ? makeAbsoluteUrl(base_url, action_attr->value) : base_url;
                if (form.action.empty()) form.action = base_url;

                GumboAttribute* method_attr = gumbo_get_attribute(&node->v.element.attributes, "method");
                form.method = method_attr ? method_attr->value : "GET";
                for (char& c : form.method) c = (char)toupper((unsigned char)c);

                using InputsWalkFn = std::function<void(GumboNode*)>;
                InputsWalkFn walk_inputs = [&](auto* n) {
                    if (!n) return;
                    if (n->type == GUMBO_NODE_ELEMENT) {
                        if (n->v.element.tag == GUMBO_TAG_INPUT) {
                            GumboAttribute* name_a  = gumbo_get_attribute(&n->v.element.attributes, "name");
                            GumboAttribute* type_a  = gumbo_get_attribute(&n->v.element.attributes, "type");
                            GumboAttribute* value_a = gumbo_get_attribute(&n->v.element.attributes, "value");
                            formInput fi;
                            fi.name  = name_a  ? name_a->value  : "";
                            fi.type  = type_a  ? type_a->value  : "text";
                            fi.value = value_a ? value_a->value : "";
                            for (char& c : fi.type) c = (char)tolower((unsigned char)c);
                            if (!fi.name.empty()) form.inputs.push_back(fi);

                        } else if (n->v.element.tag == GUMBO_TAG_TEXTAREA) {
                            GumboAttribute* name_a = gumbo_get_attribute(&n->v.element.attributes, "name");
                            formInput fi;
                            fi.name  = name_a ? name_a->value : "";
                            fi.type  = "textarea";
                            fi.value = "";
                            if (!fi.name.empty()) form.inputs.push_back(fi);
                        }

                        GumboVector* children = &n->v.element.children;
                        for (unsigned int i = 0; i < children->length; ++i)
                            walk_inputs(static_cast<GumboNode*>(children->data[i]));
                    }
                };

                walk_inputs(node);
                if (!form.inputs.empty()) forms_.push_back(std::move(form));
                return;
            }

            if (node->type == GUMBO_NODE_ELEMENT) {
                GumboVector* children = &node->v.element.children;
                for (unsigned int i = 0; i < children->length; ++i)
                    walk(static_cast<GumboNode*>(children->data[i]));
            }
        };

        walk(output->root);
        gumbo_destroy_output(&kGumboDefaultOptions, output);
    }
void tuiApp::Navigate(const std::string& url) {
    if (url.empty()) return;
    auto start = std::chrono::steady_clock::now();
    auto resp = http_client_->get(url);
    auto end = std::chrono::steady_clock::now();
    long load_time = std::chrono::duration_cast<std::chrono::milliseconds>(end - start).count();
    std::lock_guard<std::mutex> lock(state_mutex_);
    is_loading = false;
    current_url_       = url;
    last_capture_url_ = url;
    last_response_     = resp;
    last_load_time_ms_ = load_time;

    if (!last_response_.success) {
        status_line_ = "ERROR: " + last_response_.error_message;
        page_txt     = "Failed to load: " + url + "\n\nError: " + last_response_.error_message;
        links_.clear(); media_.clear(); forms_.clear();
        return;
    }

    last_html_    = last_response_.body;
    last_headers_ = last_response_.headers;
    page_txt      = htmlToText(last_response_.body);
    if (page_txt.size() > 200000) page_txt = page_txt.substr(0, 200000) + "\n...(truncated)";

    extractLinksFromHtml(last_html_, url);
    extractMediaFromHtml(last_html_, url);
    extractFormFromHtml(last_html_, url);

    std::ostringstream oss;
    oss << "HTTP " << last_response_.status_code
        << " | " << last_response_.body.size() << " bytes"
        << " | " << last_load_time_ms_ << "ms"
        << " | " << links_.size() << " link(s)"
        << " | " << forms_.size() << " form(s)";
    status_line_ = oss.str();
    if (history_.empty() || history_[history_index_] != url) {
        if (history_index_ < (int)history_.size() - 1)
            history_.erase(history_.begin() + history_index_ + 1, history_.end());
        history_.push_back(url);
        history_index_ = (int)history_.size() - 1;
    }
    url_input_ = url;
    }
    void tuiApp::submitForm(const formItem& form) {
        std::string query;
        for (const auto& fi : form.inputs) {
            if (fi.type == "submit" || (fi.type == "checkbox" && !fi.checked)) continue;
            if (!query.empty()) query += "&";
            query += url_encode(fi.name) + "=" + url_encode(fi.value);
        }
        if (form.method == "GET") {
            std::string target = form.action + (form.action.find('?') == std::string::npos ? "?" : "&") + query;
            Navigate(target);
        } else {
            std::map<std::string, std::string> hdrs;
            hdrs["Content-Type"] = "application/x-www-form-urlencoded";
            hdrs["User-Agent"]   = settings_.user_agent;
            auto start = std::chrono::steady_clock::now();
            auto resp = http_client_->post(form.action, query, hdrs);
            auto end = std::chrono::steady_clock::now();
            long load_time = std::chrono::duration_cast<std::chrono::milliseconds>(end - start).count();
            std::lock_guard<std::mutex> lock(state_mutex_);
            is_loading = false;
            last_response_ = resp;
            last_load_time_ms_ = load_time;
            if (!last_response_.success) {
                status_line_ = "POST ERROR: " + last_response_.error_message;
                page_txt     = "Form submission failed.\n\nError: " + last_response_.error_message;
            } else {
                current_url_  = form.action;
                last_html_    = last_response_.body;
                last_headers_ = last_response_.headers;
                page_txt      = htmlToText(last_response_.body);
                if (page_txt.size() > 200000) page_txt = page_txt.substr(0, 200000) + "\n...(truncated)";
                extractLinksFromHtml(last_html_, form.action);
                extractMediaFromHtml(last_html_, form.action);
                extractFormFromHtml(last_html_, form.action);
                std::ostringstream oss;
                oss << "POST " << last_response_.status_code << " | " << last_response_.body.size() << " bytes | " << last_load_time_ms_ << "ms";
                status_line_ = oss.str();
                if (history_.empty() || history_[history_index_] != form.action) {
                    if (history_index_ < (int)history_.size() - 1)
                        history_.erase(history_.begin() + history_index_ + 1, history_.end());
                    history_.push_back(form.action);
                    history_index_ = (int)history_.size() - 1;
                }
                url_input_ = form.action;
            }
        }
    }
    void tuiApp::addBookmark() {
        std::lock_guard<std::mutex> lock(state_mutex_);
        if (current_url_.empty()) { status_line_ = "No page to bookmark."; return; }
        bookmarks_.push_back(current_url_);
        status_line_ = "Bookmarked: " + current_url_;
    }void tuiApp::openMedia(const mediaItem& item) {
        std::lock_guard<std::mutex> lock(state_mutex_);
        if (settings_.media_mode == MediaMode::PrintUrl) {
            status_line_ = "[" + item.type + "] " + item.url;
            return;
        }
        const std::string& cmd = (item.type == "video") ? settings_.video_player_cmd : (item.type == "audio") ? settings_.audio_player_cmd : settings_.image_viewer_cmd;
        std::string full = cmd + " \"" + item.url + "\"";
        status_line_ = "Launching: " + full;
        std::thread([full]() { std::system(full.c_str()); }).detach();
    }void tuiApp::analyzeHeaders() {
        std::lock_guard<std::mutex> lock(state_mutex_);
        if (last_headers_.empty()) {
            security_panel_text_ = "No headers available. Load a page first.";
            return;
        }
        auto report = gar::security::HeaderAnalzer::analyze(last_headers_);
        std::ostringstream oss;
        oss << "Security Score: " << report.score << "/100\n\nIssues:\n";
        for (const auto& issue : report.issues) oss << "  [" << issue.severity << "] " << issue.name << ": " << issue.message << "\n";
        if (report.issues.empty()) oss << "  No issues found!\n";
        security_panel_text_ = oss.str();
    }
    void tuiApp::inspectTLS(const std::string& host) {
        gar::security::TLSInspector inspector;
        auto report = inspector.inspect(host);
        std::ostringstream oss;
        if (!report.success) oss << "TLS Failed:\n" << report.error;
        else oss << "Subject:   " << report.subject   << "\nIssuer:    " << report.issuer << "\nValid:     " << report.not_before << " -> " << report.not_after << "\nDays left: " << report.days_left  << "\n";
        std::lock_guard<std::mutex> lock(state_mutex_);
        security_panel_text_ = oss.str();
    }
    void tuiApp::requestNewnym() {
        gar::anonymity::TorControl ctrl;
        std::string msg;
        if (!ctrl.connect()) msg = "NEWNYM failed: " + ctrl.last_error();
        else if (!ctrl.authenticate()) { ctrl.disconnect(); msg = "NEWNYM auth failed: " + ctrl.last_error(); }
        else { msg = ctrl.signal_newnym() ? "NEWNYM sent. Wait 10s for new circuit." : "NEWNYM signal failed: " + ctrl.last_error(); ctrl.disconnect(); }
        std::lock_guard<std::mutex> lock(state_mutex_);
        status_line_ = msg;
    }
    void tuiApp::flushDNS() {
        static gar::core::dnsResolver resolver;
        resolver.clear_cache();
        std::lock_guard<std::mutex> lock(state_mutex_);
        status_line_ = "DNS cache cleared.";
    }void tuiApp::rotateFingerprint() {
        static gar::anonymity::Fingerprint fp;
        fp.rotate();
        http_client_->refreshFingerprint();
        std::lock_guard<std::mutex> lock(state_mutex_);
        status_line_ = "Fingerprint rotated.";
    }void tuiApp::replayLastRequest() {
        std::string target;
        {
            std::lock_guard<std::mutex> lock(state_mutex_);
            target = last_capture_url_;
        }
        if (target.empty()) { std::lock_guard<std::mutex> lock(state_mutex_); status_line_ = "No request to replay."; return; }

        auto resp = http_client_->get(target); // Network bound, runs safely unlocked!

        std::lock_guard<std::mutex> lock(state_mutex_);
        std::ostringstream oss;
        oss << "Replay: HTTP " << resp.status_code << " | " << resp.body.size() << " bytes";
        status_line_ = oss.str();
    }void tuiApp::saveSettingNow() {
        std::lock_guard<std::mutex> lock(state_mutex_);
        SaveSettings(settings_, settings_path_);
        status_line_ = "Settings saved to " + settings_path_;
    }
    tuiApp::tuiApp(Settings settings)
        : tor_manager_(std::make_unique<gar::anonymity::TorManager>()),
          http_client_(std::make_unique<gar::core::HttpClient>(settings.host, settings.port)),
          settings_(std::move(settings)) {
        url_input_           = "https://";
        status_line_         = "Welcome to G.A.R.  —  Press F1 for help.";
        page_txt             = "Enter a URL in the bar above and press ENTER to navigate.";
        security_panel_text_ = "Use 'Analyze Headers' or 'Inspect TLS' after loading a page.";
    }

    tuiApp::~tuiApp() { stopTor(); }

    void tuiApp::startTorAsync() {
        t_thread_ = std::thread([this] {
            if (!tor_manager_->startTor()) {
                std::lock_guard<std::mutex> lock(state_mutex_);
                t_start_failed_ = true;
                t_start_error_  = tor_manager_->getLastError();
                status_line_  = "Tor failed: " + t_start_error_;
                return;
            }
            if (!tor_manager_->waitForTorReady(60)) {
                std::lock_guard<std::mutex> lock(state_mutex_);
                t_start_failed_ = true;
                t_start_error_  = "Tor not ready after 60s.";
                status_line_      = t_start_error_;
                return;
            }
            std::lock_guard<std::mutex> lock(state_mutex_);
            t_ready_   = true;
            status_line_ = "Tor is ACTIVE. You are anonymous.";
        });
    }

    void tuiApp::stopTor() {
        if (t_thread_.joinable()) t_thread_.join();
        if (tor_manager_) tor_manager_->stopTor();
    }
    int tuiApp::Run() {
        startTorAsync();
        auto screen = ScreenInteractive::Fullscreen();

        // Async Request Helper
        auto trigger_navigate = [&](std::string url) {
            if (url.empty()) return;
            {
                std::lock_guard<std::mutex> lock(state_mutex_);
                is_loading = true;
                status_line_ = "Loading " + url + " in background...";
            }
            std::thread([this, url, &screen]() {
                Navigate(url);
                screen.PostEvent(Event::Custom);
            }).detach();
        };


        int text_scroll_y     = 0;
        int tab_selected      = 0;
        int link_selected     = 0;
        int media_selected    = 0;
        int bookmark_selected = 0;
        int history_selected  = 0;
        int form_field_sel    = 0;
        int active_form_idx   = 0;

        std::vector<std::string> tab_names = {"Links", "Forms", "Media", "Bookmarks", "History"};
        std::vector<std::string> link_labels;
        std::vector<std::string> media_labels;
        std::vector<std::string> bookmark_labels;
        std::vector<std::string> history_labels;
        std::vector<std::string> form_field_labels;

        std::string edit_buffer;
        bool show_form_editor  = false;
        int  editing_field_idx = -1;

        // Components
        auto url_input_comp   = Input(&url_input_, "Enter URL and press Enter...");
        auto field_edit_input = Input(&edit_buffer, "Type here...");

        auto btn_confirm_edit = Button("  Confirm  ", [&] {
            std::lock_guard<std::mutex> lock(state_mutex_);
            if (active_form_idx < (int)forms_.size() && editing_field_idx >= 0 && editing_field_idx < (int)forms_[active_form_idx].inputs.size()) {
                forms_[active_form_idx].inputs[editing_field_idx].value = edit_buffer;
            }
            show_form_editor  = false;
            editing_field_idx = -1;
            edit_buffer.clear();
        });

        auto btn_cancel_edit = Button("  Cancel  ", [&] {
            std::lock_guard<std::mutex> lock(state_mutex_);
            show_form_editor  = false;
            editing_field_idx = -1;
            edit_buffer.clear();
        });

        auto btn_submit_form = Button("[ Submit Form ]", [&] {
            formItem form_copy;
            {
                std::lock_guard<std::mutex> lock(state_mutex_);
                if (active_form_idx >= 0 && active_form_idx < (int)forms_.size()) {
                    form_copy = forms_[active_form_idx];
                    is_loading = true;
                    status_line_ = "Submitting form in background...";
                    active_form_idx = 0;
                    form_field_sel  = 0;
                } else return;
            }
            std::thread([this, form_copy, &screen]() {
                submitForm(form_copy);
                screen.PostEvent(Event::Custom);
            }).detach();
        });

        auto btn_edit_field = Button("[ Edit Field ]", [&] {
            std::lock_guard<std::mutex> lock(state_mutex_);
            if (active_form_idx >= (int)forms_.size()) return;
            int visible = 0;
            for (int i = 0; i < (int)forms_[active_form_idx].inputs.size(); ++i) {
                if (forms_[active_form_idx].inputs[i].type == "hidden") continue;
                if (visible == form_field_sel) {
                    editing_field_idx = i;
                    edit_buffer       = forms_[active_form_idx].inputs[i].value;
                    show_form_editor  = true;
                    break;
                }
                ++visible;
            }
        });

        auto tab_toggle      = Toggle(&tab_names, &tab_selected);
        auto link_menu       = Menu(&link_labels,       &link_selected);
        auto media_menu      = Menu(&media_labels,      &media_selected);
        auto bookmark_menu   = Menu(&bookmark_labels,   &bookmark_selected);
        auto history_menu    = Menu(&history_labels,    &history_selected);
        auto form_field_menu = Menu(&form_field_labels, &form_field_sel);

        auto btn_go       = Button("  GO  ", [&] {
            trigger_navigate(normalizeUrl(url_input_));
            link_selected = media_selected = form_field_sel = active_form_idx = 0;
        });

        auto btn_back     = Button(" Back ",   [&] {
            std::string target;
            {
                std::lock_guard<std::mutex> lock(state_mutex_);
                if (history_index_ <= 0) { status_line_ = "No back history."; return; }
                history_index_--;
                target = history_[history_index_];
            }
            trigger_navigate(target);
        });

        auto btn_forward  = Button("  Fwd  ",  [&] {
            std::string target;
            {
                std::lock_guard<std::mutex> lock(state_mutex_);
                if (history_index_ < 0 || history_index_ >= (int)history_.size() - 1) {
                    status_line_ = "No forward history."; return;
                }
                history_index_++;
                target = history_[history_index_];
            }
            trigger_navigate(target);
        });

        auto btn_bookmark = Button(" ★ BM ",   [&] { addBookmark(); });

        auto btn_open_link = Button("Open Link", [&] {
            std::string target;
            {
                std::lock_guard<std::mutex> lock(state_mutex_);
                if (link_selected >= 0 && link_selected < (int)links_.size()) target = links_[link_selected].url;
            }
            trigger_navigate(target);
        });

        auto btn_open_media = Button("Open Media", [&] {
            mediaItem target;
            {
                std::lock_guard<std::mutex> lock(state_mutex_);
                if (media_selected >= 0 && media_selected < (int)media_.size()) target = media_[media_selected];
            }
            openMedia(target);
        });

        auto btn_open_bm = Button("Open BM", [&] {
            std::string target;
            {
                std::lock_guard<std::mutex> lock(state_mutex_);
                if (bookmark_selected >= 0 && bookmark_selected < (int)bookmarks_.size()) target = bookmarks_[bookmark_selected];
            }
            trigger_navigate(target);
        });

        auto btn_open_hist = Button("Open", [&] {
            std::string target;
            {
                std::lock_guard<std::mutex> lock(state_mutex_);
                if (history_selected >= 0 && history_selected < (int)history_.size()) {
                    target = history_[history_selected];
                    history_index_ = history_selected;
                }
            }
            trigger_navigate(target);
        });

        auto btn_newnym    = Button("New Circuit", [&] {
            std::thread([this, &screen]() { requestNewnym(); screen.PostEvent(Event::Custom); }).detach();
        });
        auto btn_check_ip  = Button("Check IP",    [&] { trigger_navigate("https://check.torproject.org/"); });
        auto btn_flush_dns = Button("Flush DNS",   [&] { flushDNS(); });
        auto btn_rotate_fp = Button("Rotate FP",   [&] { rotateFingerprint(); });
        auto btn_replay    = Button("Replay",      [&] {
            std::thread([this, &screen]() { replayLastRequest(); screen.PostEvent(Event::Custom); }).detach();
        });

        std::vector<std::string> media_mode_entries = {"Print URL", "External player"};
        int media_mode_idx = (settings_.media_mode == MediaMode::ExternalPlayer) ? 1 : 0;
        auto media_mode_toggle = Radiobox(&media_mode_entries, &media_mode_idx);
        auto video_cmd_input   = Input(&settings_.video_player_cmd, "e.g. mpv");
        auto image_cmd_input   = Input(&settings_.image_viewer_cmd, "e.g. imv");
        auto audio_cmd_input   = Input(&settings_.audio_player_cmd, "e.g. mpv");
        std::vector<std::string> tor_entries = {"Tor ENABLED", "Tor DISABLED"};
        int tor_idx = settings_.use_proxy ? 0 : 1;
        auto tor_toggle = Radiobox(&tor_entries, &tor_idx);

        auto btn_save_settings = Button("Save Settings", [&] {
            std::lock_guard<std::mutex> lock(state_mutex_);
            settings_.media_mode = (media_mode_idx == 1) ? MediaMode::ExternalPlayer : MediaMode::PrintUrl;
            settings_.use_proxy  = (tor_idx == 0);
            saveSettingNow();
        });
        auto btn_close_settings = Button("Close (F2)", [&] { show_settings_ = false; });

        auto btn_analyze_hdrs   = Button("Analyze Headers", [&] { analyzeHeaders(); });
        auto btn_inspect_tls    = Button("Inspect TLS", [&] {
            std::string host;
            {
                std::lock_guard<std::mutex> lock(state_mutex_);
                if (current_url_.empty()) { security_panel_text_ = "Load a page first."; return; }
                size_t s = current_url_.find("://");
                if (s == std::string::npos) { security_panel_text_ = "Invalid URL."; return; }
                size_t slash = current_url_.find('/', s + 3);
                host = current_url_.substr(s + 3, slash == std::string::npos ? std::string::npos : slash - (s + 3));
                security_panel_text_ = "Inspecting TLS in background...";
            }
            std::thread([this, host, &screen]() { inspectTLS(host); screen.PostEvent(Event::Custom); }).detach();
        });
        auto btn_close_security = Button("Close (F3)", [&] { show_security_ = false; });


        auto tab_container = Container::Tab({
            Container::Vertical({ link_menu, btn_open_link }),
            Container::Vertical({ form_field_menu, btn_edit_field, btn_submit_form }),
            Container::Vertical({ media_menu, btn_open_media }),
            Container::Vertical({ bookmark_menu, btn_open_bm }),
            Container::Vertical({ history_menu, btn_open_hist })
        }, &tab_selected);


        auto root = Container::Vertical({
            url_input_comp, btn_go, btn_back, btn_forward, btn_bookmark,
            tab_toggle,
            tab_container,
            btn_newnym, btn_check_ip, btn_flush_dns, btn_rotate_fp, btn_replay,
            media_mode_toggle, video_cmd_input, image_cmd_input, audio_cmd_input,
            tor_toggle, btn_save_settings, btn_close_settings,
            btn_analyze_hdrs, btn_inspect_tls, btn_close_security,
            field_edit_input, btn_confirm_edit, btn_cancel_edit,
        });

        auto renderer = Renderer(root, [&] {
        std::lock_guard<std::mutex> lock(state_mutex_);

        auto nav_bar = hbox({
            text(" URL: ") | bold,
            url_input_comp->Render() | flex,
            btn_go->Render(),
            text(" | "),
            btn_back->Render(),
            btn_forward->Render(),
            btn_bookmark->Render()
        }) | border;


        Element side_content;
        if (tab_selected == 0) { // Links
            link_labels.clear();
            for (const auto& l : links_) link_labels.push_back(l.label.empty() ? l.url : l.label);
            side_content = vbox({
                text("Links (" + std::to_string(links_.size()) + ")") | bold, separator(),
                link_menu->Render() | vscroll_indicator | yframe | flex,
                separator(), btn_open_link->Render()
            });
        } else if (tab_selected == 1) { // Forms
            form_field_labels.clear();
            if (active_form_idx >= 0 && active_form_idx < (int)forms_.size()) {
                const auto& form = forms_[active_form_idx];
                for (const auto& fi : form.inputs) {
                    if (fi.type == "hidden") continue;
                    form_field_labels.push_back("[" + fi.type + "] " + fi.name + " = " + fi.value);
                }
            }
            side_content = vbox({
                text("Forms (" + std::to_string(forms_.size()) + ")") | bold, separator(),
                text(forms_.empty() ? "" : "Action: " + forms_[active_form_idx].action),
                text(forms_.empty() ? "" : "Method: " + forms_[active_form_idx].method), separator(),
                form_field_menu->Render() | vscroll_indicator | yframe | flex,
                separator(), hbox({ btn_edit_field->Render(), btn_submit_form->Render() })
            });
        } else if (tab_selected == 2) { // Media
            media_labels.clear();
            for (const auto& m : media_) media_labels.push_back("[" + m.type + "] " + m.url);
            side_content = vbox({
                text("Media (" + std::to_string(media_.size()) + ")") | bold, separator(),
                media_menu->Render() | vscroll_indicator | yframe | flex,
                separator(), btn_open_media->Render()
            });
        } else if (tab_selected == 3) { // Bookmarks
            bookmark_labels = bookmarks_;
            side_content = vbox({
                text("Bookmarks") | bold, separator(),
                bookmark_menu->Render() | vscroll_indicator | yframe | flex,
                separator(), btn_open_bm->Render()
            });
        } else if (tab_selected == 4) { // History
            history_labels = history_;
            side_content = vbox({
                text("History") | bold, separator(),
                history_menu->Render() | vscroll_indicator | yframe | flex,
                separator(), btn_open_hist->Render()
            });
        }

        auto side_panel = vbox({ tab_toggle->Render(), separator(), side_content | flex }) | border | size(WIDTH, LESS_THAN, 40);

        // 3. Build the main text pane (WITH custom Word-Wrapping and Scrolling!)
        std::string title = current_url_.empty() ? "No page loaded" : current_url_;

        std::vector<std::string> lines;
        std::string current_line;
        std::string text_to_split = is_loading ? "Loading, please wait..." : page_txt;
        for (char c : text_to_split) {
            if (c == '\n') { lines.push_back(current_line); current_line.clear(); }
            else current_line += c;
        }
        if (!current_line.empty()) lines.push_back(current_line);

        Elements text_elements;
        int index = 0;
        for (const auto& line : lines) {
            if (index == text_scroll_y) text_elements.push_back(paragraph(line) | focus);
            else text_elements.push_back(paragraph(line));
            index++;
        }

        auto main_pane = vbox({
            text(" " + title) | bold | color(Color::Cyan),
            separator(),
            vbox(std::move(text_elements)) | vscroll_indicator | yframe | flex
        }) | border | flex;

        auto main_layout = hbox({ side_panel, main_pane }) | flex;

        // 4. Build Modal Overlays
        Element final_view = vbox({ nav_bar, main_layout, text(status_line_) | border });

        if (show_form_editor) {
            auto editor_box = vbox({
                text("Edit Field Value") | bold | center, separator(),
                field_edit_input->Render(), separator(),
                hbox({ btn_confirm_edit->Render(), btn_cancel_edit->Render() }) | center
            }) | border | clear_under | center;
            final_view = dbox({ final_view, editor_box });
        } else if (show_settings_) {
            auto settings_box = vbox({
                text("Settings (F2)") | bold | center, separator(),
                text("Tor Configuration:") | bold, tor_toggle->Render(), separator(),
                text("Media Handling:") | bold, media_mode_toggle->Render(),
                hbox({ text("Video Player: "), video_cmd_input->Render() }),
                hbox({ text("Image Viewer: "), image_cmd_input->Render() }),
                hbox({ text("Audio Player: "), audio_cmd_input->Render() }),
                separator(), hbox({ btn_save_settings->Render(), btn_close_settings->Render() }) | center
            }) | border | clear_under | center;
            final_view = dbox({ final_view, settings_box });
        } else if (show_security_) {
            auto security_box = vbox({
                text("Security & Anonymity (F3)") | bold | center, separator(),
                hbox({ btn_newnym->Render(), btn_check_ip->Render(), btn_flush_dns->Render(), btn_rotate_fp->Render(), btn_replay->Render() }) | center,
                separator(),
                hbox({ btn_analyze_hdrs->Render(), btn_inspect_tls->Render() }) | center,
                separator(),
                paragraph(security_panel_text_) | vscroll_indicator | yframe | flex,
                separator(), btn_close_security->Render() | center
            }) | size(WIDTH, GREATER_THAN, 60) | size(HEIGHT, GREATER_THAN, 20) | border | clear_under | center;
            final_view = dbox({ final_view, security_box });
        }

        return final_view;
    });

    // 5. Global Event Handler for Hotkeys and Scrolling!
    auto final_component = CatchEvent(renderer, [&](Event e) {
        if (e == Event::F1) {
            std::lock_guard<std::mutex> lock(state_mutex_);
            status_line_ = "Shortcuts: F2=Settings, F3=Security, F5=Reload, PageUp/Down=Scroll";
            return true;
        }
        if (e == Event::F2) { std::lock_guard<std::mutex> lock(state_mutex_); show_settings_ = !show_settings_; return true; }
        if (e == Event::F3) { std::lock_guard<std::mutex> lock(state_mutex_); show_security_ = !show_security_; return true; }
        if (e == Event::F5) {
            std::string target;
            { std::lock_guard<std::mutex> lock(state_mutex_); target = current_url_; }
            trigger_navigate(target);
            return true;
        }

        // Universal Page Scrolling!
        if (e == Event::PageDown) { text_scroll_y += 5; return true; }
        if (e == Event::PageUp)   { text_scroll_y -= 5; if (text_scroll_y < 0) text_scroll_y = 0; return true; }

        return false;
    });

    screen.Loop(final_component);
    return 0;
    }
}
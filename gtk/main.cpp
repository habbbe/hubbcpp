#include <string>
#include <string_view>
#include <variant>
#include <thread>
#include <fcntl.h>
#include <fstream>
#include <iostream>
#include <cstdio>
#include <cstdlib>
#include <gtkmm.h>
#include <chrono>
#include <gdk/gdkkeysyms.h>
#include "parser.h"

int main(int argc, char *argv[])
{
    const std::string conf = argc > 1 ? argv[1] : default_config();
    auto entries = parse(conf);
    argc = 1;
    auto app = Gtk::Application::create(argc, argv, "");
    Gtk::Window window;
    window.set_default_size(400, 800);
    window.set_border_width(10);

    window.signal_key_press_event().connect([=](GdkEventKey* key) {
        auto modifiers = gtk_accelerator_get_default_mod_mask();
        if (key->keyval == GDK_KEY_Escape ||
                key->keyval == GDK_KEY_w && (key->state & modifiers) == GDK_CONTROL_MASK) {
            app->quit();
        }
        return true;
    });

    Gtk::ScrolledWindow scrollWindow{};
    Gtk::VBox box(Gtk::ORIENTATION_VERTICAL);
    box.set_homogeneous(false);
    scrollWindow.add(box);
    window.add(scrollWindow);

    const auto startTime = std::chrono::steady_clock::now();
    for (const entry& e_variant : entries) {
        std::visit([&](auto&& entry){
            using T = std::decay_t<decltype(entry)>;
            if constexpr (std::is_same_v<T, action>) {
                auto hbox = Gtk::make_managed<Gtk::HBox>();
                auto keep = Gtk::make_managed<Gtk::Button>("  ", false);
                auto run = [c = entry.command.c_str()] {std::thread([c]{std::system(c);}).detach();};
                keep->signal_clicked().connect(run);
                auto close = Gtk::make_managed<Gtk::Button>(entry.name, false);
                close->signal_clicked().connect([run,&app]{ run(); app->quit(); });
                close->set_tooltip_text(entry.command);
                hbox->pack_start(*keep, Gtk::PACK_SHRINK);
                hbox->pack_start(*close, Gtk::PACK_EXPAND_WIDGET);
                box.pack_start(*hbox, Gtk::PACK_SHRINK);
            } else if constexpr (std::is_same_v<T, info>) {
                auto label = Gtk::make_managed<Gtk::Label>(entry.name);
                label->set_tooltip_text(entry.command);
                box.pack_start(*label, Gtk::PACK_SHRINK);
                if (entry.command.empty()) { label->set_text(entry.name); return; }
                std::thread([label, startTime, &entry]{
                    std::string result = entry.name;
                    auto updateAt = startTime;
                    while (true) {
                        updateAt += std::chrono::milliseconds(entry.update_rate);
                        result.resize(entry.name.size());
                        {
                            std::unique_ptr<FILE, decltype(&pclose)> file(popen(entry.command.c_str(), "r"), pclose);
                            int d = fileno(file.get());
                            fcntl(d, F_SETFL, O_NONBLOCK);
                            char buffer[128];

                            while (true) {
                                auto r = read(d, buffer, 127);
                                if (r == -1 && errno == EAGAIN) {
                                    std::this_thread::sleep_for(std::chrono::milliseconds(50));
                                    continue;
                                } else if (r > 0) {
                                    result.append(buffer, buffer + r);
                                } else {
                                    break;
                                }
                            }
                        }
                        if (result.size() > entry.name.size()) result.pop_back(); 
                        Glib::signal_idle().connect_once([&] {
                            label->set_text(result);
                        });
                        std::this_thread::sleep_until(updateAt);
                    }
                }
                ).detach();
            } else if constexpr (std::is_same_v<T, separator>) {
                auto sep = Gtk::make_managed<Gtk::Separator>();
                sep->set_size_request(-1, 2);
                sep->set_margin_top(3);
                sep->set_margin_bottom(3);
                box.pack_start(*sep, Gtk::PACK_SHRINK);
            } else if constexpr (std::is_same_v<T, space>) {
                auto space = Gtk::make_managed<Gtk::Label>(" ");
                box.pack_start(*space, Gtk::PACK_SHRINK);
            } else if constexpr (std::is_same_v<T, syntax_error>) {
                auto error = Gtk::make_managed<Gtk::Label>("Syntax error: " + entry.error);
                box.pack_start(*error, Gtk::PACK_SHRINK);
            }
        }, e_variant);
    }

    window.show_all_children();
    return app->run(window);
}

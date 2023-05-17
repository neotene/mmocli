#include <iostream>

#include "neo/term/attributes.hpp"
#include "neo/term/field.hpp"
#include "neo/term/frame.hpp"
#include "neo/term/label.hpp"
#include "neo/ui/attributes.hpp"
#include <neo/term.hpp>
#include <string>

using namespace neo;

int
main(int, char **, char **)
{
    try
    {
        ui::terminal::context ctx;

        ui::terminal::frame welcome_frame(ctx, ui::terminal::attributes(0, 0));

        size_t const width = ctx.width();
        size_t const height = ctx.height();

        ui::terminal::shape box(ctx, ui::terminal::attributes(0, 0, width, height), &welcome_frame);

        ui::terminal::shape modale(
            ctx, ui::terminal::attributes(0, 0, 60, 10, &box.get_attributes(), ui::anchor::center, ui::anchor::center),
            &welcome_frame);

        ui::terminal::label email_label(
            ctx,
            ui::terminal::attributes(2, 1, 5, 0, &modale.get_attributes(), ui::anchor::top_left, ui::anchor::top_left),
            L"email", &welcome_frame);

        int const field_size = 45;

        ui::terminal::field email_field(ctx,
                                        ui::terminal::attributes(-1, 1, field_size, 0, &modale.get_attributes(),
                                                                 ui::anchor::top_right, ui::anchor::top_right),
                                        field_size, false, &welcome_frame);

        std::wstring passwd_str = L"password";

        ui::terminal::label passwd_label(ctx,
                                         ui::terminal::attributes(0, 1, passwd_str.size(), 0,
                                                                  &email_label.get_attributes(), ui::anchor::top_left,
                                                                  ui::anchor::bottom_left),
                                         passwd_str, &welcome_frame);

        ui::terminal::field passwd_field(ctx,
                                         ui::terminal::attributes(0, 1, field_size, 0, &email_field.get_attributes(),
                                                                  ui::anchor::top_right, ui::anchor::bottom_right),
                                         field_size, true, &welcome_frame);

        std::wstring login_str = L"login";
        ui::terminal::button login_button(ctx,
                                          ui::terminal::attributes(6, -1, login_str.size(), 3, &modale.get_attributes(),
                                                                   ui::anchor::bottom_left, ui::anchor::bottom_left),
                                          login_str, &welcome_frame);

        std::wstring register_str = L"register";
        ui::terminal::button register_button(
            ctx,
            ui::terminal::attributes(-6, -1, register_str.size(), 3, &modale.get_attributes(), ui::anchor::bottom_right,
                                     ui::anchor::bottom_right),
            register_str, &welcome_frame);

        ctx.push_window(welcome_frame);

        ctx.run();
    } catch (std::exception const &e)
    {
        std::cerr << "Exception: " << e.what() << std::endl;
    }
}

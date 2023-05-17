#include <iostream>

#include "neo/term/attributes.hpp"
#include "neo/term/frame.hpp"
#include "neo/ui/attributes.hpp"
#include <neo/term.hpp>

int main(int, char **, char **) {
  try {
    neo::ui::terminal::context ctx;

    neo::ui::terminal::frame welcome_frame(ctx,
                                           neo::ui::terminal::attributes(0, 0));

    size_t const width = ctx.width();
    size_t const height = ctx.height();

    neo::ui::terminal::shape box(
        ctx, neo::ui::terminal::attributes(
                 0, 0, width, height, &welcome_frame.get_position(),
                 neo::ui::anchor::center, neo::ui::anchor::center));

    neo::ui::terminal::button button(ctx, neo::ui::terminal::attributes(0, 0),
                                     L"Hello", &welcome_frame);

    ctx.push_window(welcome_frame);

    ctx.run();
  } catch (std::exception const &e) {
    std::cerr << "Exception: " << e.what() << std::endl;
  }
}

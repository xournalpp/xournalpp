
add_definitions(${GTK2_CFLAGS})

set(poppler_glib_demo_SRCS
  main.c
  find.c
  fonts.c
  forms.c
  info.cc
  images.c
  links.c
  outline.c
  page.c
  render.c
  text.c
  transitions.c
  utils.c
  annots.c
  attachments.c
  layers.c
)
poppler_add_test(poppler-glib-demo BUILD_GTK_TESTS ${poppler_glib_demo_SRCS})
target_link_libraries(poppler-glib-demo poppler-glib ${GTK2_LIBRARIES})

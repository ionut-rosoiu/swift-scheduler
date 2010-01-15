include_directories (${CMAKE_SOURCE_DIR}/include/) 

add_library (swift-workstealing STATIC
  swift_allocator.c
  swift_backoff.c
  swift_context.c
  swift_deque.c
  swift_dfg.c
  swift_scheduler.c
  swift_thread.c
)
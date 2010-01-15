include_directories (${CMAKE_SOURCE_DIR}/include/) 

link_directories (${CMAKE_BINARY_DIR}/src/ 
                  /usr/lib) 

add_executable (test_deque                     test_deque.c)
target_link_libraries(test_deque               swift-workstealing pthread)

# Fibonacci

add_executable (fibo_serial                    fibo_serial.c)
set_target_properties(fibo_serial              PROPERTIES COMPILE_FLAGS "-fno-omit-frame-pointer")

add_executable (fibo_stack_2spawns             fibo_stack_2spawns.c)
target_link_libraries(fibo_stack_2spawns       swift-workstealing pthread)

add_executable(fibo_omp                        fibo_omp.c)
set_target_properties(fibo_omp                 PROPERTIES COMPILE_FLAGS "-fopenmp" LINK_FLAGS "-fopenmp")

# Quicksort

add_executable(qsort_gen                       qsort_gen.c qsort_common.c)

add_executable(qsort_serial                    qsort_serial.c qsort_common.c)
set_target_properties(qsort_serial             PROPERTIES COMPILE_FLAGS "-fno-omit-frame-pointer")

add_executable(qsort_ws                        qsort_ws.c qsort_common.c)
target_link_libraries(qsort_ws                 swift-workstealing pthread)

add_executable(qsort_omp_par                   qsort_omp_par.c qsort_common.c)
set_target_properties(qsort_omp_par            PROPERTIES COMPILE_FLAGS "-fopenmp" LINK_FLAGS "-fopenmp")

add_executable(qsort_omp_task                  qsort_omp_task.c qsort_common.c)
set_target_properties(qsort_omp_task           PROPERTIES COMPILE_FLAGS "-fopenmp" LINK_FLAGS "-fopenmp")



#add_executable (fibo                          fibo.c)
#target_link_libraries(fibo                    swift-workstealing pthread)

#add_executable (fibo_2spawn                   fibo_2spawn.c)
#target_link_libraries(fibo_2spawn             swift-workstealing pthread)

#add_executable (fibo_2spawns_no_ksync         fibo_2spawns_no_ksync.c)
#target_link_libraries(fibo_2spawns_no_ksync   swift-workstealing pthread)
#import "template.typ"

#show: body => template.conf(
    title: [Большое домашнее задание 3],
    subtitle: [Отчет],
    author: "Чубий Савва Андреевич, БПИ233",
    body
)

= Изменения кодовой базы

== Параллелизм

- Написан `ThreadPool` (см. файлы `ThreadPool.hpp` и `ThreadPool.cpp`)
- Распараллелены все обходы матрицы.

    Это сделано в два этапа:

    - Написан метод `void Fluid::forall(const F& f)` (см. файл `Fluid.hpp`),
        который принимает функцию вида `void f(size_t x, size_t y)` и выполняет
        её в нескольких потоках для каждой клетки вида
        ${(x, y) in NN^2 mid(|) 0 <= x < n and 0 <= y < m}$.

        Причем, обход матрицы сделан в таком порядке, что, если функция `f`
        модифицирует только клетку $(x, y)$ и её соседей $(x plus.minus 1, y
        plus.minus 1)$, то два потока никогда не будут одновременно оперировать
        над одной и той же клеткой, а значит дополнительные средства
        синхронизации (такие как `std::mutex` или `std::atomic`) не требуются,
        что дополнительно ускоряет алгоритм.

    - Метод `forall` использован в соответствующих местах программы

== Не-мультипоточные оптимизации

- Использована статическая реализация Матрицы (из ДЗ 2)
- Реализованы "правильные" сравнения вещественных чисел, а именно, сравнения
    вида `lhs == rhs` заменены на `boolean::eq(lhs, rhs)`, где функция `eq`
    объявлена следующим образом:

    ```cpp
    namespace boolean {
        template<typename T, typename U>
        bool eq(T lhs, U rhs) {
            return is_zero(lhs - rhs);
        }

        template<typename T>
        bool is_zero(T v) {
            return std::abs(v) < eps;
        }
    }
    ```
- Ускорен поиск по массиву `deltas`. Так как значения массива `deltas` заранее
    известны, и их количество крайне ограничено, то линейный поиск был заменен
    на несколько `if`-ов.

== Другое (не оптимизации)

- Добавлен аргумент командной строки `--threads=N`

= Замеры

#let benchmarks = (
    (
        threads: 1,
        opt: false,
        commit: "4e93e5d",
        time: 31.4,
    ),
    (
        threads: 8,
        opt: false,
        commit: "4e93e5d",
        time: 13.2,
    ),
    (
        threads: 1,
        opt: true,
        commit: "b97f64e",
        time: 15.8,
    ),
    (
        threads: 8,
        opt: true,
        commit: "b97f64e",
        time: 10.5,
    ),
)

#figure(template.benchmarks_table(benchmarks))

*Вывод*: как видно из замеров, и параллелизм, и не-мультипоточные оптимизации дают
значительное ускорение.

== Параметры

=== Параметры устройства

#figure(table(
    columns: 2,
    align: center,

    [CPU:], [Intel(R) Pentium(R) Gold G5400 CPU \@ 3.70GHz],
    [RAM:], [8 GB]
))

=== Тест

- Флаги компиляции:

    ```
    -DSIZES=S(36, 84),S(36, 85)
    -DTYPES=FIXED(32, 16)
    -g0
    -O2
    -DNDEBUG
    -std=gnu++20
    ```

- Аргументы командной строки:

    ```
    --p-type=FIXED(32, 16)
    --v-type=FIXED(32, 16)
    --v-flow-type=FIXED(32, 16)
    --ticks=400
    --threads=*количество тредов*
    --quiet=true
    data_heavy.in
    ```
- Используется тест из условия:

    #colbreak(weak: true) 

    ```
    // N, M
    36 84
    // Field
    ####################################################################################
    #                                                                                  #
    #                                                                                  #
    #                                                                                  #
    #                                                                                  #
    #                                                                                  #
    #                                       .........                                  #
    #..............#            #           .........                                  #
    #..............#            #           .........                                  #
    #..............#            #           .........                                  #
    #..............#            #                                                      #
    #..............#            #                                                      #
    #..............#            #                                                      #
    #..............#            #                                                      #
    #..............#............#                                                      #
    #..............#............#                                                      #
    #..............#............#                                                      #
    #..............#............#                                                      #
    #..............#............#                                                      #
    #..............#............#                                                      #
    #..............#............#                                                      #
    #..............#............#                                                      #
    #..............#............################                     #                 #
    #...........................#....................................#                 #
    #...........................#....................................#                 #
    #...........................#....................................#                 #
    ##################################################################                 #
    #                                                                                  #
    #                                                                                  #
    #                                                                                  #
    #                                                                                  #
    #                                                                                  #
    #                                                                                  #
    #                                                                                  #
    #                                                                                  #
    ####################################################################################
    // G
    0.1
    // Rho
      0.1
    . 1000
    ```
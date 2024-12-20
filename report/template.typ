#let _heading_builder(
    is_bold: true,
    is_smallcapsed: false,
    is_underlined: false,
    font_size: 16pt,
    line_len: 2cm,
    vspace: 0.5cm,
    body
) = {
    if is_smallcapsed {
        body = smallcaps(body)
    }

    if is_underlined {
        body = underline(body)
    }

    let body = text(
        size: font_size,
        weight: if is_bold { "bold" } else { "regular" },
        body
    )

    let line = line(
        length: line_len,
        stroke: if is_bold { 1pt } else { 0.5pt },
    )

    v(vspace, weak: true)
    grid(
        columns: (1fr, auto, 1fr),
        align: (horizon + right, horizon + center, horizon + left),
        column-gutter: 5pt,
        line,
        body,
        line,
    )
    v(vspace, weak: true)
}

#let conf(
    title: none,
    subtitle: none,
    author: none,
    body
) = {
    // Set metadata
    set document(
        title: [#title. #subtitle],
        author: author,
    )

    [
        #metadata((
            title: title,
            subtitle: subtitle,
        )) <document_info>
    ]

    // Set language and font size
    set text(lang: "ru", size: 11pt)
    set par(justify: true)

    // Title page
    align(center + horizon, [
        #text(30pt, weight: "bold", title)

        #text(25pt, weight: "bold", subtitle)

        #text(25pt, author)
    ])
    pagebreak(weak: true)

    // Page style
    set page(
        footer: align(center, {
            line(length: 100%, stroke: 0.5pt)
            context counter(page).display("1")
        }),
    )

    // Headings style
    show heading.where(level: 1): h => _heading_builder(
        is_bold: true,
        font_size: 16pt,
        line_len: 2cm,
        h.body
    )

    show heading.where(level: 2): h => _heading_builder(
        is_bold: false,
        is_smallcapsed: true,
        font_size: 14pt,
        line_len: 1cm,
        h.body
    )

    show heading.where(level: 3): h => _heading_builder(
        is_bold: false,
        is_smallcapsed: true,
        is_underlined: true,
        font_size: 12pt,
        line_len: 0cm,
        h.body
    )

    // Table style
    set table(align: left)

    // Figure style
    set figure.caption(position: top)

    // Body
    body
}

#let benchmarks_table(benchmarks) = {
    import "@preview/oxifmt:0.2.1": strfmt

    table(
        columns: 6,
        align: (x, y) => if y == 0 {
                center + horizon
            } else {
                center
            },
        
        table.header(
            [*№\ конфиг.*],
            [*Кол-во\ потоков*],
            [*Наличие\ не-мультипоточных\ оптимизаций*],
            [*Коммит*],
            [*Время\ (секунды)*],
            [
                *Ускорение*
                #footnote[Относительно первого замера]
            ]
        ),
        
        ..range(0, benchmarks.len()).map(i => {
            let num = i + 1

            let threads = benchmarks.at(i).threads
            threads = text(
                    fill: if threads > 1 { green } else { red },
                    [#threads]
                )

            let opt = if benchmarks.at(i).opt {
                    text(fill: green)[Есть]
                } else {
                    text(fill: red)[Нет]
                }

            let commit = benchmarks.at(i).commit

            let time = benchmarks.at(i).time

            let coef = strfmt(
                    "{:.3?}",
                    benchmarks.at(0).time / benchmarks.at(i).time
                )
            
            (num, threads, opt, commit, time, coef).map(item => [#item])
        }).flatten()
    )
}

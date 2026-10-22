; State of the speed graph, shared by the routines in speedgraph.dbl through
; the global data section graph. graph_start declares the section with init.
.define GRAPH_MAX_COLUMNS, 200
record
    gr_wid      ,i4
    gr_top      ,i4
    gr_left     ,i4
    gr_height   ,i4
    gr_width    ,i4
    gr_palette  ,i4
    gr_count    ,i4
    gr_scale    ,i8
    gr_values   ,[GRAPH_MAX_COLUMNS]i8

; Application state shared by every OrderDesk routine through the global data
; section odstate. Main declares the section with init.
record
    od_dbchn    ,d1
    od_homeid   ,i4
    od_driver   ,a40
    od_server   ,a128
    od_database ,a128
    od_client   ,a40
    od_encrypt  ,a10
    od_transport,a20

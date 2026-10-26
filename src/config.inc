; SQL_CONNECT is inherited from the launching shell for each run.
    stage = "SQL_CONNECT is not set"
    xcall getlog("SQL_CONNECT", connect, len)
    if (.not. connect)
        goto fail
    stage = "invalid SQL_CONNECT"
    ix = %instr(1, connect, ":")
    if (ix .lt. 2)
        goto fail
    driver = connect(1:ix-1)

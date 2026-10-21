; Reads the active SQL_CONNECT line of orderdesk.cfg into connect, and the
; text before its first colon into driver. Jumps to fail when there is none.
    stage = "reading orderdesk.cfg"
    cfgch = 0
    open(cfgch, i, "orderdesk.cfg") [err=fail]
    repeat
    begin
        reads(cfgch, line, cfg_eof)
        if (line(1:12) .eq. "SQL_CONNECT=")
        begin
            connect = line(13, ^size(line))
            exitloop
        end
    end
cfg_eof,
    close cfgch
    ix = %instr(1, connect, ":")
    if (ix .lt. 2)
        goto fail
    driver = connect(1:ix-1)

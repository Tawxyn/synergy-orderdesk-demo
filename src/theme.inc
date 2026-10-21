; OrderDesk colours: palette entries 11-15, set up in main with WP_PALET.
; Toolkit uses the lower entries for its own windows. Each driver has its own
; accent: amber (yellow) for the legacy driver, green (lime) for the new one.
.define PAL_BODY    ,11     ; silver on black
.define PAL_HEAD    ,12     ; cyan on black
.define PAL_LEGACY  ,13     ; yellow on black; reverse video draws solid blocks
.define PAL_FIGURE  ,14     ; white on black
.define PAL_NEW     ,15     ; lime on black; reverse video draws solid blocks

fu! JexadScript_ft()
  sy match Special /[()]/
  sy match Identifier /[a-z_][0-9a-z_]*/
  sy match Function /[A-Z][0-9A-Za-z]*/
  sy match PreProc /:[0-9A-Za-z_]\+/
  sy match escape /\\\([abefnrtv\\'"]\|x\x\x\|u\x\{4}\|U\x\{8}\|\o\{3}\)/ contained
  sy region String start=/"/ skip=/\\"/ end=/"/ contains=escape
  sy match Number /-\?\(0b[01]\+\|0o\o\+\|0x\x\+\|\d\+\(\.\d\+\)\?\|'\\''\|'\(\\.*\|.\)'\)/ contains=escape
  sy match Statement /([=$]/ contains=Special
  sy match Comment /#.*/
  hi link escape Special
  setl cms=#%s com=:#
endf

au BufRead,BufNewFile *.jxds se ft=jxds
au FileType jxds cal JexadScript_ft()

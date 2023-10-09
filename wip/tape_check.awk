/^malloc/  { ptr[$4] = $2; split($0, a, "// "); nfo[$4] = a[2]; next }
/^free/    { delete ptr[$2]; next }
/^calloc/  { ptr[$5] = $3*$3; split($0, a, "// "); nfo[$5] = a[2]; next }
/^realloc/ { delete ptr[$2]; ptr[$5] = $3; split($0, a, "// "); nfo[$5] = a[2]; next }
END { if (length(ptr)) {
    print "not freed:"
    for (k in ptr) print k " (" ptr[k] " B) last seen " nfo[k]
    exit 1
} }

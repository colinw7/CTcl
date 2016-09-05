set l [list 1 2 3 4 5 6 7 8 9 10 11 12]

foreach i $l {
  if {$i == 5} continue

  puts $i

  if {$i > 10} break
}

foreach a {alpha beta gamma} {
  puts $a
}

foreach a "abcdef" {
  puts $a
}

foreach {n v} {code "Code:" notes "Notes:"} {
  puts "$n"
  puts "$v"
}

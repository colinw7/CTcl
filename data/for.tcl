for {set i 0} {$i < 20} {incr i} {
  if {$i == 5} continue

  if {$i > 10} break

  puts $i
}

set i 0

while {$i < 20} {
  incr i

  if {$i == 5} continue

  if {$i > 10} break

  puts $i
}

set i 20

while {$i >= 0} {
  incr i -1

  if {$i == 5} continue

  puts $i
}

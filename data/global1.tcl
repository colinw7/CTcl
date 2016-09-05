proc test { } {
  global a

  set a 1

  global b
}

test

puts $a
puts $b

proc test { } {
  global a

  puts $a

  set a 3

  puts $a
}

set a 1

puts $a

test

puts $a

namespace eval fred {
  puts $a

  variable a

  set a 4

  puts $a
}

puts $a
puts $fred::a
